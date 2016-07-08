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

#ifndef _LOCKSCREEN_SHORTCUT_H_
#define _LOCKSCREEN_SHORTCUT_H_

#include <stdbool.h>

/**
 * @addtogroup Models
 * @{
 */

/**
 * @defgroup Shortcut Shortcut
 */

/**
 * @addtogroup Shortcut
 * @{
 */

/**
 * @brief Initializes shortcut module
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_shortcut_init(void);

/**
 * @brief Activates system-default shortcut application.
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_shortcut_activate();

/**
 * @brief Shutdowns shortcut module
 */
void lockscreen_shortcut_shutdown(void);

/**
 * @brief Returns true if shortcut shortcut icon should be displayed
 * on lockscreen, false otherwise.
 */
bool lockscreen_shortcut_is_on(void);

/**
 * @brief Returns icon path of application shortcut
 */
const char *lockscreen_shortcut_icon_path_get(void);

/**
 * @brief Checks if shortcut requires lockscreen to unlock
 *
 * @return true if requires, false otherwise.
 */
bool lockscreen_shortcut_require_unlock(void);

/**
 * @}
 */

/**
 * @}
 */
#endif

