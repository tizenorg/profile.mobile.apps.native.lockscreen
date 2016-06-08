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

#define TOAST_POPUP_TIMEOUT 3.0 //sec

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

static void _popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = data;

	if (popup)
		evas_object_del(popup);
	else
		evas_object_del(obj);
}

void util_popup_create(Evas_Object *win, char *title, char *desc)
{
	Evas_Object *popup;
	Evas_Object *button;

	popup = elm_popup_add(win);

	if (!title) {
		elm_object_style_set(popup, "toast");
		elm_popup_timeout_set(popup, TOAST_POPUP_TIMEOUT);
		evas_object_smart_callback_add(popup, "timeout", _popup_hide_cb, NULL);
	} else {
		elm_object_part_text_set(popup, "title,text", title);
		elm_popup_align_set(popup, 0.5, 0.5);

		button = elm_button_add(win);
		elm_object_text_set(button, "OK");

		elm_object_part_content_set(popup, "button1", button);
		evas_object_smart_callback_add(button, "clicked", _popup_hide_cb, popup);
		evas_object_show(button);
	}

	elm_object_text_set(popup, desc);

	evas_object_show(popup);
}
