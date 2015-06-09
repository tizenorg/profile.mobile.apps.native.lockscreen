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
#include <efl_assist.h>
#include <app.h>
#include <minicontrol-viewer.h>
#include <bundle.h>

#include "lockscreen.h"
#include "log.h"
#include "default_lock.h"
#include "property.h"
#include "window.h"
#include "background_view.h"
#include "battery.h"
#include "lock_time.h"
#include "sim_state.h"

#define INDICATOR_HEIGHT 38
#define UNLOCK_DISTANCE 140

#define MINICONTROL_BUNDLE_KEY_WIDTH "width"
#define MINICONTROL_BUNDLE_KEY_HEIGHT "height"

static struct _s_info {
	Evas_Object *conformant;
	Evas_Object *layout;
	Evas_Object *swipe_layout;

	Ecore_Event_Handler *mouse_down_handler;
	Ecore_Event_Handler *mouse_move_handler;
	Ecore_Event_Handler *mouse_up_handler;

	Eina_Bool is_mouse_down;

	int lcd_off_count;

	int clicked_x;
	int clicked_y;

	lock_exit_state_e exit_state;
} s_info = {
	.conformant = NULL,
	.layout= NULL,
	.swipe_layout = NULL,

	.mouse_down_handler = NULL,
	.mouse_move_handler = NULL,
	.mouse_up_handler = NULL,

	.is_mouse_down = EINA_FALSE,
	.clicked_x = 0,
	.clicked_y = 0,

	.exit_state = LOCK_EXIT_STATE_NORMAL,
};

Evas_Object *lock_default_conformant_get(void)
{
	return s_info.conformant;
}

Evas_Object *lock_default_lock_layout_get(void)
{
	return s_info.layout;
}

Evas_Object *lock_default_swipe_layout_get(void)
{
	return s_info.swipe_layout;
}

void _default_lock_hw_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	_I("%s", __func__);
}

static Eina_Bool _default_lock_mouse_down_cb(void *data, int type, void *event)
{
	_D("%s", __func__);

	Ecore_Event_Mouse_Button *m = event;

	int touch_upper_y = 0;

	retv_if(!m, ECORE_CALLBACK_CANCEL);
	retv_if(!s_info.swipe_layout, ECORE_CALLBACK_CANCEL);

	/* (Up to 3 times, 30 seconds) is extended by 10 seconds Control panel area when tap */
	lockscreen_lcd_off_count_raise();

	s_info.clicked_x = m->root.x;
	s_info.clicked_y = m->root.y;
	_D("clicked x(%d), y(%d)", s_info.clicked_x, s_info.clicked_y);

	touch_upper_y = INDICATOR_HEIGHT;
	_D("touch upper y : %d", touch_upper_y);

	if (m->root.y <= touch_upper_y) {
		_D("ignore touch event(%d > %d)", m->root.y, touch_upper_y);
		s_info.is_mouse_down = EINA_FALSE;
	} else {
		elm_object_signal_emit(s_info.swipe_layout, "vi_effect_start", "padding.top");
		s_info.is_mouse_down = EINA_TRUE;
	}

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool _default_lock_mouse_move_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Move *m = event;
	retv_if(!m, ECORE_CALLBACK_CANCEL);
	retv_if(m->multi.device != 0, ECORE_CALLBACK_CANCEL);

	int const dx = m->x - s_info.clicked_x;
	int const dy = s_info.clicked_y - m->y;
	int scaled_unlock_distance = _X(UNLOCK_DISTANCE);
	int distance = sqrt(dx*dx + dy*dy) + _X(20);

	if (distance >= scaled_unlock_distance) {
		s_info.exit_state = LOCK_EXIT_STATE_EXIT;
	} else {
		s_info.exit_state = LOCK_EXIT_STATE_NORMAL;
	}

	return ECORE_CALLBACK_DONE;
}

static Eina_Bool _default_lock_mouse_up_cb(void *data, int type, void *event)
{
	_D("%s", __func__);

	Ecore_Event_Mouse_Button *m = event;

	retv_if(!m, ECORE_CALLBACK_CANCEL);
	retv_if(m->multi.device != 0, ECORE_CALLBACK_CANCEL);
	retv_if(!s_info.layout, ECORE_CALLBACK_CANCEL);
	retv_if(!s_info.swipe_layout, ECORE_CALLBACK_CANCEL);


	if (s_info.is_mouse_down == EINA_FALSE) {
		_I("ignore touch event");
		return ECORE_CALLBACK_CANCEL;
	}

	s_info.is_mouse_down = EINA_FALSE;

	switch(s_info.exit_state) {
	case LOCK_EXIT_STATE_NORMAL :
		_D("cancel unlock");
		break;
	case LOCK_EXIT_STATE_EXIT :
		_D("exit lockscreen");

		elm_object_signal_emit(s_info.swipe_layout, "vi_effect", "padding.top");
		elm_object_signal_emit(s_info.layout, "vi_effect", "vi_clipper");
		return ECORE_CALLBACK_CANCEL;
	default :
		_E("type error : %d", s_info.exit_state);
		break;
	}

	s_info.exit_state = LOCK_EXIT_STATE_NORMAL;

	elm_object_signal_emit(s_info.swipe_layout, "vi_effect_stop", "padding.top");
	elm_object_signal_emit(s_info.layout, "vi_effect_stop", "vi_clipper");
	elm_object_signal_emit(s_info.swipe_layout, "show,txt,plmn", "txt.plmn");

	return ECORE_CALLBACK_PASS_ON;
}

static void _vi_effect_end_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	_D("%s", __func__);

	ui_app_exit();
}

