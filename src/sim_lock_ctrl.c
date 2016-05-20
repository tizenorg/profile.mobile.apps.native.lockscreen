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

static Ecore_Event_Handler *handler_list[3];
static Evas_Object *main_view;
static Evas_Object *pass_view;

enum sim_lock_response_e{
	SIM_LOCK_SIM_RESPONSE_UNKNOWN,
	SIM_LOCK_SIM_RESPONSE_UNLOCKED,
	SIM_LOCK_SIM_RESPONSE_PIN_INCORRECT,
	SIM_LOCK_SIM_RESPONSE_CARD_BLOCKED,
};

extern int LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED;
extern int LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT;
extern int LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED;

static Eina_Bool _sim_lock_response(void *data, int type, void *event_info)
{
	int result = (int)data;

	if (!pass_view) {
		ERR("pass view is not created");

		return ECORE_CALLBACK_DONE;
	}

	DBG("Sim lock response: %d", result);

	switch(result) {
	case SIM_LOCK_SIM_RESPONSE_UNLOCKED:
		elm_object_signal_emit(main_view, "simlock,hide", "lockscreen");
		break;
	case SIM_LOCK_SIM_RESPONSE_PIN_INCORRECT:
		elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_INCORRECT_PIN"));
		lockscreen_password_view_clear(pass_view);
		break;
	case SIM_LOCK_SIM_RESPONSE_CARD_BLOCKED:
		elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_CARD_BLOCKED"));
		lockscreen_password_view_clear(pass_view);
		break;
	default:
		DBG("Not supported sim result");
		break;
	}

	return ECORE_CALLBACK_DONE;
}

static void
_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	DBG("Accept button clicked");

	lockscreen_sim_lock_unlock(event_info);
}

static void _lockscreen_sim_lock_view_create(Evas_Object *view)
{
	int result = lockscreen_sim_lock_pin_required(&card_num);

	main_view = view;

	DBG("Cards locked: %d", result);

	if (result) {
		pass_view = lockscreen_password_view_create(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, main_view);
		lockscreen_password_view_pin_password_length_set(pass_view, PIN_DEFAULT_LENGTH);

		handler_list[0] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED,
				_sim_lock_response, (void *)SIM_LOCK_SIM_RESPONSE_UNLOCKED);
		handler_list[1] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT,
				_sim_lock_response, (void *)SIM_LOCK_SIM_RESPONSE_PIN_INCORRECT);
		handler_list[2] = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED,
				_sim_lock_response, (void *)SIM_LOCK_SIM_RESPONSE_CARD_BLOCKED);

		evas_object_smart_callback_add(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED,
				_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked, NULL);

		lockscreen_main_view_part_content_set(main_view, PART_SIMLOCK, pass_view);

		if (result > 1) {
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_SIM_CARD_PIN"));
			elm_object_signal_emit(pass_view, "layout,pin_icon,show", "lockscreen");
		} else
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PIN"));
	}
}

int lockscreen_sim_lock_ctrl_init(Evas_Object *main_view)
{
	if (lockscreen_sim_lock_init()) {
		ERR("lockscreen_sim_lock_init failed");
		return 1;
	}

	_lockscreen_sim_lock_view_create(main_view);

	DBG("Sim lock init succeed");
	return 0;
}

void lockscreen_sim_lock_ctrl_shutdown(void)
{
	int i;
	for (i = 0; i < SIZE(handler_list); ++i)
		ecore_event_handler_del(handler_list[i]);

	lockscreen_sim_lock_shutdown();
}
