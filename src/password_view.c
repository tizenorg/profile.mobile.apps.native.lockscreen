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

static void 
_lockscreen_password_view_cancel_button_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	evas_object_smart_callback_call(data, SIGNAL_CANCEL_BUTTON_CLICKED, NULL);
}

static void
_lockscreen_password_view_pin_button_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	DBG("PIN button clicked");
	const char *txt = edje_object_part_text_get(obj, "text.number");
	Evas_Object *entry = elm_object_part_content_get(data, "sw.entry");
	elm_entry_entry_append(entry, txt);
}

static void
_lockscreen_password_view_accept_button_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *entry = elm_object_part_content_get(data, "sw.entry");
	evas_object_smart_callback_call(data, SIGNAL_ACCEPT_BUTTON_CLICKED, (void*)elm_entry_entry_get(entry));
}

static void
_lockscreen_password_view_clear_button_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	DBG("Clear button clicked");
	Evas_Object *entry = elm_object_part_content_get(data, "sw.entry");
	evas_object_focus_set(entry, EINA_TRUE);
	Evas_Object *tb = elm_entry_textblock_get(entry);
	Evas_Textblock_Cursor *cur = evas_object_textblock_cursor_get(tb);
	evas_textblock_cursor_char_delete(cur);
}

static void
_lockscreen_password_pin_button_setup(Evas_Object *ly, int row, int col, const char *text, const char *subtext, Edje_Signal_Cb cb)
{
	Evas_Object *edje = elm_layout_edje_get(ly);
	Evas_Object *child = edje_object_part_table_child_get(edje, "pinpad", col, row);
	if (text) edje_object_part_text_set(child, "text.number", text);
	if (subtext) edje_object_part_text_set(child, "text.letters", subtext);
	if (cb) edje_object_signal_callback_add(child, "button,clicked", "pinpaditem", cb, ly);
}

static Evas_Object* _lockscreen_password_view_layout_create(Evas_Object *parent)
{
	Evas_Object *ly = elm_layout_add(parent);
	if (!elm_layout_file_set(ly, util_get_res_file_path(LOCK_EDJE_FILE), "lock-simple-password")) {
		ERR("elm_layout_file_set failed.");
		evas_object_del(ly);
		return NULL;
	}

	Evas_Object *entry = elm_entry_add(ly);
	elm_entry_password_set(entry, EINA_TRUE);
	elm_object_part_content_set(ly, "sw.entry", entry);
	elm_entry_text_style_user_push(entry, "DEFAULT='font=Sans style=Regular color=#FFFFFF font_size=90 wrap=none align=center'");
	evas_object_show(entry);

	elm_object_signal_callback_add(ly, "cancel,button,clicked", "lock-simple-password", _lockscreen_password_view_cancel_button_clicked, ly);
	evas_object_show(ly);

	return ly;
}

static void
_lockscreen_password_view_entry_activated(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Key_Down *kd = event_info;
	DBG("Entry key: %s %s %u", kd->key, kd->keyname, kd->timestamp);
	DBG("Ecore: %d %d", (unsigned int)ecore_time_unix_get(), (unsigned int)ecore_time_get());
	if (kd && !strcmp(kd->key, "Return")) {
		evas_object_smart_callback_call(data, SIGNAL_ACCEPT_BUTTON_CLICKED, (void*)elm_entry_entry_get(obj));
	}
}

static Evas_Object* _lockscreen_password_view_pin_create(Evas_Object *parent)
{
	Evas_Object *ly = _lockscreen_password_view_layout_create(parent);
	if (!ly) return NULL;

	// Setup PIN pad
	_lockscreen_password_pin_button_setup(ly, 1, 1, "1", "", _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 1, 2, "2", "ABC" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 1, 3, "3", "DEF" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 2, 1, "4", "GHI" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 2, 2, "5", "JKL" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 2, 3, "6", "MNO" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 3, 1, "7", "PQRS" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 3, 2, "8", "TUV" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 3, 3, "9", "WXYZ" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 4, 1, NULL, NULL, _lockscreen_password_view_clear_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 4, 2, "0", "" , _lockscreen_password_view_pin_button_clicked);
	_lockscreen_password_pin_button_setup(ly, 4, 3, "✓", "" , _lockscreen_password_view_accept_button_clicked);

	elm_object_signal_emit(ly, "layout,pinpad,show", "lockscreen");

	return ly;
}

static Evas_Object* _lockscreen_password_view_password_create(Evas_Object *parent)
{
	Evas_Object *ly = _lockscreen_password_view_layout_create(parent);
	if (!ly) return NULL;

	elm_object_signal_emit(ly, "layout,keyboard,show", "lockscreen");

	Evas_Object *entry = elm_object_part_content_get(ly, "sw.entry");
	evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_DOWN, _lockscreen_password_view_entry_activated, ly);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_input_panel_show(entry);
	elm_object_focus_set(entry, EINA_TRUE);

	return ly;
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
	}
	return ret;
}
