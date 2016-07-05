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
#include "events_view.h"
#include "display.h"
#include "call.h"
#include "util.h"

static Ecore_Event_Handler *handler[3];
static Evas_Object *main_view;

static void _lockscreen_device_lock_ctrl_view_unlocked(void *data, Evas_Object *obj, void *event)
{
	ui_app_exit();
}

static void _lockscreen_device_lock_ctrl_pin_unlock_hide(void)
{
	Evas_Object *pin_view = lockscreen_main_view_part_content_unset(main_view, PART_PASSWORD);
	if (pin_view) evas_object_del(pin_view);
}

static Eina_Bool _lockscreen_device_lock_ctrl_unlocked(void *data, int event, void *event_info)
{
	/* When swipe finished play unlock animation and exit */
	_lockscreen_device_lock_ctrl_pin_unlock_hide();
	lockscreen_window_background_fade();
	evas_object_smart_callback_add(main_view, SIGNAL_UNLOCK_ANIMATION_FINISHED, _lockscreen_device_lock_ctrl_view_unlocked, NULL);
	lockscreen_main_view_unlock(main_view);
	return EINA_TRUE;
}

static void _lockscreen_device_lock_ctrl_pass_view_cancel_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	_lockscreen_device_lock_ctrl_pin_unlock_hide();
	lockscreen_device_lock_unlock_cancel();
}

static void _lockscreen_device_lock_ctrl_pass_view_failed_show(Evas_Object *pass_view, int attempts)
{
	char buf[256] = { 0, };
	char *trans = NULL;

	switch (lockscreen_device_lock_type_get()) {
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			trans = _("IDS_COM_BODY_INCORRECT_PIN");
			break;
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			trans = _("IDS_IDLE_BODY_INCORRECT_PASSWORD");
		default:
			ERR("Unahandled lock type");
			return;
	}

	DBG("Unlock attempts left: %d", attempts);
	elm_object_part_text_set(pass_view, PART_TEXT_TITLE, trans);

	if (attempts == -1) {
		// nothing
	} else if (attempts == 0) {
		snprintf(buf, sizeof(buf), "%s", _("IDS_ST_NO_ATTEMPTS"));
	} else if (attempts == 1) {
		int max = lockscreen_device_lock_max_unlock_attempts_get();
		snprintf(buf, sizeof(buf), _("IDS_LCKSCN_POP_YOU_HAVE_ATTEMPTED_TO_UNLOCK_THE_DEVICE_INCORRECTLY_P1SD_TIMES_YOU_HAVE_P2SD_ATTEMPTS_LEFT_BEFORE_THE_DEVICE_IS_RESET_TO_FACTORY_MSG"), max - attempts, attempts);
		util_popup_create(elm_object_top_widget_get(pass_view), _("IDS_ST_AUTO_FACTORY_RESET"), buf);
		snprintf(buf, sizeof(buf), "%s", _("IDS_IDLE_BODY_1_ATTEMPT_LEFT"));
	} else if (attempts > 1) {
		snprintf(buf, sizeof(buf), _("IDS_IDLE_BODY_PD_ATTEMPTS_LEFT"), attempts);
	}
	elm_object_part_text_set(pass_view, PART_TEXT_SUBTITLE, buf);

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

static void
_lockscreen_device_lock_pass_view_password_is_typing(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_display_timer_renew();
}

static void _lockscreen_device_lock_ctrl_pass_view_return_to_call_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	int ret = lockscreen_call_app_launch_request();
	if (ret)
		ERR("Could not send launch request");
}

