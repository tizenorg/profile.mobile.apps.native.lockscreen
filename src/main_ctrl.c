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

#include "log.h"
#include "main_ctrl.h"
#include "main_view.h"
#include "window.h"
#include "battery_ctrl.h"
#include "background.h"
#include "camera_ctrl.h"
#include "time_format_ctrl.h"
#include "util.h"
#include "sim_ctrl.h"
#include "display.h"
#include "events_ctrl.h"

#include <Elementary.h>
#include <app.h>

static Evas_Object *win;
static Evas_Object *view;

static void _view_unlocked(void *data, Evas_Object *obj, void *event)
{
	ui_app_exit();
}

static void _swipe_finished(void *data, Evas_Object *obj, void *event)
{
	/* When swipe finished play unlock animation and exit */
	evas_object_smart_callback_add(obj, SIGNAL_UNLOCK_ANIMATION_FINISHED, _view_unlocked, NULL);
	lockscreen_main_view_unlock(obj);
}

static Eina_Bool _lockscreen_main_ctrl_win_event_cb(void *data, Evas_Object *obj, Evas_Object *source, Evas_Callback_Type type, void *event_info)
{
	if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_TRUE;
	Evas_Event_Key_Down *ev = event_info;

	if (!strcmp(ev->key, "XF86PowerOff") || !strcmp(ev->key, "XF86Menu")) {
		lockscreen_time_format_ctrl_time_update();
	}
	else if (!strcmp(ev->key, "XF86Back")) {
		util_feedback_tap_play();
	}

	return EINA_TRUE;
}

static void _lockcscreen_main_ctrl_win_touch_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_display_timer_freeze();
}

static void _lockcscreen_main_ctrl_win_touch_end_cb(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_display_timer_renew();
}

int lockscreen_main_ctrl_init(void)
{
	win = lockscreen_window_create();
	if (!win)
		FAT("elm_win_add failed.");

	view = lockscreen_main_view_create(win);
	if (!view)
		FAT("lockscreen_main_view_create failed.");

	if (lockscreen_background_init()) {
		FAT("lockscreen_background_init failed. Background changes will not be available");
	} else {
		if (!lockscreen_main_view_background_set(view, LOCKSCREEN_BACKGROUND_TYPE_DEFAULT, lockscreen_background_file_get()))
			FAT("lockscreen_main_view_background_image_set failed");
	}

	if (lockscreen_display_init()) {
		FAT("lockscreen_display_init failed. Display on/off changes will not be available.");
	} else {
		evas_object_smart_callback_add(win, SIGNAL_TOUCH_STARTED, _lockcscreen_main_ctrl_win_touch_start_cb, NULL);
		evas_object_smart_callback_add(win, SIGNAL_TOUCH_ENDED, _lockcscreen_main_ctrl_win_touch_end_cb, NULL);
	}

	lockscreen_window_content_set(view);
	evas_object_smart_callback_add(view, SIGNAL_SWIPE_GESTURE_FINISHED, _swipe_finished, NULL);
	elm_object_event_callback_add(win, _lockscreen_main_ctrl_win_event_cb, NULL);

	// init subcontrollers
	if (lock_battery_ctrl_init(view))
		FAT("lock_battery_ctrl_init failed. Battery information will not be available");

	if (lockscreen_camera_ctrl_init(view))
		FAT("lockscreen_camera_ctrl_init failed. Camera quickshot will not be available");

	if (lockscreen_time_format_ctrl_init(view))
		FAT("lockscreen_time_format_ctrl_init failed. Time format changes will not be available");

	if (lockscreen_sim_ctrl_init(view))
		FAT("lockscreen_sim_ctrl_init failed. Sim PLMN updates will not be available");

	if (lockscreen_events_ctrl_init(view))
		FAT("lockscreen_events_ctrl_init failed. Lockscreen events will not be displayed");

	return 0;
}

void lockscreen_main_ctrl_shutdown(void)
{
	lockscreen_background_shutdown();
	evas_object_del(win);
}
