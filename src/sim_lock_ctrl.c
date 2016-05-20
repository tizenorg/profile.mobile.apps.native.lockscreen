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
#include "sim_lock.h"
#include "password_view.h"
#include "main_view.h"

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;
extern int LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED;

static Eina_Bool _sim_lock_unlocked(void *data, int type, void *event_info)
{
	elm_object_signal_emit(main_view, "simlock,hide", "lockscreen");

	return ECORE_CALLBACK_DONE;
}

static void
_lockscreen_sim_lock_ctrl_pass_view_accept_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	DBG("Accept button clicked");

	lockscreen_sim_lock_unlock(event_info);
	elm_object_signal_emit(main_view, "simlock,hide", "lockscreen");
}

static void _lockscreen_sim_lock_view_create(Evas_Object *view)
{
	Evas_Object *pass_view = NULL;
	int result = lockscreen_sim_lock_pin_required();

	main_view = view;

	DBG("Cards locked: %d", result);

	result = 2;
	if (result) {
		pass_view = lockscreen_password_view_create(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, main_view);
		handler = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED, _sim_lock_unlocked, pass_view);
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
	lockscreen_sim_lock_shutdown();
}
