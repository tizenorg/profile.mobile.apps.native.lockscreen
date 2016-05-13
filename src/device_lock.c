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

static int init_count;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED;

static void _lockscreen_device_unlock(void)
{
	ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, NULL, NULL, NULL);
}

static void _lockscreen_device_vconf_idle_key_changed(keynode_t *node, void *user_data)
{
	if (node->value.i == VCONFKEY_IDLE_UNLOCK)
		_lockscreen_device_unlock();
}

int lockscreen_device_lock_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST = ecore_event_type_new();
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED = ecore_event_type_new();
		vconf_notify_key_changed(VCONFKEY_IDLE_LOCK_STATE, _lockscreen_device_vconf_idle_key_changed, NULL);
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
	/* Currently no password check is implemented */
	INF("Device successfully unlocked");
	_lockscreen_device_unlock();
	return 0;
}

lockscreen_device_lock_type_e lockscreen_device_lock_type_get(void)
{
	return LOCKSCREEN_DEVICE_LOCK_NONE;
}

int lockscreen_device_lock_attempts_left_get(void)
{
	return -1;
}

int lockscreen_device_lock_unlock(const char *pass)
{
	_lockscreen_device_unlock();
	return 0;
}
