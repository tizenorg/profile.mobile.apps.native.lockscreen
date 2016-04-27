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

static int init_count;
static lockscreen_device_lock_type_e lock_type;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED;

static void _lockscreen_device_unlock(void)
{
	ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, NULL, NULL, NULL);
}

static void _lockscreen_device_vconf_idle_key_changed(keynode_t *node, void *user_data)
{
	if (node->value.i == VCONFKEY_IDLE_UNLOCK)
		lockscreen_device_lock_unlock_request();
}

int lockscreen_device_lock_init(void)
{
	int type;
	if (!init_count) {
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST = ecore_event_type_new();
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED = ecore_event_type_new();
		vconf_notify_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed, NULL);
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
		if (!init_count)
			vconf_ignore_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed);
	}
}

int lockscreen_device_lock_unlock_request(void)
{
	if (lock_type == LOCKSCREEN_DEVICE_LOCK_NONE)
		_lockscreen_device_unlock();
	else
		ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST, NULL, NULL, NULL);

	return 0;
}

lockscreen_device_lock_type_e lockscreen_device_lock_type_get(void)
{
	return lock_type;
}

int lockscreen_device_lock_attempts_left_get(void)
{
	return -1;
}

static int
_lockscreen_device_unlock_with_password(const char *pass)
{
	unsigned int attempt, max_attempts, expire_sec;

	int ret = auth_passwd_check_passwd_state(AUTH_PWD_NORMAL, &attempt, &max_attempts, &expire_sec);
	if (ret == AUTH_PASSWD_API_ERROR_NO_PASSWORD) {
	   return 0;
	}

	ret = auth_passwd_check_passwd(AUTH_PWD_NORMAL, pass, &attempt, &max_attempts, &expire_sec);
	if (ret == AUTH_PASSWD_API_ERROR_PASSWORD_MISMATCH) {
		DBG("Wrong password: attempt: %d, max: %d,", attempt, max_attempts);
		return max_attempts != 0 ? max_attempts - attempt : -1;
	}
	else if (ret == AUTH_PASSWD_API_SUCCESS) {
		DBG("Password correct");
		return 0;
	}

	return -2;
}

int lockscreen_device_lock_unlock(const char *pass)
{
	int ret = -2;

	switch (lock_type) {
		case LOCKSCREEN_DEVICE_LOCK_NONE:
			ret = 0;
			break;
		case LOCKSCREEN_DEVICE_LOCK_PASSWORD:
		case LOCKSCREEN_DEVICE_LOCK_PIN:
			ret = _lockscreen_device_unlock_with_password(pass);
			break;
		default:
			break;
	}
	if (!ret)
		_lockscreen_device_unlock();

	return ret;
}
