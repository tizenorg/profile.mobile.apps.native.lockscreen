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
#include "camera_ctrl.h"
#include "camera.h"
#include "swipe_icon.h"
#include "main_view.h"

static Ecore_Event_Handler *handler;
static Evas_Object *main_view, *main_win;

static void _camera_clicked(void *data, Evas_Object *obj, void *event)
{
	lockscreen_camera_activate();
}

static void _camera_view_update()
{
	Evas_Object *cam_view;

	if (lockscreen_camera_is_on()) {
		cam_view = lockscreen_swipe_icon_view_create(main_view, ICON_PATH_CAMERA);
		evas_object_smart_callback_add(cam_view, SIGNAL_ICON_SELECTED, _camera_clicked, NULL);
		lockscreen_main_view_part_content_set(main_view, PART_CAMERA, cam_view);
	}
	else {
		cam_view = lockscreen_main_view_part_content_unset(main_view, PART_CAMERA);
		evas_object_del(cam_view);
	}
}

static Eina_Bool _cam_status_changed(void *data, int event, void *event_info)
{
	_camera_view_update();
	return EINA_TRUE;
}

int lockscreen_camera_ctrl_init(Evas_Object *win, Evas_Object *view)
{
	if (lockscreen_camera_init()) {
		ERR("lockscreen_camera_init failed");
		return 1;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_CAMERA_STATUS_CHANGED, _cam_status_changed, NULL);
	if (!handler)
		FAT("ecore_event_handler_add failed on LOCKSCREEN_EVENT_BATTERY_CHANGED event");
int lockscreen_camera_ctrl_init(Evas_Object *win, Evas_Object *view)
{
	if (lockscreen_camera_init()) {
		ERR("lockscreen_camera_init failed");
		return 1;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_CAMERA_STATUS_CHANGED, _cam_status_changed, NULL);
	if (!handler)
		FAT("ecore_event_handler_add failed on LOCKSCREEN_EVENT_BATTERY_CHANGED event");
	main_view = view;
	main_win = win;
	_camera_view_update();

	return 0;
}

void lockscreen_camera_ctrl_fini(void)
{
	Evas_Object *cam_view = lockscreen_main_view_part_content_get(main_view, PART_CAMERA);
	if (cam_view) evas_object_smart_callback_del(cam_view, SIGNAL_ICON_SELECTED, _camera_clicked);
	ecore_event_handler_del(handler);
	lockscreen_camera_shutdown();
}

void lockscreen_camera_ctrl_app_paused(void)
{
	Evas_Object *cam_view = lockscreen_main_view_part_content_get(main_view, PART_CAMERA);
	if (cam_view) {
		lockscreen_swipe_icon_view_reset(cam_view);
		/* Quick fix for rendering artifacts
		 * When camera is launched the lockscreen goes into "paused" state.
		 * On pause callback camera view is reset.
		 * However it occurs that edje signals sent to camera view layout
		 * are not processed after paused callback and frame is not refreshed
		 * properly.
		 * This leads to visible artifact when lockscreen change state from
		 * "pasued" => "resume"
		 * We force to render before app goes into "pasued" state by adding 2 calls: */
		edje_object_message_signal_process(elm_layout_edje_get(cam_view));
		evas_render(evas_object_evas_get(cam_view));
	}
}
