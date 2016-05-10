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

#include "lockscreen.h"
#include "log.h"
#include "device_lock.h"
#include "app_control.h"

#include <app.h>
#include <Ecore.h>

static Ecore_Event_Handler *handler;
static Eina_List *unlock_requests;

static void
_lockscren_app_control_device_reply(app_control_h request, bool result)
{
	bool requested;
	app_control_h answer;

	int err = app_control_is_reply_requested(request, &requested);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_is_reply_requested failed: %s", get_error_message(err));
		return;
	}

	if (!requested) return;

	err = app_control_create(&answer);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_create failed: %s", get_error_message(err));
		return;
	}
	err = app_control_reply_to_launch_request(answer, request, result ? APP_CONTROL_RESULT_SUCCEEDED : APP_CONTROL_RESULT_FAILED);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_reply_to_launch_request failed: %s", get_error_message(err));
	}
	app_control_destroy(answer);
	DBG("Replied to app_control launch request, result: %d", result);
}

static void
_lockscreen_app_control_device_lock_handle(app_control_h request)
{
	int ret = lockscreen_device_lock_lock();
	if ((ret == 0) || (ret == 1)) 
		_lockscren_app_control_device_reply(request, true);
	else
		_lockscren_app_control_device_reply(request, false);
}

static void
_lockscreen_app_control_device_unlock_handle(app_control_h request)
{
	/* If device is not locked exit lockscreen immideatly */
	if (!lockscreen_device_is_locked()) {
		_lockscren_app_control_device_reply(request, true);
		ui_app_exit();
	} else {
		app_control_h cloned;
		app_control_clone(&cloned, request);
		unlock_requests = eina_list_append(unlock_requests, cloned);
		lockscreen_device_lock_unlock_request();
	}
}

void lockscreen_app_control_ctrl_handle(app_control_h request)
{
	char *operation = NULL;

	int ret = app_control_get_operation(request, &operation);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_get_operation failed: %s", get_error_message(ret));
		return;
	}

	DBG("App_control request: %s", operation);

	if (!operation)
		operation = strdup(APP_CONTROL_OPERATION_DEFAULT);

	if (!strcmp(operation, APP_CONTROL_OPERATION_DEFAULT) ||
		!strcmp(operation, "http://tizen.org/appcontrol/operation/lock")) {
		_lockscreen_app_control_device_lock_handle(request);
	} else if (!strcmp(operation, "http://tizen.org/appcontrol/operation/unlock")) {
		_lockscreen_app_control_device_unlock_handle(request);
	}

	free(operation);
}

int lockscreen_app_control_ctrl_init(void)
{
	if (lockscreen_device_lock_init()) {
		ERR("lockscreen_device_lock_init failed");
		return 1;
	}
	return 0;
}

void lockscreen_app_control_ctrl_shutdown(void)
{
	app_control_h request;
	EINA_LIST_FREE(unlock_requests, request) {
		_lockscren_app_control_device_reply(request, true);
		app_control_destroy(request);
	}
	ecore_event_handler_del(handler);
	lockscreen_device_lock_shutdown();
}
