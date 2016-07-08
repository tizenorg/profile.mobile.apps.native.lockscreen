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

#ifndef _LOCKSCREEN_TIME_FORMAT_H_
#define _LOCKSCREEN_TIME_FORMAT_H_

#include <stdbool.h>

/**
 * @addtogroup Models
 * @{
 */

/**
 * @defgroup Time Time Format
 */

/**
 * @addtogroup Time
 * @{
 */

/**
 * @brief Event fired when locale/timezone/24h fromat information changes
 */
extern int LOCKSCREEN_EVENT_TIME_FORMAT_CHANGED;

/**
 * @brief Initializes time format information events.
 */
int lockscreen_time_format_init(void);

/**
 * @brief Shutdowns time format information
 */
void lockscreen_time_format_shutdown(void);

/**
 * @brief Get system default locale
 */
const char *lockscreen_time_format_locale_get(void);

/**
 * @brief Get system default timezone
 */
const char *lockscreen_time_format_timezone_get(void);

/**
 * @brief Get 24-h format option
 * @note return true is system is using 24-h time date format, false otherwise.
 */
bool lockscreen_time_format_use_24h(void);

/**
 * @}
 */

/**
 * @}
 */

#endif
