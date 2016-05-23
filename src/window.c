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

#include <Elementary.h>
#include <tzsh_lockscreen_service.h>

#include "window.h"
#include "log.h"


static struct {
	Evas_Object *win;
	Evas_Object *conformant;
} view;

static void _lockscreen_window_event_rect_mouse_down_cb(void *data, Evas *e, Evas_Object *src, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_TOUCH_STARTED, NULL);
}

static void _lockscreen_window_event_rect_mouse_up_cb(void *data, Evas *e, Evas_Object *src, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_TOUCH_ENDED, NULL);
}

static void _lockscreen_window_event_rect_geometry_changed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	int x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	evas_object_geometry_set(data, x, y, w, h);
}

Evas_Object *lockscreen_window_create(void)
{
	tzsh_h tzsh = NULL;
	tzsh_lockscreen_service_h lockscreen_service = NULL;
	Evas_Object *win = elm_win_add(NULL, "LOCKSCREEN", ELM_WIN_BASIC);
	if (!win) return NULL;

	elm_win_alpha_set(win, EINA_TRUE);
	elm_win_title_set(win, "LOCKSCREEN");
	elm_win_borderless_set(win, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_role_set(win, "notification-normal");
	elm_win_fullscreen_set(win, EINA_TRUE);
	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);

	tzsh = tzsh_create(TZSH_TOOLKIT_TYPE_EFL);
	if (!tzsh) {
		ERR("tzsh_create failed");
		evas_object_del(win);
		return NULL;
	}

	lockscreen_service = tzsh_lockscreen_service_create(tzsh, elm_win_window_id_get(win));
	if (!lockscreen_service) {
		ERR("tzsh_lockscreen_service_create failed");
		tzsh_destroy(tzsh);
		evas_object_del(win);
		return NULL;
	}

	Evas_Object *conformant = elm_conformant_add(win);
	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_win_resize_object_add(win, conformant);

	elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "elm");

	Evas_Object *event_rect = evas_object_rectangle_add(evas_object_evas_get(win));
	evas_object_color_set(event_rect, 0, 0, 0, 0);
	evas_object_layer_set(event_rect, EVAS_LAYER_MAX);
	evas_object_repeat_events_set(event_rect, EINA_TRUE);
	evas_object_event_callback_add(event_rect, EVAS_CALLBACK_MOUSE_DOWN, _lockscreen_window_event_rect_mouse_down_cb, win);
	evas_object_event_callback_add(event_rect, EVAS_CALLBACK_MOUSE_UP, _lockscreen_window_event_rect_mouse_up_cb, win);
	evas_object_show(event_rect);

	evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _lockscreen_window_event_rect_geometry_changed_cb, event_rect);
	evas_object_event_callback_add(win, EVAS_CALLBACK_MOVE, _lockscreen_window_event_rect_geometry_changed_cb, event_rect);
	evas_object_show(win);
	evas_object_show(conformant);

	view.win = win;
	view.conformant = conformant;

	return win;
}

void lockscreen_window_content_set(Evas_Object *content)
{
	elm_object_part_content_set(view.conformant, NULL, content);
}

void lockscreen_window_quickpanel_block_set(Eina_Bool block)
{
}

int lock_window_width_get(void)
{
	int ret;
	evas_object_geometry_get(view.win, NULL, NULL, &ret, NULL);
	return ret;
}

int lock_window_height_get(void)
{
	int ret;
	evas_object_geometry_get(view.win, NULL, NULL, NULL, &ret);
	return ret;
}
