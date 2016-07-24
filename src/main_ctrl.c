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
#include "sim_lock_ctrl.h"
#include "display.h"
#include "events_ctrl.h"
#include "background_ctrl.h"
#include "device_lock_ctrl.h"

#include <Elementary.h>

static Evas_Object *win;
static Evas_Object *view;

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

static void _lockcscreen_main_ctrl_win_display_freeze(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_display_timer_freeze();
}

static void _lockcscreen_main_ctrl_win_display_thaw(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_display_timer_renew(false);
}

int lockscreen_main_ctrl_init(void)
{
	win = lockscreen_window_create();
	if (!win)
		FAT("elm_win_add failed.");

	view = lockscreen_main_view_create(win);
	if (!view)
		FAT("lockscreen_main_view_create failed.");

	if (lockscreen_display_init()) {
		FAT("lockscreen_display_init failed. Display on/off changes will not be available.");
	} else {
		evas_object_smart_callback_add(win, SIGNAL_TOUCH_STARTED, _lockcscreen_main_ctrl_win_display_freeze, NULL);
		evas_object_smart_callback_add(win, SIGNAL_TOUCH_ENDED, _lockcscreen_main_ctrl_win_display_thaw, NULL);
		evas_object_smart_callback_add(win, "unfocused", _lockcscreen_main_ctrl_win_display_freeze, NULL);
		evas_object_smart_callback_add(win, "normal", _lockcscreen_main_ctrl_win_display_thaw, NULL);
	}

	lockscreen_window_content_set(view);
	elm_object_event_callback_add(win, _lockscreen_main_ctrl_win_event_cb, NULL);

	// init subcontrollers
	if (lock_battery_ctrl_init(view))
		FAT("lock_battery_ctrl_init failed. Battery information will not be available");

	if (lockscreen_camera_ctrl_init(win, view))
		FAT("lockscreen_camera_ctrl_init failed. Camera quickshot will not be available");

	if (lockscreen_time_format_ctrl_init(view))
		FAT("lockscreen_time_format_ctrl_init failed. Time format changes will not be available");

	if (lockscreen_sim_ctrl_init(view))
		FAT("lockscreen_sim_ctrl_init failed. Sim PLMN updates will not be available");

	if (lockscreen_events_ctrl_init(view))
		FAT("lockscreen_events_ctrl_init failed. Lockscreen events will not be displayed");

	if (lockscreen_background_ctrl_init(view))
		FAT("lockscreen_background_ctrl_init failed. Lockscreen background changes will not be available");

	if (lockscreen_device_lock_ctrl_init(view))
		FAT("lockscreen_device_lock_ctrl_init failed. Password unlock will not be available");

	if (lockscreen_sim_lock_ctrl_init(view))
		FAT("lockscreen_sim_lock_ctrl_init failed. Password unlock will not be available");

	return 0;
}

void lockscreen_main_ctrl_shutdown(void)
{
	lockscreen_events_ctrl_shutdown();
	lockscreen_background_ctrl_shutdown();
	evas_object_del(win);
}

void lockscreen_main_ctrl_app_paused(void)
{
	lockscreen_camera_ctrl_app_paused();
}
