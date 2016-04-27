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

#include <Ecore.h>
#include <app.h>

#include "device_lock.h"
#include "main_view.h"
#include "log.h"
#include "password_view.h"

static Ecore_Event_Handler *handler[2];
static Evas_Object *main_view;

static void _lockscreen_device_lock_ctrl_view_unlocked(void *data, Evas_Object *obj, void *event)
{
	ui_app_exit();
}

static Eina_Bool _lockscreen_device_lock_ctrl_unlocked(void *data, int event, void *event_info)
{
	/* When swipe finished play unlock animation and exit */
	evas_object_smart_callback_add(main_view, SIGNAL_UNLOCK_ANIMATION_FINISHED, _lockscreen_device_lock_ctrl_view_unlocked, NULL);
	lockscreen_main_view_unlock(main_view);
	return EINA_TRUE;
}

static void _lockscreen_device_lock_ctrl_pin_unlock_hide(void)
{
	Evas_Object *pin_view = lockscreen_main_view_part_content_unset(main_view, PART_PASSWORD);
	if (pin_view) evas_object_del(pin_view);
}

static void _lockscreen_device_lock_ctrl_pass_view_cancel_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	_lockscreen_device_lock_ctrl_pin_unlock_hide();
}

static void _lockscreen_device_lock_ctrl_pass_view_accept_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	int att_left;
	lockscreen_device_unlock_result_e ret = lockscreen_device_lock_unlock(event_info, &att_left);
	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (!pass_view) FAT("lockscreen_main_view_part_content_get failed");

	switch (ret) {
		case LOCKSCREEN_DEVICE_UNLOCK_SUCCESS:
			break;
		case LOCKSCREEN_DEVICE_UNLOCK_FAILED:
			if (lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_PIN)
				elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_INCORRECT_PIN"));
			else if (lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_PASSWORD) {
				elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_IDLE_BODY_INCORRECT_PASSWORD"));
			}
			lockscreen_password_view_clear(pass_view);
			break;
		case LOCKSCREEN_DEVICE_UNLOCK_ERROR:
			ERR("Unlocking error occured");
			break;
	}
}

static void _lockscreen_device_lock_ctrl_unlock_panel_show(lockscreen_device_lock_type_e type)
{
	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) return;

	pass_view = lockscreen_password_view_create(type, main_view);
	evas_object_smart_callback_add(pass_view, SIGNAL_CANCEL_BUTTON_CLICKED, _lockscreen_device_lock_ctrl_pass_view_cancel_button_clicked, NULL);
	evas_object_smart_callback_add(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED, _lockscreen_device_lock_ctrl_pass_view_accept_button_clicked, NULL);
	lockscreen_main_view_part_content_set(main_view, PART_PASSWORD, pass_view);

	switch (type) {
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PIN"));
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PASSWORD"));
			break;
		default:
			FAT("Unhandled view type");
			break;
	}
	elm_object_part_text_set(pass_view, PART_TEXT_CANCEL, _("IDS_ST_BUTTON_CANCEL"));
}

static Eina_Bool _lockscreen_device_lock_ctrl_unlock_request(void *data, int event, void *event_info)
{
	lockscreen_device_lock_type_e type = lockscreen_device_lock_type_get();

	switch (type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			if (lockscreen_device_lock_unlock(NULL, NULL))
				ERR("lockscreen_device_lock_unlock failed");
			break;
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			_lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			_lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PATTERN:
			WRN("Unhandled lock type");
			break;
	}

	return EINA_TRUE;
}

static void _lockscreen_device_lock_ctrl_swipe_finished(void *data, Evas_Object *obj, void *event)
{
	lockscreen_device_lock_unlock_request();
}

int lockscreen_device_lock_ctrl_init(Evas_Object *view)
{
	if (lockscreen_device_lock_init()) {
		ERR("lockscreen_device_lock_init failed");
		return 1;
	}

	handler[0] = ecore_event_handler_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST, _lockscreen_device_lock_ctrl_unlock_request, NULL);
	handler[1] = ecore_event_handler_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, _lockscreen_device_lock_ctrl_unlocked, NULL);

	evas_object_smart_callback_add(view, SIGNAL_SWIPE_GESTURE_FINISHED, _lockscreen_device_lock_ctrl_swipe_finished, NULL);
	main_view = view;
	return 0;
}

void lockscreen_device_lock_ctrl_shutdown()
{
	ecore_event_handler_del(handler[0]);
	ecore_event_handler_del(handler[1]);
	lockscreen_device_lock_shutdown();
}
