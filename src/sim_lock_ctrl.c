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

#include "log.h"
#include "util.h"
#include "sim_lock.h"
#include "password_view.h"
#include "main_view.h"

#define PIN_DEFAULT_LENGTH 4
#define PUK_DEFAULT_LENGTH 8

#define TOAST_POPUP_TIMEOUT 3.0 //sec

static Ecore_Event_Handler *handler_list[3];
static Evas_Object *main_view;
static Evas_Object *pass_view;

static void _sim_lock_ctrl_view_create(Evas_Object *view, int locked_count, sim_lock_e unlock_card_num, pin_type_e type);
static void _sim_lock_ctrl_view_label_set(pin_type_e type);
static void _sim_lock_ctrl_popup_create(Evas_Object *win, char *title, char *desc);
static void _sim_lock_ctrl_popup_prepare(Evas_Object *win, int attempts);

static Eina_Bool _sim_lock_response(void *data, int type, void *event_info)
{
	Evas_Object *layout = NULL;
	int locked_left = 0;
	int attempts_left = lockscreen_sim_lock_get_attempts_left();
	sim_lock_e first_locked_card = SIM_LOCK_NONE;

	pin_type_e pin_type = SIM_LOCK_PIN_TYPE_NONE;

	if (!pass_view) {
		ERR("pass view is not created");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (type ==  LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED) {
		lockscreen_password_view_clear(pass_view);
		locked_left = lockscreen_sim_lock_pin_required(&first_locked_card, &pin_type);

		if (locked_left && first_locked_card != SIM_LOCK_NONE)
			_sim_lock_ctrl_view_create(main_view, locked_left, first_locked_card, pin_type);
		else {
			layout = lockscreen_main_view_part_content_unset(main_view, PART_SIMLOCK);
			evas_object_del(layout);
		}

	} else if (type == LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT) {
		_sim_lock_ctrl_popup_prepare(main_view, attempts_left);

		lockscreen_password_view_clear(pass_view);
	} else if (type == LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED) {
		_sim_lock_ctrl_popup_prepare(main_view, 0);
		_sim_lock_ctrl_view_label_set(SIM_LOCK_PIN_TYPE_CARD_BLOCKED);
		lockscreen_password_view_clear(pass_view);
	} else
		DBG("Not supported sim response result");

	return ECORE_CALLBACK_PASS_ON;
}

static void
_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	DBG("Accept button clicked");

	lockscreen_sim_lock_unlock((sim_lock_e)data, event_info);
}

static void _sim_lock_ctrl_view_icon_set(int unlock_card_num)
{
	if (lockscreen_sim_lock_available_sim_card_count() > 1) {
		switch (unlock_card_num) {
		case SIM_LOCK_SIM1:
			elm_object_signal_emit(pass_view, SIGNAL_SIM_ICON_SHOW_SIM1, "lockscreen");
			break;
		case SIM_LOCK_SIM2:
			elm_object_signal_emit(pass_view, SIGNAL_SIM_ICON_SHOW_SIM2, "lockscreen");
			break;
		default:
			elm_object_signal_emit(pass_view, SIGNAL_SIM_ICON_SHOW, "lockscreen");
		}
	} else
		elm_object_signal_emit(pass_view, SIGNAL_SIM_ICON_SHOW, "lockscreen");
}

static void _sim_lock_ctrl_view_label_set(pin_type_e type)
{
	if (lockscreen_sim_lock_available_sim_card_count() > 1) {
		switch (type) {
		case SIM_LOCK_PIN_TYPE_PIN:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_SIM_CARD_PIN"));
			break;
		case SIM_LOCK_PIN_TYPE_PUK:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_SIM_CARD_PUK"));
			break;
		case SIM_LOCK_PIN_TYPE_CARD_BLOCKED:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_SIM_CARD_BLOCKED"));
			break;
		default:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PIN"));
			break;
		}
	} else
		elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PIN"));

}

static void _sim_lock_ctrl_pass_view_create(Evas_Object *main_view)
{
	pass_view = lockscreen_password_view_create(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, main_view);
	lockscreen_password_view_btn_cancel_hide(pass_view);

	lockscreen_main_view_part_content_set(main_view, PART_SIMLOCK, pass_view);

	handler_list[0] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED,
			_sim_lock_response, NULL);
	handler_list[1] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT,
			_sim_lock_response, NULL);
	handler_list[2] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED,
			_sim_lock_response, NULL);
}

static void
_sim_lock_ctrl_view_create(Evas_Object *view, int locked_count, sim_lock_e unlock_card_num, pin_type_e type)
{
	if (!pass_view) {
		_sim_lock_ctrl_pass_view_create(view);

		evas_object_smart_callback_add(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED,
				_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked,
				(void *)unlock_card_num);
	} else {
		evas_object_smart_callback_del(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED,
				_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked);
		evas_object_smart_callback_add(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED,
				_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked,
				(void *)unlock_card_num);
	}

	if (type == SIM_LOCK_PIN_TYPE_PUK)
		lockscreen_password_view_pin_password_length_set(pass_view, PUK_DEFAULT_LENGTH);
	else
		lockscreen_password_view_pin_password_length_set(pass_view, PIN_DEFAULT_LENGTH);

	_sim_lock_ctrl_view_icon_set(unlock_card_num);
	_sim_lock_ctrl_view_label_set(type);
}

static void _popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = data;

	if (popup)
		evas_object_del(popup);
	else
		evas_object_del(obj);
}

static void _sim_lock_ctrl_popup_prepare(Evas_Object *win, int attempts)
{
	char buf[512] = {0, };

	if (!attempts) {
		snprintf(buf, SIZE(buf), _("IDS_LCKSCN_POP_YOU_HAVE_ATTEMPTED_TO_UNLOCK_THE_DEVICE_INCORRECTLY_P1SD_TIMES_YOU_HAVE_P2SD_ATTEMPTS_LEFT_BEFORE_THE_DEVICE_IS_RESET_TO_FACTORY_MSG") , 3 - attempts, attempts);

		_sim_lock_ctrl_popup_create(win, _("IDS_LCKSCN_HEADER_INCORRECT_PIN_ABB"), buf);

	} else {
		snprintf(buf, SIZE(buf), _("IDS_LCKSCN_POP_INCORRECT_PIN_ENTERED_P1SD_ATTEMPTS_LEFT_TRY_AGAIN") , attempts);

		_sim_lock_ctrl_popup_create(win, NULL, buf);
	}
}

static void _sim_lock_ctrl_popup_create(Evas_Object *win, char *title, char *desc)
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

int lockscreen_sim_lock_ctrl_init(Evas_Object *view)
{
	sim_lock_e first_locked_card = SIM_LOCK_NONE;
	pin_type_e pin_type = SIM_LOCK_PIN_TYPE_NONE;
	int locked_count = 0;

	if (lockscreen_sim_lock_init()) {
		ERR("lockscreen_sim_lock_init failed");
		return 1;
	}

	main_view = view;
	locked_count = lockscreen_sim_lock_pin_required(&first_locked_card, &pin_type);

	if (locked_count)
		_sim_lock_ctrl_view_create(main_view, locked_count, first_locked_card, pin_type);

	return 0;
}

void lockscreen_sim_lock_ctrl_shutdown(void)
{
	int i;
	for (i = 0; i < SIZE(handler_list); ++i)
		ecore_event_handler_del(handler_list[i]);

	if (pass_view)
		evas_object_smart_callback_del(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED,
				_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked);

	lockscreen_sim_lock_shutdown();
}
