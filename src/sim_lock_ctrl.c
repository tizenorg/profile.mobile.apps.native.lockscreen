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

#include "log.h"
#include "util.h"
#include "sim_lock.h"
#include "password_view.h"
#include "main_view.h"

#define PIN_DEFAULT_LENGTH 4
#define PUK_DEFAULT_LENGTH 8

static Ecore_Event_Handler *handler_list[3];
static Evas_Object *main_view;
static Evas_Object *pass_view;
static int init_pin_request_count;

extern int LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED;
extern int LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT;
extern int LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED;

static void _sim_lock_ctrl_view_create(Evas_Object *view, int locked_count, int unlock_card_num, pin_type_e type);

static Eina_Bool _sim_lock_response(void *data, int type, void *event_info)
{
	int first_locked_card = -1;
	int locked_remaining = 0;
	pin_type_e pin_type = SIM_LOCK_PIN_TYPE_NONE;

	if (!pass_view) {
		ERR("pass view is not created");
		return ECORE_CALLBACK_DONE;
	}

	if(type == LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED) {
		DBG("Sim lock response UNLOCKED");

		first_locked_card = -1;
		lockscreen_password_view_clear(pass_view);

		locked_remaining = lockscreen_sim_lock_pin_required(&first_locked_card, &pin_type);
		if (locked_remaining && first_locked_card >= 0)
			_sim_lock_ctrl_view_create(main_view, locked_remaining, first_locked_card, pin_type);
		else
			elm_object_signal_emit(main_view, "simlock,hide", "lockscreen");

	} else if (type ==  LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT) {
		DBG("Sim lock response INCORRECT PIN");

		elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_INCORRECT_PIN"));
		lockscreen_password_view_clear(pass_view);

	} else if (type ==  LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED) {
		DBG("Sim lock response BLOCKED");

		elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_CARD_BLOCKED"));
		lockscreen_password_view_clear(pass_view);

	} else
		DBG("Not supported sim result");

	return ECORE_CALLBACK_DONE;
}

static void
_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	DBG("Accept button clicked");

	lockscreen_sim_lock_unlock((int)data, event_info);
}

static void _sim_lock_ctrl_view_icon_set(int unlock_card_num)
{
	if (init_pin_request_count > 1) {
		switch (unlock_card_num) {
		case 0:
			elm_object_signal_emit(pass_view, "layout,pin_icon,show,1", "lockscreen");
			break;
		case 1:
			elm_object_signal_emit(pass_view, "layout,pin_icon,show,2", "lockscreen");
			break;
		default:
			elm_object_signal_emit(pass_view, "layout,pin_icon,show", "lockscreen");
		}
	} else
		elm_object_signal_emit(pass_view, "layout,pin_icon,show", "lockscreen");
}

static void _sim_lock_ctrl_view_label_set(pin_type_e type)
{
	if (init_pin_request_count > 1) {
		switch (type) {
		case SIM_LOCK_PIN_TYPE_PIN:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_SIM_CARD_PIN"));
			break;
		case SIM_LOCK_PIN_TYPE_PUK:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_SIM_CARD_PUK"));
			break;
		default:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PIN"));
			break;
		}
	} else
		elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PIN"));
}

static Evas_Object *_sim_lock_ctrl_pass_view_create(Evas_Object *main_view)
{
	Evas_Object *pass_view = lockscreen_password_view_create(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, main_view);

	lockscreen_main_view_part_content_set(main_view, PART_SIMLOCK, pass_view);

	handler_list[0] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED,
			_sim_lock_response, NULL);
	handler_list[1] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT,
			_sim_lock_response, NULL);
	handler_list[2] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED,
			_sim_lock_response, NULL);

	return pass_view;
}

static void
_sim_lock_ctrl_view_create(Evas_Object *view, int locked_count, int unlock_card_num, pin_type_e type)
{
	DBG("locked %d", locked_count);
	if (!pass_view) {
		pass_view = _sim_lock_ctrl_pass_view_create(view);

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

int lockscreen_sim_lock_ctrl_init(Evas_Object *view)
{
	int result = -1;
	int first_locked_card = -1;
	pin_type_e pin_type = SIM_LOCK_PIN_TYPE_NONE;

	if (lockscreen_sim_lock_init()) {
		ERR("lockscreen_sim_lock_init failed");
		return 1;
	}

	main_view = view;
	result = lockscreen_sim_lock_pin_required(&first_locked_card, &pin_type);
	init_pin_request_count = result;

	if (result && first_locked_card >= 0)
		_sim_lock_ctrl_view_create(main_view, result, first_locked_card, pin_type);

	DBG("Sim lock init succeed");
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
