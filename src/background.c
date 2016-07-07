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

#include "lockscreen.h"
#include "log.h"
#include "background.h"
#include "util.h"

#include <message_port.h>
#include <system_settings.h>
#include <Ecore_File.h>

#define DEFAULT_BG IMAGE_DIR"Default.jpg"


#define RUNTIME_BACKGROUND_LOCAL_PORT "lockscreen/port/background/ondemand"
#define RUNTIME_BACKGROUND_KEY "lockscreen/background/file_path"

static char *background_file = NULL;
static char *background_runtime_file = NULL;

static Eina_Bool enable_runtime_background_change = EINA_FALSE;
static int port_id;
static int init_count;
int LOCKSCREEN_EVENT_BACKGROUND_CHANGED;

static int _lockscreen_background_file_set(const char *path);

static void _lockscreen_background_load_from_system_settings(void)
{
	char *bg;
	int ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, &bg);
	DBG("bg:%s", bg);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		ERR("system_settings_set_value_string failed: %s", get_error_message(ret));
		_lockscreen_background_file_set(NULL);
		return;
	}
	if (_lockscreen_background_file_set(bg))
		_lockscreen_background_file_set(NULL);
	free(bg);
}

static void _lockscreen_background_remote_file_set(char *file)
{
	if (!strcmp(file, background_file))
		return;

	if (!strcmp(file, "bg/unset")) {
		free(background_file);

		int ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, &background_file);

		if (ret != SYSTEM_SETTINGS_ERROR_NONE)
				ERR("system_settings_set_value_string failed: %s", get_error_message(ret));

		if (enable_runtime_background_change)
			ecore_event_add(LOCKSCREEN_EVENT_BACKGROUND_CHANGED, NULL, NULL, NULL);

		return;
	}

	if (file) {
		if (!ecore_file_can_read(file)) {
			ERR("Cannot access/read background file: %s", file);
			return;
		}

		free(background_file);
		background_file = strdup(file);

		if (enable_runtime_background_change)
			ecore_event_add(LOCKSCREEN_EVENT_BACKGROUND_CHANGED, NULL, NULL, NULL);
	}
}

static void _background_message_port_cb(int trusted_local_port_id, const char *remote_app_id,
		const char *remote_port, bool trusted_remote_port, bundle *message, void *data)
{
	if(!trusted_remote_port) {
		ERR("The remote port is untrusted");
		return;
	}

	int ret;

	ret = bundle_get_str(message, RUNTIME_BACKGROUND_KEY, &background_runtime_file);
	if(ret != BUNDLE_ERROR_NONE) {
		ERR("bundle_get_type failed[%d]:%s", ret, get_error_message(ret));
		return;
	}

	_lockscreen_background_remote_file_set(background_runtime_file);
}

static int _lockscreen_background_message_port_init(void)
{
	DBG("lockscreen_background_message_port_init");
	port_id = message_port_register_trusted_local_port(RUNTIME_BACKGROUND_LOCAL_PORT, _background_message_port_cb, NULL);
	if(port_id < MESSAGE_PORT_ERROR_NONE) {
		ERR("message_port_register_trusted_local_port failed[%d]:%s", port_id, get_error_message(port_id));
		return port_id;
	}

	return 0;
}

static void _lockscreen_background_system_settings_key_changed(system_settings_key_e key, void *user_data)
{
	if (key != SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN)
		return;
	_lockscreen_background_load_from_system_settings();
}

int lockscreen_background_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_BACKGROUND_CHANGED = ecore_event_type_new();
		int ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, _lockscreen_background_system_settings_key_changed, NULL);
		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("system_settings_set_changed_cb: %s", get_error_message(ret));
			return -1;
		}
		_lockscreen_background_message_port_init();

		if (!enable_runtime_background_change)
			_lockscreen_background_load_from_system_settings();
	}
	init_count++;
	return 0;
}

void lockscreen_background_runtime_background_enabled_set(Eina_Bool val)
{
	enable_runtime_background_change = val;
	if (val)
		ecore_event_add(LOCKSCREEN_EVENT_BACKGROUND_CHANGED, NULL, NULL, NULL);
	else
		_lockscreen_background_load_from_system_settings();
}


static int _lockscreen_background_file_set(const char *path)
{
	if (!path) {
		return _lockscreen_background_file_set(util_get_res_file_path(DEFAULT_BG));
	}

	if (background_file && !strcmp(background_file, path)) {
		return 0;
	}

	if (!ecore_file_can_read(path)) {
		ERR("Cannot access/read background file: %s", path);
		return -1;
	}

	free(background_file);
	background_file = strdup(path);

	INF("Background file: %s", path);

	ecore_event_add(LOCKSCREEN_EVENT_BACKGROUND_CHANGED, NULL, NULL, NULL);

	return 0;
}

static void lockscreen_background_message_port_deinit(void)
{
	int ret;

	ret = message_port_unregister_trusted_local_port(port_id);
	if(ret < MESSAGE_PORT_ERROR_NONE)
		ERR("message_port_register_trusted_local_port failed[%d]:%s", ret, get_error_message(ret));
}

void lockscreen_background_shutdown(void)
{
	if (init_count) {
		init_count--;
		free(background_file);
		background_file = NULL;

		lockscreen_background_message_port_deinit();

	}
}

const char *lockscreen_background_file_get(void)
{
	return background_file;
}
