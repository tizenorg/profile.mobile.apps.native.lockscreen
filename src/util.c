/*
 * Copyright 2016  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <app_common.h>
#include <feedback.h>

#include "log.h"
#include "util.h"
#include "lockscreen.h"

const char *util_get_file_path(enum app_subdir dir, const char *relative)
{
	static char buf[PATH_MAX];
	char *prefix;

	switch (dir) {
	case APP_DIR_DATA:
		prefix = app_get_data_path();
		break;
	case APP_DIR_CACHE:
		prefix = app_get_cache_path();
		break;
	case APP_DIR_RESOURCE:
		prefix = app_get_resource_path();
		break;
	case APP_DIR_SHARED_DATA:
		prefix = app_get_shared_data_path();
		break;
	case APP_DIR_SHARED_RESOURCE:
		prefix = app_get_shared_resource_path();
		break;
	case APP_DIR_SHARED_TRUSTED:
		prefix = app_get_shared_trusted_path();
		break;
	case APP_DIR_EXTERNAL_DATA:
		prefix = app_get_external_data_path();
		break;
	case APP_DIR_EXTERNAL_CACHE:
		prefix = app_get_external_cache_path();
		break;
	case APP_DIR_EXTERNAL_SHARED_DATA:
		prefix = app_get_external_shared_data_path();
		break;
	default:
		FAT("Not handled directory type.");
		return NULL;
	}
	size_t res = eina_file_path_join(buf, sizeof(buf), prefix, relative);
	free(prefix);
	if (res > sizeof(buf)) {
		ERR("Path exceeded PATH_MAX");
		return NULL;
	}

	return &buf[0];
}

const Elm_Theme *util_lockscreen_theme_get(void)
{
	static Elm_Theme *theme;
	if (!theme)
	{
		theme = elm_theme_new();
		elm_theme_ref_set(theme, NULL);
		elm_theme_overlay_add(NULL, util_get_res_file_path(EDJE_DIR"index.edj"));
	}
	return theme;
}

void util_feedback_tap_play(void)
{
	static int init;
	if (!init) {
		int ret = feedback_initialize();
		if (ret != FEEDBACK_ERROR_NONE) {
			FAT("feedback_initialize failed.");
		}
		init = 1;
	}
	feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
}
