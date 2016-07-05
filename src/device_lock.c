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

#include "device_lock.h"
#include "log.h"

#include <Ecore.h>
#include <vconf.h>
#include <auth-passwd.h>
#include <dpm/password.h>
#include <app.h>

static int init_count;
static lockscreen_device_lock_type_e lock_type;
static int unlock_mutex;
static lockscreen_device_unlock_context_t *context;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_CANCELLED;

static void _lockscreen_device_unlock(void);

static int _lockscreen_device_lock_dpm_status_set(int status)
{
	dpm_context_h handle;
	dpm_password_policy_h password_policy_handle;

	handle = dpm_context_create();
	if (!handle) {
		ERR("dpm_context_create failed");
		return -1;
	}

	password_policy_handle = dpm_context_acquire_password_policy(handle);
	if (!password_policy_handle) {
		ERR("dpm_context_acquire_password_policy failed");
		dpm_context_destroy(handle);
		return -1;
	}

	if (dpm_password_set_status(password_policy_handle, status) != DPM_ERROR_NONE) {
		ERR("dpm_password_set_status failed");
		dpm_context_release_password_policy(handle, password_policy_handle);
		dpm_context_destroy(handle);
		return -1;
	}

	dpm_context_release_password_policy(handle, password_policy_handle);
	dpm_context_destroy(handle);
	return 0;
}

static void _lockscreen_device_vconf_idle_key_changed(keynode_t *node, void *user_data)
{
	if (node->value.i == VCONFKEY_IDLE_UNLOCK) {
		if (lockscreen_device_lock_type_get() == LOCKSCREEN_DEVICE_LOCK_NONE)
			lockscreen_device_lock_unlock();
		else
			lockscreen_device_lock_unlock_request(NULL);
	}
}

int lockscreen_device_lock_init(void)
{
	int type;
	if (!init_count) {
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED = ecore_event_type_new();
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST = ecore_event_type_new();
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_CANCELLED  = ecore_event_type_new();
		int err = vconf_notify_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed, NULL);
		if (err) {
			ERR("vconf_notify_key_changed failed: %s", get_error_message(err));
		}

		int ret = vconf_get_int(VCONFKEY_SETAPPL_SCREEN_LOCK_TYPE_INT, &type);
		if (ret) {
			ERR("vconf_get_int failed");
			lock_type = LOCKSCREEN_DEVICE_LOCK_NONE;
		} else {
			switch (type) {
				case SETTING_SCREEN_LOCK_TYPE_NONE:
				case SETTING_SCREEN_LOCK_TYPE_SWIPE:
					lock_type = LOCKSCREEN_DEVICE_LOCK_NONE;
					break;
				case SETTING_SCREEN_LOCK_TYPE_PASSWORD:
					lock_type = LOCKSCREEN_DEVICE_LOCK_PASSWORD;
					break;
				case SETTING_SCREEN_LOCK_TYPE_SIMPLE_PASSWORD:
					lock_type = LOCKSCREEN_DEVICE_LOCK_PIN;
					break;
				default:
					lock_type = LOCKSCREEN_DEVICE_LOCK_NONE;
					break;
			}
		}
	}
	init_count++;
	return 0;
}

void lockscreen_device_lock_shutdown(void)
{
	if (init_count) {
		init_count--;
		vconf_ignore_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed);
	}
}

lockscreen_device_lock_type_e lockscreen_device_lock_type_get(void)
{
	return lock_type;
}

static void
_lockscreen_device_password_refresh(void)
{
	ERR("Unahandled password change...");
}

static lockscreen_device_auth_result_e
_lockscreen_device_unlock_with_password(const char *pass, int *attempts_left)
{
	unsigned int current_attempt, max_attempts, expire_sec;

	int ret = auth_passwd_check_passwd(AUTH_PWD_NORMAL, pass, &current_attempt, &max_attempts, &expire_sec);
	switch (ret) {
		case AUTH_PASSWD_API_SUCCESS:
		case AUTH_PASSWD_API_ERROR_NO_PASSWORD:
			return LOCKSCREEN_DEVICE_AUTH_SUCCESS;
		case AUTH_PASSWD_API_ERROR_PASSWORD_EXPIRED:
			_lockscreen_device_password_refresh();
			break;
		case AUTH_PASSWD_API_ERROR_PASSWORD_MISMATCH:
		case AUTH_PASSWD_API_ERROR_PASSWORD_RETRY_TIMER:
		case AUTH_PASSWD_API_ERROR_PASSWORD_MAX_ATTEMPTS_EXCEEDED:
			DBG("auth_passwd_check_passwd : %d %d %d", current_attempt, max_attempts, expire_sec);
			if (max_attempts == 0) {
				*attempts_left = -1; //infinite
			} else {
				*attempts_left = current_attempt > max_attempts ? 0 : max_attempts - current_attempt;
				if (current_attempt == max_attempts) {
					_lockscreen_device_lock_dpm_status_set(DPM_PASSWORD_STATUS_MAX_ATTEMPTS_EXCEEDED);
				}
			}
			return LOCKSCREEN_DEVICE_AUTH_FAILED;
		case AUTH_PASSWD_API_ERROR_ACCESS_DENIED:
		case AUTH_PASSWD_API_ERROR_SOCKET:
		case AUTH_PASSWD_API_ERROR_INPUT_PARAM:
			ERR("auth_passwd_check_passwd failed: %d", ret);
			return LOCKSCREEN_DEVICE_AUTH_ERROR;
	}

	return LOCKSCREEN_DEVICE_AUTH_ERROR;
}