static lock_error_e _unlock_panel_create(void)
{
	retv_if(!s_info.swipe_layout, LOCK_ERROR_FAIL);

	s_info.mouse_down_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, _default_lock_mouse_down_cb, NULL);
	if (!s_info.mouse_down_handler) {
		_E("Failed to add mouse down handler");
	}

	s_info.mouse_move_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _default_lock_mouse_move_cb, NULL);
	if (!s_info.mouse_move_handler) {
		_E("Failed to add mouse move handler");
	}

	s_info.mouse_up_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, _default_lock_mouse_up_cb, NULL);
	if (!s_info.mouse_up_handler) {
		_E("Failed to add mouse up handler");
	}

	elm_object_signal_callback_add(s_info.swipe_layout, "vi_effect_end", "vi_clipper", _vi_effect_end_cb, NULL);

	return LOCK_ERROR_OK;
}

static Evas_Object *_swipe_layout_create(Evas_Object *parent)
{
	Evas_Object *swipe_layout = NULL;

	retv_if(!parent, NULL);

	swipe_layout = elm_layout_add(parent);
	retv_if(!swipe_layout, NULL);

	if (!elm_layout_file_set(swipe_layout, LOCK_EDJE_FILE, "swipe-lock")) {
		_E("Failed to set edje file for swipe lock");
		goto ERROR;
	}

	evas_object_size_hint_weight_set(swipe_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(swipe_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(swipe_layout);
	s_info.swipe_layout = swipe_layout;

	/* initialize time & date information */
	lock_time_init();

	/* initialize battery information */
	if (LOCK_ERROR_OK != lock_battery_init()) {
		_E("Failed to initialize battery information");
	}

	/* initialize PLMN-SPN information */
	if (LOCK_ERROR_OK != lock_sim_state_init()) {
		_E("Failed to initialize sim state");
	}

	return swipe_layout;

ERROR:
	_E("Failed to create swipe layout");

	if(swipe_layout) {
		evas_object_del(swipe_layout);
		swipe_layout = NULL;
	}

	return NULL;
}

static Evas_Object *_layout_create(void)
{
	Evas_Object *layout = NULL;
	Evas_Object *swipe_layout = NULL;
	Evas_Object *win = NULL;

	win = lock_window_win_get();
	retv_if(!win, NULL);

	layout = elm_layout_add(win);
	retv_if(!layout, NULL);

	evas_object_show(layout);

	if (!elm_layout_file_set(layout, LOCK_EDJE_FILE, "lockscreen")) {
		_E("Failed to set edje file");
		goto ERROR;
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	swipe_layout = _swipe_layout_create(layout);
	if (!swipe_layout) {
		_E("Failed to create swipe layout");
		goto ERROR;
	}

	elm_object_part_content_set(layout, "sw.swipe_layout", swipe_layout);
	if (!elm_object_part_content_get(layout, "sw.swipe_layout")) {
		_E("Failed to set swipe layout");
		goto ERROR;
	}

	elm_win_resize_object_add(win, layout);

	return layout;

ERROR:
	_E("Failed to create layout");

	if (layout) {
		evas_object_del(layout);
		layout = NULL;
	}

	if (swipe_layout) {
		evas_object_del(swipe_layout);
		swipe_layout = NULL;
	}

	return NULL;
}

static Evas_Object *_comformant_create(void)
{
	Evas_Object *conformant = NULL;
	Evas_Object *win = NULL;

	win = lock_window_win_get();
	retv_if(!win, NULL);

	conformant = elm_conformant_add(win);
	retv_if(!conformant, NULL);

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, conformant);

	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "elm");

	evas_object_show(conformant);

	return conformant;
}

lock_error_e lock_default_lock_init(void)
{
	Evas_Object *conformant = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *bg = NULL;

	int ret = 0;

	layout = _layout_create();
	goto_if(!layout, ERROR);

	s_info.layout = layout;

	conformant = _comformant_create();
	goto_if(!conformant, ERROR);
	s_info.conformant = conformant;

	ea_object_event_callback_add(layout, EA_CALLBACK_BACK, _default_lock_hw_back_cb, NULL);

	bg = lock_background_view_bg_get();
	if (!bg) {
		_E("Failed to get BG");
	} else {
		elm_object_part_content_set(layout, "sw.bg", bg);
	}

	ret = _unlock_panel_create();
	goto_if(LOCK_ERROR_OK != ret, ERROR);

	return LOCK_ERROR_OK;

ERROR:
	_E("Failed to initialize default lock");

	if (conformant) {
		evas_object_del(conformant);
		conformant = NULL;
	}

	if (layout) {
		evas_object_del(layout);
		layout = NULL;
	}

	return LOCK_ERROR_FAIL;
}

void lock_default_lock_fini(void)
{
	/* delete network status */
	lock_sim_state_deinit();

	/* delete batteyr information */
	lock_battery_fini();

	/* delete data&time information */
	lock_time_fini();

	/* delete wallpaper */
	lock_background_view_bg_del();

	if (s_info.mouse_down_handler) {
		ecore_event_handler_del(s_info.mouse_down_handler);
		s_info.mouse_down_handler = NULL;
	}

	if (s_info.mouse_move_handler) {
		ecore_event_handler_del(s_info.mouse_move_handler);
		s_info.mouse_move_handler = NULL;
	}

	if (s_info.mouse_up_handler) {
		ecore_event_handler_del(s_info.mouse_up_handler);
		s_info.mouse_up_handler = NULL;
	}

	if (s_info.swipe_layout) {
		evas_object_del(s_info.swipe_layout);
		s_info.swipe_layout = NULL;
	}

	if (s_info.conformant) {
		evas_object_del(s_info.conformant);
		s_info.conformant = NULL;
	}

	if (s_info.layout) {
		evas_object_del(s_info.layout);
		s_info.layout = NULL;
	}
}
