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

#include "shortcut.h"
#include "log.h"
#include "deviced.h"
#include "util.h"
#include "lockscreen.h"
#include "device_lock.h"

#include <app_control.h>
#include <Ecore.h>
#include <vconf.h>
#define KEY_DISPLAY_OVER_LOCKSCREEN "http://tizen.org/lock/window/above"
#define CAMERA_ICON_PATH_SHORTCUT IMAGE_DIR"quick_shot_icon.png"
#define CAMERA_PACKAGE "org.tizen.camera-app"

static int shortcut_enabled;
bool shortcut_secure_mode;
static int init_count;
static const char *package;
static const char *image;
static Ecore_Event_Handler *unlock_handlers[2];

/* Following array contains packages that
 * can be run in "secure-mode" (without unlocking lockscreen, displaying
 * over lockscreen). Since there is currently no way to obtain such
 * list on runtime (eg. from app_control API), we whitelist some
 * applications that supports such mode when writing this code.
 * */
static const char *secured_mode_apps[] = {
	CAMERA_PACKAGE,
};

static void _app_control_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	switch (result) {
		case APP_CONTROL_RESULT_APP_STARTED:
		case APP_CONTROL_RESULT_SUCCEEDED:
			DBG("%s launch succcessed.", package);
			lockscreen_deviced_lockscreen_background_state_set(true);
			break;
		case APP_CONTROL_RESULT_FAILED:
		case APP_CONTROL_RESULT_CANCELED:
			DBG("%s launch failed.", package);
			break;
	}
}

static int
_lockscreen_shortcut_activate_internal(void)
{
	app_control_h app_ctr;

	int err = app_control_create(&app_ctr);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_create failed: %s", get_error_message(err));
		return 1;
	}

	if (shortcut_secure_mode) {
		err = app_control_set_launch_mode(app_ctr, APP_CONTROL_LAUNCH_MODE_GROUP);
		if (err != APP_CONTROL_ERROR_NONE) {
			ERR("app_control_set_launch_mode failed: %s", get_error_message(err));
			app_control_destroy(app_ctr);
			return 1;
		}

		/* Send a hint to shortcut app to display itself over lockscreen */
		err = app_control_add_extra_data(app_ctr, KEY_DISPLAY_OVER_LOCKSCREEN, "on");
		if (err != APP_CONTROL_ERROR_NONE) {
			ERR("app_control_add_extra_data failed: %s", get_error_message(err));
			app_control_destroy(app_ctr);
			return 1;
		}

		/* Send a hint to shortcut app to display itself in secure mode */
		err = app_control_add_extra_data(app_ctr, "secure_mode", "true");
		if (err != APP_CONTROL_ERROR_NONE) {
			ERR("app_control_add_extra_data failed: %s", get_error_message(err));
			app_control_destroy(app_ctr);
			return 1;
		}
	}

	err = app_control_set_app_id(app_ctr, package);
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

	INF("%s package launched", package);
	app_control_destroy(app_ctr);

	return 0;
}

static Eina_Bool
_lockscreen_shortcut_device_unlock_cancelled(void *data, int event, void *event_info)
{
	ecore_event_handler_del(unlock_handlers[0]);
	ecore_event_handler_del(unlock_handlers[1]);
	return EINA_TRUE;
}

static Eina_Bool
_lockscreen_shortcut_device_unlocked(void *data, int event, void *event_info)
{
	_lockscreen_shortcut_activate_internal();
	ecore_event_handler_del(unlock_handlers[0]);
	ecore_event_handler_del(unlock_handlers[1]);
	return EINA_TRUE;
}

int lockscreen_shortcut_activate()
{
	/* Currently do not support launching apps requireing unlock */
	if (!shortcut_secure_mode) {
		if (!lockscreen_device_lock_unlock_request(NULL)) {
			unlock_handlers[0] = ecore_event_handler_add(
					LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_CANCELLED,
					_lockscreen_shortcut_device_unlock_cancelled, NULL);
			unlock_handlers[1] = ecore_event_handler_add(
					LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED,
					_lockscreen_shortcut_device_unlocked, NULL);
		} else {
			INF("lockscreen_device_lock_unlock_request failed");
			return 1;
		}
		return 0;
	}

	return _lockscreen_shortcut_activate_internal();
}

static bool _lockscreen_shortcut_secured_mode_have(const char *package)
{
	int i;
	if (!package) return false;
	for (i = 0; i < SIZE(secured_mode_apps); i++) {
		if (secured_mode_apps[i] && !strcmp(secured_mode_apps[i], package))
			return true;
	}
	return false;
}

int lockscreen_shortcut_init(void)
{
	if (!init_count) {

		if (lockscreen_device_lock_init()) {
			ERR("lockscreen_device_lock_init failed");
			return 1;
		}

		// FIXME
		// change following code when loading shortcut application from settings will
		// be available
		int ret = vconf_get_bool(VCONFKEY_LOCKSCREEN_CAMERA_QUICK_ACCESS, &shortcut_enabled);
		if (ret) {
			ERR("vconf_get_bool failed");
			shortcut_enabled = 0;
		}
		if (shortcut_enabled) {
			image = eina_stringshare_add(util_get_res_file_path(CAMERA_ICON_PATH_SHORTCUT));
			package = eina_stringshare_add(CAMERA_PACKAGE);
			shortcut_secure_mode = _lockscreen_shortcut_secured_mode_have(package);
		}
	}

	init_count++;
	return 0;
}

void lockscreen_shortcut_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count) {
			eina_stringshare_del(image);
			eina_stringshare_del(package);
		}
	}
}

bool lockscreen_shortcut_is_on(void)
{
	return shortcut_enabled;
}

const char *lockscreen_shortcut_icon_path_get(void)
{
	return image;
}