static Eina_Bool
_lockscreen_device_lock_ctrl_unlock_panel_show(lockscreen_password_view_type type, const lockscreen_device_unlock_context_t *context)
{
	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) return EINA_TRUE;

	pass_view = lockscreen_password_view_create(type, main_view);
	evas_object_smart_callback_add(pass_view, SIGNAL_CANCEL_BUTTON_CLICKED, _lockscreen_device_lock_ctrl_pass_view_cancel_button_clicked, NULL);
	evas_object_smart_callback_add(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED, _lockscreen_device_lock_ctrl_pass_view_accept_button_clicked, NULL);
	evas_object_smart_callback_add(pass_view, SIGNAL_RETURN_TO_CALL_BUTTON_CLICKED, _lockscreen_device_lock_ctrl_pass_view_return_to_call_button_clicked, NULL);

	if (lockscreen_call_active_is())
		lockscreen_password_view_btn_return_to_call_show(pass_view);

	lockscreen_main_view_part_content_set(main_view, PART_PASSWORD, pass_view);

	switch (type) {
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PIN"));
			lockscreen_password_view_pin_password_length_set(pass_view, 4);
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_COM_BODY_ENTER_PASSWORD"));
			evas_object_smart_callback_add(pass_view, SIGNAL_PASSWORD_TYPING, _lockscreen_device_lock_pass_view_password_is_typing, NULL);
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_SWIPE:
			elm_object_part_text_set(pass_view, PART_TEXT_TITLE, _("IDS_LCKSCN_POP_SWIPE_SCREEN_TO_UNLOCK"));
			break;
		default:
			FAT("unhandled view type");
			break;
	}

	if (context && context->type == LOCKSCREEN_DEVICE_UNLOCK_CONTEXT_EVENT) {
		Evas_Object *mini = lockscreen_events_view_event_miniature_create(pass_view,
				context->data.event);
		elm_object_part_content_set(pass_view, PART_CONTENT_EVENT, mini);
	}

	if (type == LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD)
		lockscreen_password_view_keyboard_show(pass_view);

	return EINA_TRUE;
}

static Eina_Bool
_lockscreen_device_lock_ctrl_unlock_request(void *data, int event, void *ev)
{
	lockscreen_device_lock_type_e type = lockscreen_device_lock_type_get();
	lockscreen_device_unlock_context_t *ctx = ev;

	switch (type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			return _lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_SWIPE, ctx);
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			return _lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, ctx);
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			return _lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD, ctx);
		case LOCKSCREEN_DEVICE_LOCK_PATTERN:
			WRN("Unhandled lock type");
			return 1;
	}
	return EINA_TRUE;
}

static Eina_Bool _lockscreen_device_lock_ctrl_btn_return_update(void *data, int event, void *event_info)
{
	int active_call = lockscreen_call_active_is();

	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) {
		if (active_call)
			lockscreen_password_view_btn_return_to_call_show(pass_view);
		else
			lockscreen_password_view_btn_return_to_call_hide(pass_view);
	}

	return EINA_TRUE;
}

static void _lockscreen_device_lock_ctrl_swipe_finished(void *data, Evas_Object *obj, void *event)
{
	lockscreen_device_lock_type_e type = lockscreen_device_lock_type_get();

	switch (type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			if (!lockscreen_device_lock_unlock_request(NULL))
				lockscreen_device_lock_unlock(NULL, NULL);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PIN:
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			lockscreen_device_lock_unlock_request(NULL);
			break;
		case LOCKSCREEN_DEVICE_LOCK_PATTERN:
			WRN("Unhandled lock type");
			break;
	}
}

static void _lockscreen_device_vconf_idle_key_changed(keynode_t *node, void *user_data)
{
	if (node->value.i == VCONFKEY_IDLE_UNLOCK) {
		if (lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_NONE)
			ui_app_exit();
		else
			lockscreen_device_lock_unlock_request(NULL);
	}
}

int lockscreen_device_lock_ctrl_init(Evas_Object *view)
{
	if (lockscreen_device_lock_init()) {
		ERR("lockscreen_device_lock_init failed");
		return 1;
	}

	if (lockscreen_display_init()) {
		ERR("lockscreen_display_init failed");
	}

	if (lockscreen_call_init()) {
		ERR("lockscreen_call_init failed");
	}

	int err = vconf_notify_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed, NULL);
	if (err) {
		ERR("vconf_notify_key_changed failed: %s", get_error_message(err));
	}

	handler[0] = ecore_event_handler_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, _lockscreen_device_lock_ctrl_unlocked, NULL);
	handler[1] = ecore_event_handler_add(LOCKSCREEN_EVENT_CALL_STATUS_CHANGED, _lockscreen_device_lock_ctrl_btn_return_update, NULL);
	handler[2] = ecore_event_handler_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST, _lockscreen_device_lock_ctrl_unlock_request, NULL);

	evas_object_smart_callback_add(view, SIGNAL_SWIPE_GESTURE_FINISHED, _lockscreen_device_lock_ctrl_swipe_finished, NULL);
	main_view = view;
	return 0;
}

void lockscreen_device_lock_ctrl_shutdown()
{
	vconf_ignore_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed);
	ecore_event_handler_del(handler[0]);
	ecore_event_handler_del(handler[1]);
	ecore_event_handler_del(handler[2]);
	lockscreen_device_lock_shutdown();
	lockscreen_display_shutdown();
}
