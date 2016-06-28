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

static Evas_Object *main_view, *main_win;

static void _shortcut_clicked(void *data, Evas_Object *obj, void *event)
{
	lockscreen_shortcut_activate();
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

static void _lockscreen_shortcut_ctrl_win_normal(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *sc_view = lockscreen_main_view_part_content_get(main_view, PART_SHORTCUT);
	if (sc_view)
		lockscreen_swipe_icon_view_reset(sc_view);
}

int lockscreen_shortcut_ctrl_init(Evas_Object *win, Evas_Object *view)
{
	if (lockscreen_shortcut_init()) {
		ERR("lockscreen_shortcut_init failed");
		return 1;
	}

	main_view = view;
	main_win = win;
	/* "widthdrawn" seems to be better event, however on Tizen 3.0 is never
	 * triggered */
	evas_object_smart_callback_add(win, "normal", _lockscreen_shortcut_ctrl_win_normal, NULL);
	_shortcut_view_update();

	return 0;
}

void lockscreen_shortcut_ctrl_fini(void)
{
	Evas_Object *sc_view = lockscreen_main_view_part_content_get(main_view, PART_SHORTCUT);
	if (sc_view) evas_object_smart_callback_del(sc_view, SIGNAL_ICON_SELECTED, _shortcut_clicked);
	evas_object_smart_callback_del(main_win, "normal", _lockscreen_shortcut_ctrl_win_normal);
	lockscreen_shortcut_shutdown();
}
