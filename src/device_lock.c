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

static int init_count;
static lockscreen_device_lock_type_e lock_type;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED;

static void _lockscreen_device_unlock(void)
{
	ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, NULL, NULL, NULL);
}

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

int lockscreen_device_lock_init(void)
{
	int type;
	if (!init_count) {
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED = ecore_event_type_new();
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

static lockscreen_device_unlock_result_e
_lockscreen_device_unlock_with_password(const char *pass, int *attempts_left)
{
	unsigned int current_attempt, max_attempts, expire_sec;

	int ret = auth_passwd_check_passwd(AUTH_PWD_NORMAL, pass, &current_attempt, &max_attempts, &expire_sec);
	switch (ret) {
		case AUTH_PASSWD_API_SUCCESS:
		case AUTH_PASSWD_API_ERROR_NO_PASSWORD:
			return LOCKSCREEN_DEVICE_UNLOCK_SUCCESS;
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
			return LOCKSCREEN_DEVICE_UNLOCK_FAILED;
		case AUTH_PASSWD_API_ERROR_ACCESS_DENIED:
		case AUTH_PASSWD_API_ERROR_SOCKET:
		case AUTH_PASSWD_API_ERROR_INPUT_PARAM:
			ERR("auth_passwd_check_passwd failed: %d", ret);
			return LOCKSCREEN_DEVICE_UNLOCK_ERROR;
	}

	return LOCKSCREEN_DEVICE_UNLOCK_ERROR;
}

lockscreen_device_unlock_result_e lockscreen_device_lock_unlock(const char *pass, int *attempts_left)
{
	lockscreen_device_unlock_result_e ret = LOCKSCREEN_DEVICE_UNLOCK_ERROR;
	int al = -1;

	switch (lockscreen_device_lock_type_get()) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			ret = LOCKSCREEN_DEVICE_UNLOCK_SUCCESS;
			break;
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			ret = _lockscreen_device_unlock_with_password(pass, &al);
			break;
		default:
			break;
	}
	if (ret == LOCKSCREEN_DEVICE_UNLOCK_SUCCESS)
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
