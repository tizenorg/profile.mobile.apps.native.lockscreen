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

#include <Evas.h>
#include <Ecore.h>
#include <Elementary.h>
#include <vconf.h>
#include <efl_util.h>

#include "lockscreen.h"
#include "log.h"
#include "window.h"
#include "tzsh_lockscreen_service.h"

#define STR_ATOM_PANEL_SCROLLABLE_STATE "_E_MOVE_PANEL_SCROLLABLE_STATE"

static struct _s_info {
	Evas_Object *win;

	tzsh_h tzsh;
	tzsh_lockscreen_service_h lockscreen_service;

	int win_w;
	int win_h;
} s_info = {
	.win = NULL,

	.tzsh = NULL,
	.lockscreen_service = NULL,

	.win_w = 0,
	.win_h = 0,
};

Evas_Object *lock_window_win_get(void)
{
	return s_info.win;
}

int lock_window_width_get(void)
{
	return s_info.win_w;
}

int lock_window_height_get(void)
{
	return s_info.win_h;
}

static lock_error_e _tzsh_set(Evas_Object *win)
{
	tzsh_h tzsh = NULL;
	tzsh_lockscreen_service_h lockscreen_service = NULL;
	tzsh_window tz_win;

	retv_if(!win, LOCK_ERROR_INVALID_PARAMETER);

	tzsh = tzsh_create(TZSH_TOOLKIT_TYPE_EFL);
	retv_if(!tzsh, LOCK_ERROR_FAIL);
	s_info.tzsh = tzsh;

	tz_win = elm_win_window_id_get(win);
	if (!tz_win) {
		tzsh_destroy(tzsh);
		return LOCK_ERROR_FAIL;
	}

	lockscreen_service = tzsh_lockscreen_service_create(tzsh, tz_win);
	if (!lockscreen_service) {
		tzsh_destroy(tzsh);
		return LOCK_ERROR_FAIL;
	}
	s_info.lockscreen_service = lockscreen_service;

	return LOCK_ERROR_OK;
}

static void _tzsh_unset(void)
{
	if (s_info.lockscreen_service) {
		tzsh_lockscreen_service_destroy(s_info.lockscreen_service);
		s_info.lockscreen_service = NULL;
	}

	if (s_info.tzsh) {
		tzsh_destroy(s_info.tzsh);
		s_info.tzsh = NULL;
	}
}

Evas_Object *lock_window_create(int type)
{
	int x = 0, y = 0, w = 0, h = 0;

	Evas_Object *win = elm_win_add(NULL, "LOCKSCREEN", ELM_WIN_NOTIFICATION);
	retv_if(!win, NULL);

	elm_win_alpha_set(win, EINA_TRUE);
	elm_win_title_set(win, "LOCKSCREEN");
	elm_win_borderless_set(win, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);
	efl_util_set_notification_window_level(win, EFL_UTIL_NOTIFICATION_LEVEL_MEDIUM);

	elm_win_screen_size_get(win, &x, &y, &w, &h);

	s_info.win = win;
	s_info.win_w = w;
	s_info.win_h = h;

	if (LOCK_ERROR_OK != _tzsh_set(win)) {
		_E("Failed to set tzsh");
	}

	return win;
}

void lock_window_destroy(void)
{
	_tzsh_unset();

	if (s_info.win) {
		evas_object_del(s_info.win);
		s_info.win = NULL;
	}
}
