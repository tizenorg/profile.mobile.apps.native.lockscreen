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
#include "events.h"

/**
 * @addtogroup Models
 * @{
 */

/**
 * @defgroup Device Device Lock
 */

/**
 * @addtogroup Device
 * @{
 */

/**
 * @brief Lockscreen lock type
 */
typedef enum {
	LOCKSCREEN_DEVICE_LOCK_NONE, /* No password is set */
	LOCKSCREEN_DEVICE_LOCK_PIN,  /* PIN password type [0-9]* */
	LOCKSCREEN_DEVICE_LOCK_PASSWORD, /* Alphanumeric passoword */
} lockscreen_device_lock_type_e;

/**
 * @brief Event fired when device has been successfully
 * authenticated with lockscreen_device_lock_authenticate function.
 */ 
extern int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED;

/**
 * @brief Event fired when device has been asked to unlock.
 */
extern int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST;

/**
 * @brief Event fired when unlock attempt has been unsuccessfull.
 */
extern int LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_CANCELLED;

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
 * @brief Get current lock type.
 */
lockscreen_device_lock_type_e lockscreen_device_lock_type_get(void);

/**
 * @brief Unlock result description
 */
typedef enum {
	LOCKSCREEN_DEVICE_AUTH_SUCCESS, /** Authentication successed */
	LOCKSCREEN_DEVICE_AUTH_FAILED, /** Creditials were incorrect */
	LOCKSCREEN_DEVICE_AUTH_ERROR, /** Internal error occured. Unable to validate password */
} lockscreen_device_auth_result_e;

/**
 * @brief Type of unlock context
 *
 * The unlock context describes what action will be performed
 * when unlock attempt success.
 */
typedef enum {
	LOCKSCREEN_DEVICE_UNLOCK_CONTEXT_LAUNCH_EVENT, /* Launch event after unlock */
} lockscreen_device_unlock_context_type_e;

/**
 * @brief Unlock context.
 *
 * @description Contains detailed information that
 * are passed to LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST event.
 */
typedef struct {
	lockscreen_device_unlock_context_type_e type;
	union {
		lockscreen_event_t *event;
	} data;
} lockscreen_device_unlock_context_t;

/**
 * @brief Try to authenticate using given password.
 *
 * @param pass password used to unlock
 * @param[out] attempts_left number of unlock attempts left
 *
 * @note May trigger LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED event.
 *
 * @return auth result
 *
 * @note if return value == LOCKSCREEN_DEVICE_AUTH_ERROR, attempts_left is undefined.
 * @note attempts_left = -1, means infinite number of attempts left.
 */
lockscreen_device_auth_result_e lockscreen_device_lock_authenticate(const char *pass, int *attempts_left);

/**
 * @brief Unlock the device.
 *
 * @return 0 on success, other value on failure (eg. lockscreen is secured
 * and must be first authenticated with @lockscreen_device_lock_authenticate
 * function)
 *
 * @note This function may exit application;
 */
int lockscreen_device_lock_unlock(void);

/**
 * @brief Gets number of maximum unlock attempts
 *
 * @return -1 on error
 * @return 0 if max attempts limit is infinite
 */
int lockscreen_device_lock_max_unlock_attempts_get(void);

/**
 * @brief Request device to unlock.
 *
 * This API is meant to be used by lockscreen modules that do not
 * implement unlock "view" by themself, however they require to
 * unlock device before making actions. The unlock request is secured
 * by global mutex (so only single lockscreen module may have request to
 * unlock).
 *
 * May emit LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_REQUEST event.
 *
 * @param Unlock request context.
 *
 * @note @param context may be NULL,
 *
 * @return 0 on success, other value on failue.
 */
int lockscreen_device_lock_unlock_request(const lockscreen_device_unlock_context_t *context);

/**
 * @brief Cancel active unlock request.
 *
 * @return 0 on success, other value on failue.
 *
 * @note May emit LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_CANCELLED event.
 */
int lockscreen_device_lock_unlock_request_cancel(void);

#endif
