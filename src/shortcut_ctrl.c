/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Ecore.h>

#include "log.h"
#include "shortcut_ctrl.h"
#include "shortcut.h"
#include "swipe_icon.h"
#include "main_view.h"
#include "device_lock.h"

static Evas_Object *main_view, *main_win;
static Ecore_Event_Handler *unlock_request;

static void _shortcut_clicked(void *data, Evas_Object *obj, void *event)
{
	Evas_Object *sc_view = lockscreen_main_view_part_content_get(main_view, PART_SHORTCUT);
	if (lockscreen_shortcut_activate()) {
		ERR("lockscreen_shortcut_activate failed");
		lockscreen_swipe_icon_view_reset(sc_view);
	}
}

static void _shortcut_view_update()
{
	Evas_Object *sc_view;

	if (lockscreen_shortcut_is_on()) {
		sc_view = lockscreen_swipe_icon_view_create(main_view, lockscreen_shortcut_icon_path_get());
		evas_object_smart_callback_add(sc_view, SIGNAL_ICON_SELECTED, _shortcut_clicked, NULL);
		lockscreen_main_view_part_content_set(main_view, PART_SHORTCUT, sc_view);
	}
	else {
		sc_view = lockscreen_main_view_part_content_unset(main_view, PART_SHORTCUT);
		evas_object_del(sc_view);
	}
}

static Eina_Bool
_lockscreen_shortcut_ctrl_device_unlock_request(void *data, int event, void *event_info)
{
	Evas_Object *sc_view = lockscreen_main_view_part_content_get(main_view, PART_SHORTCUT);
	if (sc_view) {
		lockscreen_swipe_icon_view_reset(sc_view);
	}
	return EINA_TRUE;
}

int lockscreen_shortcut_ctrl_init(Evas_Object *win, Evas_Object *view)
{
	if (lockscreen_shortcut_init()) {
		ERR("lockscreen_shortcut_init failed");
		return 1;
	}

	if (lockscreen_device_lock_init()) {
		ERR("lockscreen_device_lock_init failed");
		lockscreen_shortcut_shutdown();
		return 1;
	}
	unlock_request = ecore_event_handler_add(
			LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST,
			_lockscreen_shortcut_ctrl_device_unlock_request, NULL);

	main_view = view;
	main_win = win;
	_shortcut_view_update();

	return 0;
}

void lockscreen_shortcut_ctrl_fini(void)
{
	ecore_event_handler_del(unlock_request);
	Evas_Object *sc_view = lockscreen_main_view_part_content_get(main_view, PART_SHORTCUT);
	if (sc_view) evas_object_smart_callback_del(sc_view, SIGNAL_ICON_SELECTED, _shortcut_clicked);
	lockscreen_shortcut_shutdown();
}

void lockscreen_shortcut_ctrl_app_paused(void)
{
	Evas_Object *sc_view = lockscreen_main_view_part_content_get(main_view, PART_SHORTCUT);
	if (sc_view) {
		lockscreen_swipe_icon_view_reset(sc_view);
		/* Quick fix for rendering artifacts
		 * When camera is launched the lockscreen goes into "paused" state.
		 * On pause callback shortcut view is reset.
		 * However it occurs that edje signals sent to camera view layout
		 * are not processed after paused callback and frame is not refreshed
		 * properly.
		 * This leads to visible artifact when lockscreen change state from
		 * "pasued" => "resume"
		 * We force to render before app goes into "pasued" state by adding 2 calls: */
		edje_object_message_signal_process(elm_layout_edje_get(sc_view));
		evas_render(evas_object_evas_get(sc_view));
	}
}
