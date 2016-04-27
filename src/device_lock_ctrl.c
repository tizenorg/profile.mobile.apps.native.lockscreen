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

static void _lockscreen_device_lock_ctrl_pin_unlock_handle(void)
{
	Evas_Object *pin_view = lockscreen_password_view_create(LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN, main_view);

	lockscreen_main_view_part_content_set(main_view, PART_PASSWORD, pin_view);
}

static Eina_Bool _lockscreen_device_lock_ctrl_unlock_request(void *data, int event, void *event_info)
{
	lockscreen_device_lock_type_e type = lockscreen_device_lock_type_get();

	ERR("Unlock request");
	switch (type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			if (lockscreen_device_lock_unlock(NULL))
				ERR("lockscreen_device_lock_unlock failed");
			break;
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			_lockscreen_device_lock_ctrl_pin_unlock_handle();
			break;
		case LOCKSCREEN_DEVICE_LOCK_NUMBER:
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
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