lockscreen_device_auth_result_e lockscreen_device_lock_authenticate(const char *pass, int *attempts_left)
{
	lockscreen_device_auth_result_e ret = LOCKSCREEN_DEVICE_AUTH_ERROR;
	int al = -1;

	switch (lockscreen_device_lock_type_get()) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			ret = LOCKSCREEN_DEVICE_AUTH_SUCCESS;
			break;
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			ret = _lockscreen_device_unlock_with_password(pass, &al);
			break;
		default:
			break;
	}
	if (ret == LOCKSCREEN_DEVICE_AUTH_SUCCESS)
		_lockscreen_device_unlock();

	if (attempts_left) *attempts_left = al;

	return ret;
}

int lockscreen_device_lock_max_unlock_attempts_get(void)
{
	unsigned int current_attempt, max_attempts, expire_sec;
	int ret = auth_passwd_check_passwd_state(AUTH_PWD_NORMAL, &current_attempt, &max_attempts, &expire_sec);
	switch (ret) {
		case AUTH_PASSWD_API_ERROR_NO_PASSWORD:
			return 0;
		case AUTH_PASSWD_API_SUCCESS:
			return max_attempts;
		default:
			return -1;
	}
	return -1;
}

static lockscreen_device_unlock_context_t*
_lockscreen_device_unlock_context_copy(const lockscreen_device_unlock_context_t *context)
{
	if (!context) return NULL;

	lockscreen_device_unlock_context_t *ret = calloc(1, sizeof(lockscreen_device_unlock_context_t));

	ret->type = context->type;

	switch (context->type) {
		case LOCKSCREEN_DEVICE_UNLOCK_CONTEXT_LAUNCH_EVENT:
			ret->data.event = lockscreen_event_copy(context->data.event);
		default:
			break;
	}

	return ret;
}

static void
_lockscreen_context_dummy_free(void *fn, void  *data)
{
}

int lockscreen_device_lock_unlock_request(const lockscreen_device_unlock_context_t *ctx)
{
	if (unlock_mutex) return 1;

	if (ctx) {
		context = _lockscreen_device_unlock_context_copy(ctx);
		if (!context) {
			ERR("_lockscreen_device_unlock_context_copy failed");
			return 1;
		}
	} else
		context = NULL;

	ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST, context,
			_lockscreen_context_dummy_free, NULL);
	unlock_mutex = 1;
	return 0;
}

static void
_lockscreen_context_free(void *fn, void  *data)
{
	lockscreen_device_unlock_context_t *ctx = data;
	if (!ctx) return;

	switch (ctx->type) {
		case LOCKSCREEN_DEVICE_UNLOCK_CONTEXT_LAUNCH_EVENT:
			lockscreen_event_free(ctx->data.event);
		default:
			break;
	}

	free(ctx);
}

int lockscreen_device_lock_unlock_request_cancel(void)
{
	if (!unlock_mutex) return 1;

	ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_CANCELLED, context, _lockscreen_context_free, NULL);
	unlock_mutex = 0;
	return 0;
}

static void _lockscreen_device_unlock(void)
{
	if (!unlock_mutex) return;

	ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, context, _lockscreen_context_free, NULL);
	unlock_mutex = 0;
}

int lockscreen_device_lock_unlock(void)
{
	unsigned int attempt, max_attempts, expire_sec;

	int ret = auth_passwd_check_passwd_state(AUTH_PWD_NORMAL, &attempt, &max_attempts,
			&expire_sec);
	if (ret == AUTH_PASSWD_API_ERROR_NO_PASSWORD) {
		ui_app_exit();
	} else if (ret == AUTH_PASSWD_API_SUCCESS && expire_sec > 0 && attempt <= max_attempts) {
		ui_app_exit();
	} else {
		DBG("Authentication required before unlock");
		return 1;
	}
	return 0;
}
