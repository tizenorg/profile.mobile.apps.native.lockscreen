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
#include "device_lock.h"
#include "device_lock_ctrl.h"
#include "util.h"
#include "lockscreen.h"

#include <app_control.h>
#include <Ecore.h>
#include <vconf.h>
#define KEY_DISPLAY_OVER_LOCKSCREEN "http://tizen.org/lock/window/above"
#define CAMERA_ICON_PATH_SHORTCUT IMAGE_DIR"quick_shot_icon.png"
#define CAMERA_PACKAGE "org.tizen.camera-app"

static int shortcut_enabled;
bool shortcut_secure_mode;
static int init_count;
static Ecore_Timer *delay_timer;
static const char *package;
static const char *image;

/* Following table contains NULL terminated list of packages that
 * can be run in "secure-mode" (without unlocking lockscreen, displaying
 * over lockscreen). Since there is currently no way to obtain such
 * list on runtime (eg. from app_control API), we whitelist some
 * applications that supports such mode when writing this code.
 * */
static const char *secured_mode_apps[] = {
	CAMERA_PACKAGE,
	NULL,
};

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
			if (lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_NONE) {
				if (delay_timer) ecore_timer_del(delay_timer);
				lockscreen_device_lock_ctrl_unlock_request();
			}
			break;
		case APP_CONTROL_RESULT_FAILED:
		case APP_CONTROL_RESULT_CANCELED:
			DBG("%s launch failed.", package);
			break;
	}
}

int lockscreen_shortcut_activate()
{
	app_control_h app_ctr;

	/* Currently do not support launching apps requireing unlock */
	if (lockscreen_shortcut_require_unlock())
		return 1;

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

	app_control_destroy(app_ctr);

	return 0;
}

static bool _lockscreen_shortcut_secured_mode_have(const char *package)
{
	const char **tmp;
	if (!package) return false;
	for (tmp = secured_mode_apps; tmp; tmp++) {
		if (!strcmp(*tmp, package))
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
		lockscreen_device_lock_shutdown();
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

bool lockscreen_shortcut_require_unlock(void)
{
	return !shortcut_secure_mode;
}
