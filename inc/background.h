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

#ifndef _LOCKSCREEN_BACKGROUND_H_
#define _LOCKSCREEN_BACKGROUND_H_

/**
 * @addtogroup Models
 * @{
 */

/**
 * @defgroup Background Background
 */

/**
 * @addtogroup Background
 * @{
 */

/**
 * @brief Event fired when lockscreen background changes.
 * @note register via ecore_event_handler_add
 * @note can be triggered after lockscreen_background_init
 */
extern int LOCKSCREEN_EVENT_BACKGROUND_CHANGED;

/**
 * @brief Initializes background notification changes.
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_background_init(void);

/**
 * @brief Sets background file that will be display be lockscreen.
 * If path parameter is NULL the default background will be used.
 *
 * @note can fire LOCKSCREEN_EVENT_BACKGROUND_CHANGED event
 * @return: 0 on success, other value on failure.
 */
int lockscreen_background_file_set(const char *path);

/**
 * @brief Deinitialize background notification changes.
 */
void lockscreen_background_shutdown(void);

/**
 * @brief Get current background file path.
 */
const char *lockscreen_background_file_get(void);

/**
 * @}
 */

/**
 * @}
 */

#endif

