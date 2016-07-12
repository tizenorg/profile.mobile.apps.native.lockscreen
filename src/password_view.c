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

#include "password_view.h"
#include "log.h"
#include "lockscreen.h"
#include "util.h"

#include <Elementary.h>
#include <Edje.h>
#include <Ecore.h>

#define PIN_PASS_LEN_FILTER "__pass_len_key"

static void
_lockscreen_password_view_cancel_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_CANCEL_BUTTON_CLICKED, NULL);
}

static void
_lockscreen_password_view_pin_button_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	const char *txt = edje_object_part_text_get(obj, "text.number");
	Evas_Object *entry = elm_object_part_content_get(data, "sw.entry");
	elm_entry_entry_insert(entry, txt);
}

static void
_lockscreen_password_view_clear_button_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *entry = elm_object_part_content_get(data, "sw.entry");
	Evas_Object *tbl = elm_entry_textblock_get(entry);
	Evas_Textblock_Cursor *cursor = evas_object_textblock_cursor_get(tbl);
	evas_textblock_cursor_char_prev(cursor);
	evas_textblock_cursor_char_delete(cursor);
}

static void
_lockscreen_password_pin_button_setup(Evas_Object *ly, int row, int col, const char *text, const char *subtext, Edje_Signal_Cb cb)
{
	Evas_Object *edje = elm_layout_edje_get(ly);
	Evas_Object *child = edje_object_part_table_child_get(edje, "pinpad", col, row);
	edje_object_part_text_set(child, "text.number", text);
	edje_object_part_text_set(child, "text.letters", subtext);
	if (cb) edje_object_signal_callback_add(child, "button,clicked", "pinpaditem", cb, ly);
}

static Evas_Object* _lockscreen_password_view_layout_create(Evas_Object *parent, const char *group_name)
{
	Evas_Object *ly = elm_layout_add(parent);
	if (!elm_layout_file_set(ly, util_get_res_file_path(LOCK_EDJE_FILE), group_name)) {
		ERR("elm_layout_file_set failed.");
		evas_object_del(ly);
		return NULL;
	}

	/* Load theme extension */
	util_lockscreen_theme_get();

	Evas_Object *btn = elm_button_add(ly);
	if (!elm_object_style_set(btn, "panel"))
		ERR("elm_object_style_set failed");
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL"));
	evas_object_smart_callback_add(btn, "clicked", _lockscreen_password_view_cancel_button_clicked, ly);

	elm_object_part_content_set(ly, "sw.cancel", btn);
	evas_object_show(btn);
	evas_object_show(ly);

	return ly;
}

static void
_lockscreen_password_view_entry_activated(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Key_Down *kd = event_info;
	if (kd && !strcmp(kd->key, "Return")) {
		evas_object_smart_callback_call(data, SIGNAL_ACCEPT_BUTTON_CLICKED, (void*)elm_entry_entry_get(obj));
	}
}

static void
_lockscreen_password_view_pin_entry_activated(void *data, Evas_Object *obj, void *event_info)
{
	const char *content = elm_entry_entry_get(obj);
	Elm_Entry_Filter_Limit_Size *filter = evas_object_data_get(data, PIN_PASS_LEN_FILTER);
	evas_object_smart_callback_call(data, SIGNAL_PASSWORD_TYPING, NULL);
	if (content && strlen(content) == filter->max_char_count)
		evas_object_smart_callback_call(data, SIGNAL_ACCEPT_BUTTON_CLICKED, (void*)elm_entry_entry_get(obj));
}

static void
_lockscreen_password_view_password_entry_changed(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_PASSWORD_TYPING, NULL);
}

static Evas_Object* _lockscreen_password_view_pin_create(Evas_Object *parent)
{
	Evas_Object *ly = _lockscreen_password_view_layout_create(parent, "lock-simple-password");
	if (!ly) return NULL;

	Evas_Object *entry = elm_entry_add(ly);
	elm_entry_password_set(entry, EINA_TRUE);
	elm_object_part_content_set(ly, "sw.entry", entry);
	elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
	evas_object_pass_events_set(entry, EINA_TRUE);
	evas_object_show(entry);

	// Setup PIN pad
	_lockscreen_password_pin_button_setup(ly, 1, 1, "1", NULL, _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 1, 2, "2", "ABC" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 1, 3, "3", "DEF" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 2, 1, "4", "GHI" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 2, 2, "5", "JKL" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 2, 3, "6", "MNO" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 3, 1, "7", "PQRS" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 3, 2, "8", "TUV" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 3, 3, "9", "WXYZ" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 4, 3, NULL, NULL, _lockscreen_password_view_clear_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 4, 2, "0", NULL , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 4, 1, NULL, NULL , NULL);

	elm_object_signal_emit(ly, "layout,pinpad,show", "lockscreen");
	elm_entry_text_style_user_push(entry, "DEFAULT='font=Sans style=Regular color=#FFFFFF font_size=110 wrap=none align=center'");
	evas_object_smart_callback_add(entry, "changed", _lockscreen_password_view_pin_entry_activated, ly);

	return ly;
}

