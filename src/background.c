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

#include <system_settings.h>
#include <Ecore.h>
#include <Ecore_File.h>

#define DEFAULT_BG IMAGE_DIR"Default.jpg"

static char *background_file;
static int init_count;
int LOCKSCREEN_EVENT_BACKGROUND_CHANGED;

static void _lockscreen_background_load_from_system_settings(void)
{
	char *bg;
	int ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, &bg);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		ERR("system_settings_set_value_string failed: %s", get_error_message(ret));
		lockscreen_background_file_set(NULL);
		return;
	}
	if (lockscreen_background_file_set(bg))
		lockscreen_background_file_set(NULL);
	free(bg);
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
		if (!ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("system_settings_set_changed_cb: %s", get_error_message(ret));
			return -1;
		}
		_lockscreen_background_load_from_system_settings();
	}
	init_count++;
	return 0;
}

int lockscreen_background_file_set(const char *path)
{
	if (!path) {
		return lockscreen_background_file_set(util_get_res_file_path(DEFAULT_BG));
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

	ecore_event_add(LOCKSCREEN_EVENT_BACKGROUND_CHANGED, NULL, NULL, NULL);
	return 0;
}

void lockscreen_background_shutdown(void)
{
	if (init_count) {
		init_count--;
		free(background_file);
		background_file = NULL;
	}
}

const char *lockscreen_background_file_get(void)
{
	return background_file;
}
