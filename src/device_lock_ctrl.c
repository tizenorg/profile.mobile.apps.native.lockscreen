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
#include "util_time.h"
#include "events_view.h"
#include "time_format.h"


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
	char buf[128] = { 0, };
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
	elm_object_part_text_set(pass_view, "text.subtitle", buf);

	lockscreen_password_view_clear(pass_view);
}

static void _lockscreen_device_lock_ctrl_pass_view_accept_button_clicked(void *data, Evas_Object  *obj, void *event_info)
{
	int att_left;
	Evas_Object *pass_view;
	lockscreen_event_t *event = data;
	lockscreen_device_unlock_result_e ret = lockscreen_device_lock_unlock(event_info, &att_left);

	switch (ret) {
		case LOCKSCREEN_DEVICE_UNLOCK_SUCCESS:
			if (event) {
				if (!lockscreen_event_launch(event, NULL))
					ERR("lockscreen_event_launch failed");
			}
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

static Evas_Object*
_lockscreen_device_lock_miniature_create(Evas_Object *parent, const lockscreen_event_t *event)
{
	Evas_Object *icon, *ret = elm_layout_add(parent);
	char *time;

	if (!elm_layout_theme_set(ret, "layout", "noti", "default")) {
		FAT("elm_layout_theme_set failed");
		evas_object_del(ret);
		return NULL;
	}

	if (lockscreen_event_icon_get(event)) {
		icon = elm_icon_add(ret);
		elm_image_fill_outside_set(icon, EINA_TRUE);
		elm_image_file_set(icon, lockscreen_event_icon_get(event), NULL);
		evas_object_show(icon);
		elm_object_part_content_set(ret, NOTI_ITEM_ICON, icon);
	}
	if (lockscreen_event_sub_icon_get(event)) {
		icon = elm_icon_add(ret);
		elm_image_file_set(icon, lockscreen_event_sub_icon_get(event), NULL);
		evas_object_show(icon);
		elm_object_part_content_set(ret, NOTI_ITEM_ICON_SUB, icon);
	}
	elm_object_part_text_set(ret, NOTI_ITEM_TEXT, lockscreen_event_title_get(event));
	elm_object_part_text_set(ret, NOTI_ITEM_TEXT_SUB, lockscreen_event_content_get(event));
	const char *locale = lockscreen_time_format_locale_get();
	const char *timezone = lockscreen_time_format_timezone_get();
	bool use24hformat = lockscreen_time_format_use_24h();
	time = util_time_string_get(lockscreen_event_time_get(event), locale, timezone, use24hformat);
	elm_object_part_text_set(ret, NOTI_ITEM_TEXT_TIME, time);
	free(time);

	evas_object_show(ret);
	return ret;
}

static void
_lockscreen_device_lock_ctrl_view_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	lockscreen_event_free(data);
}

static int _lockscreen_device_lock_ctrl_unlock_panel_show(lockscreen_device_lock_type_e type, const lockscreen_event_t *event)
{
	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) return 1;
	lockscreen_event_t *copy = NULL;

	if (event) {
		copy = lockscreen_event_copy(event);
	}

	pass_view = lockscreen_password_view_create(type, main_view);
	evas_object_smart_callback_add(pass_view, SIGNAL_CANCEL_BUTTON_CLICKED, _lockscreen_device_lock_ctrl_pass_view_cancel_button_clicked, NULL);
	evas_object_smart_callback_add(pass_view, SIGNAL_ACCEPT_BUTTON_CLICKED, _lockscreen_device_lock_ctrl_pass_view_accept_button_clicked, copy);
	lockscreen_main_view_part_content_set(main_view, PART_PASSWORD, pass_view);
	if (copy)
		evas_object_event_callback_add(pass_view, EVAS_CALLBACK_DEL,
				_lockscreen_device_lock_ctrl_view_del, copy);

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
		Evas_Object *mini = _lockscreen_device_lock_miniature_create(pass_view, event);
		elm_object_part_content_set(pass_view, PART_CONTENT_EVENT, mini);
	}
	return 0;
}

static int _lockscreen_device_lock_ctrl_unlock_request(const lockscreen_event_t *ev)
{
	lockscreen_device_lock_type_e type = lockscreen_device_lock_type_get();

	switch (type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			return _lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_SWIPE, ev);
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			return _lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, ev);
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
			return _lockscreen_device_lock_ctrl_unlock_panel_show(LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD, ev);
		case LOCKSCREEN_DEVICE_LOCK_PATTERN:
			WRN("Unhandled lock type");
			return 1;
	}
	return 1;
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
			lockscreen_device_lock_ctrl_unlock_request();
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
			lockscreen_device_lock_ctrl_unlock_request();
	}
}

int lockscreen_device_lock_ctrl_init(Evas_Object *view)
{
	if (lockscreen_device_lock_init()) {
		ERR("lockscreen_device_lock_init failed");
		return 1;
	}

	if (lockscreen_events_init()) {
		ERR("lockscreen_events_init failed");
		lockscreen_device_lock_shutdown();
		return 1;
	}

	if (lockscreen_time_format_init()) {
		ERR("lockscreen_time_format_init failed");
		lockscreen_events_shutdown();
		lockscreen_device_lock_shutdown();
		return 1;
	}

	switch (lockscreen_device_lock_type_get()) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			lockscreen_window_quickpanel_block_set(EINA_FALSE);
			break;
		default:
			lockscreen_window_quickpanel_block_set(EINA_TRUE);
	}

	int err = vconf_notify_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed, NULL);
	if (err) {
		ERR("vconf_notify_key_changed failed: %s", get_error_message(err));
	}

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
	if (!main_view) return 1;
	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) return 1;
	return _lockscreen_device_lock_ctrl_unlock_request(NULL);
}

int lockscreen_device_lock_ctrl_unlock_and_launch_request(const lockscreen_event_t *event)
{
	if (!main_view) return 1;
	Evas_Object *pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) return 1;
	return _lockscreen_device_lock_ctrl_unlock_request(event);
}
