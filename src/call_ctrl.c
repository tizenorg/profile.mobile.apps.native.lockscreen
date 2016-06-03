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

#include <Elementary.h>

#include "log.h"
#include "util.h"
#include "call.h"
#include "password_view.h"
#include "main_view.h"
#include "swipe_icon.h"
#include "device_lock.h"

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;

static void _call_icon_view_update(void)
{
	Evas_Object *pass_view = NULL;
	Evas_Object *call_icon_view = NULL;

	int call_active = lockscreen_call_status_active();

	pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (pass_view) {
		if (call_active)
			lockscreen_password_view_btn_return_to_call_show(pass_view);
		else
			lockscreen_password_view_btn_return_to_call_hide(pass_view);
	}

	if (lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_NONE && !call_active) {
		call_icon_view = lockscreen_main_view_part_content_unset(main_view, PART_CALL);
		if (call_icon_view)
			evas_object_del(call_icon_view);
		lockscreen_main_view_plmn_text_center_set(main_view, false);

		return;
	}

	call_icon_view = lockscreen_main_view_part_content_get(main_view, PART_CALL);
	if (!call_icon_view)
		call_icon_view = lockscreen_swipe_icon_view_create(main_view, ICON_CALL);

	if (call_active)
		lockscreen_swipe_icon_text_set(call_icon_view, _("IDS_LCKSCN_BUTTON_RETURN_TO_CALL_ABB"));
	else
		lockscreen_swipe_icon_text_set(call_icon_view, _("IDS_LCKSCN_BODY_EMERGENCY_CALL"));

	lockscreen_main_view_plmn_text_center_set(main_view, true);
	lockscreen_main_view_part_content_set(main_view, PART_CALL, call_icon_view);
}

static Eina_Bool _call_status_changed_cb(void *data, int type, void *event_info)
{
	if (!main_view) {
		ERR("main_view == NULL");
		return ECORE_CALLBACK_PASS_ON;
	}

	_call_icon_view_update();

	return ECORE_CALLBACK_PASS_ON;
}

static void _call_icon_selected(void *data, Evas_Object *obj, void *event)
{
	lockscreen_call_app_launch_request();
}

int lockscreen_call_ctrl_init(Evas_Object *view)
{
	Evas_Object *call_icon_view = NULL;

	if (lockscreen_call_init()) {
		ERR("lockscreen_call_init failed");
		return 1;
	}

	main_view = view;
	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_CALL_STATUS_CHANGED, _call_status_changed_cb, NULL);

	_call_icon_view_update();
	evas_object_smart_callback_add(call_icon_view, SIGNAL_CALL_SELECTED, _call_icon_selected, NULL);

	return 0;
}

void lockscreen_call_ctrl_shutdown(void)
{
	ecore_event_handler_del(handler);

	Evas_Object *call_icon_view = lockscreen_main_view_part_content_unset(main_view, PART_CALL);
	if (call_icon_view) {
		evas_object_del(call_icon_view);
		evas_object_smart_callback_del(call_icon_view, SIGNAL_CALL_SELECTED, _call_icon_selected);
	}

	lockscreen_call_shutdown();
}
