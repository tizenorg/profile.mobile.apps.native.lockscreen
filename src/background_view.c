/*
 * Copyright (c) 2009-2014 Samsung Electronics Co., Ltd All Rights Reserved
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
#include "background_view.h"
#include "window.h"
#include "property.h"
#include "default_lock.h"

#define EDJE_SIGNAL_SOURCE "bg"
#define EDJE_SIGNAL_EMIT_MUSIC_ON "music_on"
#define EDJE_SIGNAL_EMIT_MUSIC_OFF "music_off"

static struct _s_info {
	Evas_Object *bg;
} s_info = {
	.bg = NULL,
};

Evas_Object *lock_background_view_bg_get(void)
{
	return s_info.bg;
}

lock_error_e lock_background_view_image_set(lock_bg_type_e type, char *file)
{
	Evas_Object *lock_layout = NULL;
	const char *old_filename = NULL;
	const char *emission;

	char *lock_bg = NULL;

	retv_if(!s_info.bg, LOCK_ERROR_INVALID_PARAMETER);

	elm_bg_file_get(s_info.bg, &old_filename, NULL);
	if (!old_filename) {
		old_filename = LOCK_DEFAULT_BG_PATH;
	}
	_D("old file name : %s", old_filename);

	switch(type) {
	case LOCK_BG_DEFAULT:
		if (LOCK_ERROR_OK != lock_property_get_string(PROPERTY_TYPE_SYSTEM_SETTINGS, (void *)SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, &lock_bg)) {
			_E("Failed to get lockscreen BG");
			goto ERROR;
		}
		goto_if(!lock_bg, ERROR);

		_D("lock_bg : %s", lock_bg);

		if (!elm_bg_file_set(s_info.bg, lock_bg, NULL)) {
			_E("Failed to set a BG image : %s", lock_bg);
			free(lock_bg);
			goto ERROR;
		}

		emission = EDJE_SIGNAL_EMIT_MUSIC_OFF;

		free(lock_bg);
		break;
	case LOCK_BG_ALBUM_ART:
		if (!file) {
			_E("Failed to set a BG image");
			return LOCK_ERROR_INVALID_PARAMETER;
		}

		if (!elm_bg_file_set(s_info.bg, file, NULL)) {
			_E("Failed to set album art BG : %s", file);
			goto ERROR;
		}

		emission = EDJE_SIGNAL_EMIT_MUSIC_ON;
		break;
	default:
		_E("Failed to set background image : type error(%d)", type);
		goto ERROR;
	}

	lock_layout = lock_default_lock_layout_get();
	if (lock_layout) {
		elm_layout_signal_emit(lock_layout, emission, EDJE_SIGNAL_SOURCE);
	}

	return LOCK_ERROR_OK;

ERROR:
	if (!elm_bg_file_set(s_info.bg, old_filename, NULL)) {
		_E("Failed to set old BG file : %s. Retry to set default BG.", old_filename);
		if (!elm_bg_file_set(s_info.bg, LOCK_DEFAULT_BG_PATH, NULL)) {
			_E("Failed to set default BG : %s.", LOCK_DEFAULT_BG_PATH);
			return LOCK_ERROR_FAIL;
		}
	}

	return LOCK_ERROR_OK;
}

Evas_Object *lock_background_view_bg_create(Evas_Object *win)
{
	int win_w = lock_window_width_get();
	int win_h = lock_window_height_get();
	_D("win size : %dx%d", win_w, win_h);

	retv_if(!win, NULL);

	Evas_Object *bg = elm_bg_add(win);
	retv_if(!bg , NULL);

	elm_bg_option_set(bg, ELM_BG_OPTION_SCALE);

	evas_object_size_hint_min_set(bg, win_w, win_h);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	s_info.bg = bg;

	if (LOCK_ERROR_OK != lock_background_view_image_set(LOCK_BG_DEFAULT, NULL)) {
		_E("Failed to set a BG image");
	}

	return bg;
}

void lock_background_view_bg_del(void)
{
	if (s_info.bg) {
		evas_object_del(s_info.bg);
		s_info.bg = NULL;
	}
}
