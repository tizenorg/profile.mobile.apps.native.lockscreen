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

#ifndef _LOCKSCREEN_DEVICE_LOCK_CTRL_H_
#define _LOCKSCREEN_DEVICE_LOCK_CTRL_H_

#include <Elementary.h>

/**
 * @brief Event fired when unlock request has been canceled by the user.
 */
extern int LOCKSCREEN_EVENT_DEVICE_LOCK_CTRL_UNLOCK_REQUEST_CANCELED;

/**
 * @brief Request controller to unlock device.
 *
 * @note This function may trigger LOCKSCREEN_EVENT_DEVICE_LOCK_CTRL_UNLOCK_REQUEST_CANCELED
 *
 * @param content object that should be displayed on unlock panel
 * @return 0 on success, other value on failure.
 */
int lockscreen_device_lock_ctrl_unlock_request(Evas_Object *content);

/**
 * @brief Initialize device_lock controller.
 *
 * Device lock controller module manages displaying unlock panels (password type, PIN,
 * pattern, etc.) and password verification.
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_device_lock_ctrl_init(Evas_Object *main_view);

/**
 * @brief Deinitialize device_lock controller.
 */
void lockscreen_device_lock_ctrl_shutdown();

#endif
