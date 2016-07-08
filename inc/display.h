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

#ifndef _LOCKSCREEN_DISPLAY_H_
#define _LOCKSCREEN_DISPLAY_H_

#include <stdbool.h>

/**
 * @addtogroup Models
 * @{
 */

/**
 * @defgroup Display Display
 */

/**
 * @addtogroup Display
 * @{
 */

/**
 * @brief Event fired when device display truns off or on.
 */
extern int LOCKSCREEN_EVENT_DISPLAY_STATUS_CHANGED;

/**
 * @brief Initializes display notifications.
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_display_init(void);

/**
 * @brief Deinitialize display notifications.
 */
void lockscreen_display_shutdown(void);

/**
 * @brief
 * Freeze auto disabling device screen after predefined number of seconds.
 *
 * @note if device display in already in "off" state this function has no effect.
 */
void lockscreen_display_timer_freeze(void);

/**
 * @brief
 * Resumes auto disabling device screen after predefined number of seconds.
 *
 * @note if device display in already in "off" state this function will
 * turn on display and renew timer.
 * @note if lockscreen_data_model_display_timer_freeze function was not called
 * beforehead, this function resets time estimated to turn off display.
 */
void lockscreen_display_timer_renew(void);

/**
 * @brief Gets current display status.
 * Returns true if display is turned on, false otherwise.
 */
bool lockscreen_display_is_off(void);

/**
 * @}
 */

/**
 * @}
 */

#endif
