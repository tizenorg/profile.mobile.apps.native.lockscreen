/*
 * Copyright 2016  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LOCK_SCREEN_UTIL_TIME_H__
#define __LOCK_SCREEN_UTIL_TIME_H__

#include <Elementary.h>

/**
 * @addtogroup Utils
 * @{
 */

/**
 * @brief Gets formatted date string according to locale, timezone and given
 * skeleton
 *
 * @param time time to be formatted as string
 * @param locale valid ISO15897 locale string
 * @param timezone valid ISO15897 timezone string
 * @param skeleton valid ICU skeleton string
 * @param[out] str_date formatted date string
 *
 * @return true on success, false otherwise
 *
 * @note str_date should be free()
 */
bool util_time_formatted_date_get(time_t time, const char *locale, const char *timezone, const char *skeleton, char **str_date);

/**
 * @brief Gets formatted time string.
 *
 * @param time time to be formatted as string
 * @param locale valid ISO15897 locale string
 * @param timezone valid ISO15897 timezone string
 * @param use24hformat use 24h format
 * @param[out] str_time formatted date string
 * @param[out] str_meridiem formatted date string
 *
 * @note str_time and str_meridiem should be free()
 */
bool util_time_formatted_time_get(time_t time, const char *locale, const char *timezone, bool use24hformat, char **str_time, char **str_meridiem);

/**
 * @brief Get time string with concatenated meridiem
 *
 * @param time time to be formatted as string
 * @param locale valid ISO15897 locale string
 * @param timezone valid ISO15897 timezone string
 * @param use24hformat use 24h format
 *
 * @return string on success, NULL otherwise
 * @note returned string must be free()
 */
char *util_time_string_get(time_t time, const char *locale, const char *timezone, bool use24hformat);

/**
 * @brief Gets time zone from /opt/etc/localtime symbolic link
 *
 * @return time zone on success, NULL otherwise
 * @note returned string must be freed using free()
 */
char *util_timezone_get(void);

/**
 * @}
 */

#endif
