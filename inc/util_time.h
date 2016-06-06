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

bool util_time_formatted_date_get(time_t time, const char *locale, const char *timezone, const char *skeleton, char **str_date);

bool util_time_formatted_time_get(time_t time, const char *locale, const char *timezone, bool use24hformat, char **str_time, char **str_meridiem);

/**
 * @brief Get time string with concatenated meridiem
 *
 * @return string on success, NULL otherwise
 * @note returned string must be @free
 */
char *util_time_string_get(time_t time, const char *locale, const char *timezone, bool use24hformat);

#endif
