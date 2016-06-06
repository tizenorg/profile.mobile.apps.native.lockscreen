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
#include <vconf.h>

#include "device_lock.h"
#include "device_lock_ctrl.h"
#include "main_view.h"
#include "log.h"
#include "password_view.h"
#include "window.h"
#include "util.h"

static Ecore_Event_Handler *handler[1];
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

static void _lockscreen_device_lock_ctrl_pass_view_failed_show(Evas_Object *pass_view, int attempts)
{
	char buf[128], tmp[32];
	char *trans = NULL;

	switch (lockscreen_device_lock_type_get()) {
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			trans = _("IDS_COM_BODY_INCORRECT_PIN");
			break;
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			trans = _("IDS_IDLE_BODY_INCORRECT_PASSWORD");
			break;
		default:
			ERR("Unahandled lock type");
			return;
	}

	DBG("Unlock attempts left: %d", attempts);

	if (attempts == -1) {
		snprintf(buf, sizeof(buf), "%s", trans);
	} else if (attempts == 0) {
		snprintf(buf, sizeof(buf), "%s<br><small>No attempts left.</>", trans);
	} else if (attempts == 1) {
		snprintf(buf, sizeof(buf), "%s<br><small>%s</>", trans, _("IDS_IDLE_BODY_1_ATTEMPT_LEFT"));
	} else if (attempts > 1) {
		snprintf(tmp, sizeof(tmp), _("IDS_IDLE_BODY_PD_ATTEMPTS_LEFT"), attempts);
		snprintf(buf, sizeof(buf), "%s<br><small>%s</>", trans, tmp);
	}
	elm_object_part_text_set(pass_view, PART_TEXT_TITLE, buf);
	lockscreen_password_view_clear(pass_view);
}

static void _lockscreen_device_lock_ctrl_pass_view_accept_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	int att_left;
	Evas_Object *pass_view;
	lockscreen_device_unlock_result_e ret = lockscreen_device_lock_unlock(event_info, &att_left);

	switch (ret) {
		case LOCKSCREEN_DEVICE_UNLOCK_SUCCESS:
			break;
		case LOCKSCREEN_DEVICE_UNLOCK_FAILED:
			pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
			if (!pass_view) FAT("lockscreen_main_view_part_content_get failed");
			_lockscreen_device_lock_ctrl_pass_view_failed_show(pass_view, att_left);
			break;
		case LOCKSCREEN_DEVICE_UNLOCK_ERROR:
			ERR("Unlocking error occured");
			break;
	}
}

static void _lockscreen_device_lock_ctrl_unlock_panel_show(lockscreen_device_lock_type_e type, const lockscreen_event_t *event)
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
			lockscreen_password_view_pin_password_length_set(pass_view, 4);
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PASSWORD"));
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_SWIPE:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_LCKSCN_POP_SWIPE_SCREEN_TO_UNLOCK"));
			break;
		default:
			FAT("unhandled view type");
			break;
	}

	if (event) {
		elm_object_part_content_set(pass_view, PART_CONTENT_EVENT, event);
	}
}

static void _lockscreen_device_lock_ctrl_unlock_request(lockscreen_event_t *ev)
{
	lockscreen_device_lock_type_e type = lockscreen_device_lock_type_get();

	switch (type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			_lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_SWIPE, ev);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			_lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, ev);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			_lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD, ev);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PATTERN:
			WRN("Unhandled lock type");
			break;
	}
}

static void _lockscreen_device_lock_ctrl_swipe_finished(void *data, Evas_Object *obj, void *event)
{
	lockscreen_device_lock_type_e type = lockscreen_device_lock_type_get();

	switch (type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			lockscreen_device_lock_unlock(NULL, NULL);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PIN:
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			lockscreen_device_lock_ctrl_unlock_request(NULL);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PATTERN:
			WRN("Unhandled lock type");
			break;
	}
}

static void _lockscreen_device_vconf_idle_key_changed(keynode_t *node, void *user_data)
{
	if (node->value.i == VCONFKEY_IDLE_UNLOCK)
		lockscreen_device_lock_ctrl_unlock_request(NULL);
}

int lockscreen_device_lock_ctrl_init(Evas_Object *view)
{
	if (lockscreen_device_lock_init()) {
		ERR("lockscreen_device_lock_init failed");
		return 1;
	}

	switch (lockscreen_device_lock_type_get()) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			lockscreen_window_quickpanel_block_set(EINA_FALSE);
			break;
		default:
			lockscreen_window_quickpanel_block_set(EINA_TRUE);
	}

	vconf_notify_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed, NULL);

	handler[0] = ecore_event_handler_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, _lockscreen_device_lock_ctrl_unlocked, NULL);
	evas_object_smart_callback_add(view, SIGNAL_SWIPE_GESTURE_FINISHED, _lockscreen_device_lock_ctrl_swipe_finished, NULL);
	main_view = view;
	return 0;
}

void lockscreen_device_lock_ctrl_shutdown()
{
	vconf_ignore_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed);
	ecore_event_handler_del(handler[0]);
	lockscreen_device_lock_shutdown();
}

int lockscreen_device_lock_ctrl_unlock_request(void)
{
	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) return 0;
	return _lockscreen_device_lock_ctrl_unlock_request(NULL);
}