static Evas_Object* _lockscreen_password_view_password_create(Evas_Object *parent)
{
	Evas_Object *ly = _lockscreen_password_view_layout_create(parent, "lock-password");
	if (!ly) return NULL;

	Evas_Object *entry = elm_entry_add(ly);
	elm_entry_password_set(entry, EINA_TRUE);
	elm_object_part_content_set(ly, "sw.entry", entry);
	evas_object_show(entry);
	elm_entry_context_menu_disabled_set(entry, EINA_TRUE);

	evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_DOWN, _lockscreen_password_view_entry_activated, ly);
	elm_entry_text_style_user_push(entry, "DEFAULT='font=Sans style=Regular color=#FFFFFF font_size=90 wrap=none align=center'");
	elm_entry_input_panel_enabled_set(entry, EINA_TRUE);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	lockscreen_password_view_pin_password_length_set(ly, 4);
	evas_object_smart_callback_add(entry, "changed,user", _lockscreen_password_view_password_entry_changed, ly);

	return ly;
}

void lockscreen_password_view_keyboard_show(Evas_Object *view)
{
	Evas_Object *entry = elm_object_part_content_get(view, "sw.entry");
	if (!entry) return;
	elm_object_focus_set(entry, EINA_TRUE);
}

static Evas_Event_Flags _lockscreen_password_view_swipe_state_end(void *data, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_ACCEPT_BUTTON_CLICKED, NULL);
	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Object* _lockscreen_password_view_swipe_create(Evas_Object *parent)
{
	Evas_Object *ly = _lockscreen_password_view_layout_create(parent, "lock-base");
	if (!ly) return NULL;

	Evas_Object *gesture_layer = elm_gesture_layer_add(ly);
	elm_gesture_layer_hold_events_set(gesture_layer, EINA_TRUE);
	elm_gesture_layer_attach(gesture_layer, ly);
	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _lockscreen_password_view_swipe_state_end, ly);
	elm_gesture_layer_flick_time_limit_ms_set(gesture_layer, 500);
	// set minimum swipe length scaled by edje scale factor
	elm_gesture_layer_line_min_length_set(gesture_layer, 140 * edje_scale_get() / edje_object_scale_get(elm_layout_edje_get(ly)));
	evas_object_show(gesture_layer);

	return ly;
}

void lockscreen_password_view_btn_cancel_hide(Evas_Object *view)
{
	if (!view) {
		ERR("layout == NULL");
		return;
	}

	elm_object_signal_emit(view, "btn,cancel,hide", "lockscreen");
}

Evas_Object *lockscreen_password_view_create(lockscreen_password_view_type type, Evas_Object *parent)
{
	Evas_Object *ret = NULL;
	switch (type) {
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN:
			ret = _lockscreen_password_view_pin_create(parent);
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD:
			ret = _lockscreen_password_view_password_create(parent);
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_SWIPE:
			ret = _lockscreen_password_view_swipe_create(parent);
			break;
	}
	return ret;
}

void lockscreen_password_view_clear(Evas_Object *view)
{
	Evas_Object *entry = elm_object_part_content_get(view, "sw.entry");
	if (!entry) {
		ERR("elm_object_part_content_get failed");
		return;
	}
	elm_entry_entry_set(entry, "");
}

static void
_lockscreen_password_view_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	free(data);
}

void lockscreen_password_view_pin_password_length_set(Evas_Object *view, int length)
{
	Elm_Entry_Filter_Limit_Size *filter = evas_object_data_get(view, PIN_PASS_LEN_FILTER);
	if (!filter) {
		filter = malloc(sizeof(Elm_Entry_Filter_Limit_Size));
		evas_object_data_set(view, PIN_PASS_LEN_FILTER, filter);
		elm_entry_markup_filter_append(elm_object_part_content_get(view, "sw.entry"), elm_entry_filter_limit_size, filter);
		evas_object_event_callback_add(view, EVAS_CALLBACK_DEL, _lockscreen_password_view_del, filter);
	}
	filter->max_byte_count = 0;
	filter->max_char_count = length;
}
