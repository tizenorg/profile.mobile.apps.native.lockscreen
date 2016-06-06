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

#ifndef _LOCKSCREEN_DEVICE_LOCK_H_
#define _LOCKSCREEN_DEVICE_LOCK_H_

#include <stdbool.h>

typedef enum {
	LOCKSCREEN_DEVICE_LOCK_NONE, /* No password is set */
	LOCKSCREEN_DEVICE_LOCK_PIN,  /* PIN password type [0-9]* */
	LOCKSCREEN_DEVICE_LOCK_PASSWORD, /* Alphanumeric passoword */
	LOCKSCREEN_DEVICE_LOCK_PATTERN, /* Patter password */
} lockscreen_device_lock_type_e;

/**
 * @brief Event fired when unlock request has been triggered
 * with lockscreen_device_lock_unlock_request method.
 */
extern int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST;

/**
 * @brief Event fired when unlock has been cancelled
 * with lockscreen_device_lock_unlock_request_cancel method.
 */
extern int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST_CANCELLED;

/**
 * @brief Event fired when device has been successfully
 * unlocked with lockscreen_device_lock_unlock method.
 */
extern int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED;

/**
 * @brief Initialize device lock module
 *
 * @return 0 on success, other value on failure.
 */
int lockscreen_device_lock_init(void);

/**
 * @brief Shutdowns device lock module
 */
void lockscreen_device_lock_shutdown(void);

/**
 * @brief Request device unlock.
 *
 * @note This function may trigger LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST
 * event or LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED event in case when no lock
 * type is set.
 *
 * @return 0 on success, other value on failure.
 */
int lockscreen_device_lock_unlock_request(void);

int lockscreen_device_lock_unlock_request_cancel(void);

/**
 * @brief Get current lock type.
 */
lockscreen_device_lock_type_e lockscreen_device_lock_type_get(void);

/**
 * @brief Unlock result description
 */
typedef enum {
	LOCKSCREEN_DEVICE_UNLOCK_SUCCESS,
	LOCKSCREEN_DEVICE_UNLOCK_FAILED,
	LOCKSCREEN_DEVICE_UNLOCK_ERROR,
} lockscreen_device_unlock_result_e;

/**
 * @brief Try to unlock device using given password.
 *
 * @param pass password used to unlock
 * @param[out] attempts_left number of unlock attempts left
 *
 * @note May trigger LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED event.
 *
 * @return error code.
 *
 * @note if return value == LOCKSCREEN_DEVICE_UNLOCK_ERROR, attempts_left is undefined.
 * @note attempts_left == -1, means infinite number of attempts left.
 */
lockscreen_device_unlock_result_e lockscreen_device_lock_unlock(const char *pass, int *attempts_left);

#endif
