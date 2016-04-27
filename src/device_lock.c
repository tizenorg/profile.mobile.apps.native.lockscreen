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

static int init_count;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST;
int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED;

int lockscreen_device_lock_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST = ecore_event_type_new();
		LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED = ecore_event_type_new();
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

int lockscreen_device_lock_unlock_request(void)
{
	/* Currently no password check is implemented */
	ecore_event_add(LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED, NULL, NULL, NULL);
	INF("Device successfully unlocked");
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
	return lockscreen_device_lock_unlock_request();
}
