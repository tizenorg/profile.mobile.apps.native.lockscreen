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

#include <telephony/telephony.h>
#include <Elementary.h>
#include <app_control.h>

#include "log.h"
#include "call.h"

static int init_count;
static telephony_handle_list_s handle_list;

int LOCKSCREEN_EVENT_CALL_STATUS_CHANGED;

static void _call_status_changed_cb(telephony_h handle, telephony_noti_e noti_id, void *data, void *user_data)
{
	ecore_event_add(LOCKSCREEN_EVENT_CALL_STATUS_CHANGED, NULL, NULL, NULL);
}

static void _lockscreen_call_unregister_callbacks(void)
{
	int i;
	for (i = 0; i < handle_list.count; ++i)
		telephony_unset_noti_cb(handle_list.handle[i],
		       TELEPHONY_NOTI_VOICE_CALL_STATUS_IDLE);
}

static int _lockscreen_call_register_callbacks(void)
{
	int i;
	for (i = 0; i < handle_list.count; ++i) {
		int j;
		for (j = TELEPHONY_NOTI_VOICE_CALL_STATUS_IDLE; j <= TELEPHONY_NOTI_VIDEO_CALL_STATUS_INCOMING; ++j) {
			int ret = telephony_set_noti_cb(handle_list.handle[i], j, _call_status_changed_cb, NULL);
			if (ret != TELEPHONY_ERROR_NONE) {
				ERR("telephony_set_noti_cb failed");
				_lockscreen_call_unregister_callbacks();
				continue;
			}
		}
	}

	return 0;
}

static void
call_app_launch_request_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	switch (result) {
		case APP_CONTROL_RESULT_APP_STARTED:
		case APP_CONTROL_RESULT_SUCCEEDED:
			DBG("Call application launch successed.");
			break;
		case APP_CONTROL_RESULT_FAILED:
		case APP_CONTROL_RESULT_CANCELED:
			DBG("Call application launch failed.");
			break;
	}
}

int lockscreen_call_app_launch_request(void)
{
	DBG("Launch %s", CALL_APP_CONTROL_CALL_APP_ID);
	app_control_h control = NULL;

	int ret = app_control_create(&control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_create failed");
		return -1;
	}

	ret = app_control_set_app_id(control, CALL_APP_CONTROL_CALL_APP_ID);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_set_app_id failed[%d]: %s", ret, get_error_message(ret));
		app_control_destroy(control);
		return -1;
	}

	ret = app_control_set_operation(control, CALL_APP_CONTROL_OPERATION_RESUME);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_set_operation failed[%d]: %s", ret, get_error_message(ret));
		app_control_destroy(control);
		return -1;
	}

	ret = app_control_send_launch_request(control, call_app_launch_request_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_send_launch_request failed[%d]: %s", ret, get_error_message(ret));
		app_control_destroy(control);
		return -1;
	}

	app_control_destroy(control);

	return 0;
}

int lockscreen_call_active_is(void)
{
	telephony_call_state_e call_state;
	int ret = TELEPHONY_ERROR_NONE;

	int i;
	for (i = 0; i < handle_list.count; ++i) {

		ret = telephony_call_get_voice_call_state(handle_list.handle[i], &call_state);
		if (ret != TELEPHONY_ERROR_NONE) {
			ERR("telephony_call_get_voice_call_state failed[%d]: %s", ret, get_error_message(ret));
			continue;
		}

		if (call_state != TELEPHONY_CALL_STATE_IDLE)
			return 1;

		ret = telephony_call_get_video_call_state(handle_list.handle[i], &call_state);
		if (ret != TELEPHONY_ERROR_NONE) {
			ERR("telephony_call_get_voice_call_state failed[%d]: %s", ret, get_error_message(ret));
			continue;
		}

		if (call_state != TELEPHONY_CALL_STATE_IDLE)
			return 1;
	}

	DBG("No active calls");

	return 0;
}

int lockscreen_call_init(void)
{
	if (!init_count) {

		int ret = telephony_init(&handle_list);
		if (ret != TELEPHONY_ERROR_NONE) {
			ERR("telephony_init failed: %s", get_error_message(ret));
			return -1;
		}

		if (_lockscreen_call_register_callbacks()) {
			ERR("_lockscreen_call_register_callbacks failed");
			telephony_deinit(&handle_list);
			return -1;
		}

		LOCKSCREEN_EVENT_CALL_STATUS_CHANGED = ecore_event_type_new();
	}
	init_count++;
	return 0;
}

void lockscreen_call_shutdown(void)
{
	if (init_count) {

		init_count--;

		if (!init_count) {
			_lockscreen_call_unregister_callbacks();
			telephony_deinit(&handle_list);
		}
	}
}
