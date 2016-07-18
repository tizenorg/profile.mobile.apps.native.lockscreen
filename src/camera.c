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

#include "camera.h"
#include "log.h"
#include "deviced.h"
#include "device_lock.h"
#include "device_lock_ctrl.h"

#include <app_control.h>
#include <Ecore.h>
#include <vconf.h>
#define KEY_DISPLAY_OVER_LOCKSCREEN "http://tizen.org/lock/window/above"

static int camera_enabled;
static int init_count;

int LOCKSCREEN_EVENT_CAMERA_STATUS_CHANGED;
static Ecore_Timer *delay_timer;

static Eina_Bool
_lockscreen_delayed_unlock(void *data)
{
	lockscreen_device_lock_ctrl_unlock_request();
	delay_timer = NULL;
	return EINA_FALSE;
}

static void _app_control_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	switch (result) {
		case APP_CONTROL_RESULT_APP_STARTED:
			DBG("Camera application launch succcessed.");
			if (lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_NONE) {
				//Quickfix. Add small delay until some sync mechanism for
				//app_control will be added to validate if callee window has
				//appeared.
				delay_timer = ecore_timer_add(1.5, _lockscreen_delayed_unlock, NULL);
			}
			break;
		case APP_CONTROL_RESULT_SUCCEEDED:
			DBG("App result callback");
			lockscreen_deviced_lockscreen_background_state_set(true);
			if ((lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_NONE) &&
				(result == APP_CONTROL_RESULT_SUCCEEDED)) {
				if (delay_timer) ecore_timer_del(delay_timer);
				lockscreen_device_lock_ctrl_unlock_request();
			}
			break;
		case APP_CONTROL_RESULT_FAILED:
		case APP_CONTROL_RESULT_CANCELED:
			DBG("Camera application launch failed.");
			break;
	}
}

int lockscreen_camera_activate()
{
	app_control_h app_ctr;

	int err = app_control_create(&app_ctr);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_create failed: %s", get_error_message(err));
		return 1;
	}

	if (lockscreen_device_lock_type_get() != LOCKSCREEN_DEVICE_LOCK_NONE) {
		err = app_control_set_launch_mode(app_ctr, APP_CONTROL_LAUNCH_MODE_GROUP);
		if (err != APP_CONTROL_ERROR_NONE) {
			ERR("app_control_set_launch_mode failed: %s", get_error_message(err));
			app_control_destroy(app_ctr);
			return 1;
		}

		/* Send a hint to camera-app to display itself over lockscreen */
		err = app_control_add_extra_data(app_ctr, KEY_DISPLAY_OVER_LOCKSCREEN, "on");
		if (err != APP_CONTROL_ERROR_NONE) {
			ERR("app_control_add_extra_data failed: %s", get_error_message(err));
			app_control_destroy(app_ctr);
			return 1;
		}

		/* Send a hint to camera-app to display itself in secure mode*/
		err = app_control_add_extra_data(app_ctr, "secure_mode", "true");
		if (err != APP_CONTROL_ERROR_NONE) {
			ERR("app_control_add_extra_data failed: %s", get_error_message(err));
			app_control_destroy(app_ctr);
			return 1;
		}
	}

	err = app_control_set_app_id(app_ctr, "org.tizen.camera-app");
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_set_app_id failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return 1;
	}

	err = app_control_enable_app_started_result_event(app_ctr);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_enable_app_started_result_event failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return 1;
	}

	err = app_control_send_launch_request(app_ctr, _app_control_reply_cb, NULL);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_send_launch_request failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return 1;
	}

	DBG("Launch request send for %s", APP_CONTROL_OPERATION_CREATE_CONTENT);
	app_control_destroy(app_ctr);

	return 0;
}

int lockscreen_camera_init(void)
{
	if (!init_count) {
		if (lockscreen_device_lock_init()) {
			ERR("lockscreen_device_lock_init failed");
			return 1;
		}
		int ret = vconf_get_bool(VCONFKEY_LOCKSCREEN_CAMERA_QUICK_ACCESS, &camera_enabled);
		if (ret) {
			ERR("vconf_get_bool failed");
			camera_enabled = 0;
		}
		LOCKSCREEN_EVENT_CAMERA_STATUS_CHANGED = ecore_event_type_new();
	}

	init_count++;
	return 0;
}

void lockscreen_camera_shutdown(void)
{
	if (init_count) {
		lockscreen_device_lock_shutdown();
		init_count--;
	}
}

bool lockscreen_camera_is_on(void)
{
	return camera_enabled;
}
