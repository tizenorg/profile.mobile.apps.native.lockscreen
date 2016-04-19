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

#include <utils_i18n.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "util_time.h"
#include "log.h"

static i18n_udatepg_h _util_time_generator_get(const char *timezone_id)
{
	static i18n_udatepg_h generator;
	static char *tz;

	if (tz && !strcmp(tz, timezone_id)) {
		return generator;
	}

	if (generator) {
		i18n_udatepg_destroy(generator);
		generator = NULL;
	}

	int ret = i18n_udatepg_create(timezone_id, &generator);
	if (ret != I18N_ERROR_NONE) {
		ERR("i18n_udatepg_create failed: %s", get_error_message(ret));
		return NULL;
	}
	free(tz);
	tz = strdup(timezone_id);
	return generator;
}

static i18n_udate_format_h __util_time_date_formatter_get(const char *locale, const char *timezone_id, const char *skeleton)
{
	int status;
	i18n_uchar u_skeleton[64] = {0,};
	int32_t skeleton_len = 0, pattern_len;

	i18n_uchar u_best_pattern[64] = {0,};
	int32_t u_best_pattern_capacity;
	i18n_udate_format_h formatter = NULL;

	const i18n_udatepg_h generator = _util_time_generator_get(timezone_id);
	if (!generator) {
		ERR("_util_time_generator_get failed");
		return NULL;
	}

	i18n_ustring_copy_ua_n(u_skeleton, skeleton, strlen(skeleton));
	skeleton_len = i18n_ustring_get_length(u_skeleton);

	u_best_pattern_capacity =
		(int32_t) (sizeof(u_best_pattern) / sizeof((u_best_pattern)[0]));

	status = i18n_udatepg_get_best_pattern(generator, u_skeleton, skeleton_len,
			u_best_pattern, u_best_pattern_capacity, &pattern_len);
	if (status != I18N_ERROR_NONE) {
		ERR("i18n_udatepg_get_best_pattern failed: %s", get_error_message(status));
		return NULL;
	}

	i18n_uchar u_timezone_id[64] = {0,};
	i18n_ustring_copy_ua_n(u_timezone_id, timezone_id, sizeof(u_timezone_id));
	status = i18n_udate_create(I18N_UDATE_PATTERN, I18N_UDATE_PATTERN, locale, u_timezone_id, -1,
			u_best_pattern, -1, &formatter);
	if (status != I18N_ERROR_NONE) {
		ERR("i18n_udate_create() failed");
		return NULL;
	}

	return formatter;
}

static i18n_udate_format_h __util_time_time_formatter_get(bool use24hformat, const char *locale, const char *timezone_id)
{
	char buf[64] = {0,};
	int status;
	i18n_uchar u_pattern[64] = {0,};
	i18n_uchar u_best_pattern[64] = {0,};
	int32_t u_best_pattern_capacity, u_best_pattern_len;
	char a_best_pattern[128] = {0,};

	i18n_udate_format_h formatter = NULL;

	const i18n_udatepg_h generator = _util_time_generator_get(timezone_id);
	if (!generator) {
		ERR("_util_time_generator_get failed");
		return NULL;
	}

	if (use24hformat) {
		snprintf(buf, sizeof(buf)-1, "%s", "HH:mm");
	} else {
		/* set time format 12 */
		snprintf(buf, sizeof(buf)-1, "%s", "h:mm");
	}

	i18n_ustring_copy_ua_n(u_pattern, buf, sizeof(u_pattern));

	u_best_pattern_capacity =
		(int32_t) (sizeof(u_best_pattern) / sizeof((u_best_pattern)[0]));

	status = i18n_udatepg_get_best_pattern(generator, u_pattern, sizeof(u_pattern),
			u_best_pattern, u_best_pattern_capacity, &u_best_pattern_len);
	if (status != I18N_ERROR_NONE) {
		ERR("i18n_udatepg_get_best_pattern() failed: %s", get_error_message(status));
		return NULL;
	}

	i18n_ustring_copy_au(a_best_pattern, u_best_pattern);

	char *a_best_pattern_fixed = strtok(a_best_pattern, "a");
	a_best_pattern_fixed = strtok(a_best_pattern_fixed, " ");
	if (a_best_pattern_fixed) {
		i18n_ustring_copy_ua(u_best_pattern, a_best_pattern_fixed);
	}

	i18n_uchar u_timezone_id[64] = {0,};
	i18n_ustring_copy_ua_n(u_timezone_id, timezone_id, sizeof(u_timezone_id));

	status = i18n_udate_create(I18N_UDATE_PATTERN, I18N_UDATE_PATTERN, locale, u_timezone_id, -1,
			u_best_pattern, -1, &formatter);
	if (status != I18N_ERROR_NONE) {
		ERR("i18n_udate_create() failed");
		return NULL;
	}

	return formatter;
}

static i18n_udate_format_h __util_time_ampm_formatter_get(const char *locale, const char *timezone_id)
{
	int status;

	i18n_uchar u_best_pattern[64] = {0,};
	i18n_udate_format_h formatter = NULL;

	i18n_ustring_copy_ua(u_best_pattern, "a");

	i18n_uchar u_timezone_id[64] = {0,};
	i18n_ustring_copy_ua_n(u_timezone_id, timezone_id, sizeof(u_timezone_id));

	status = i18n_udate_create(I18N_UDATE_PATTERN, I18N_UDATE_PATTERN, locale, u_timezone_id, -1,
			u_best_pattern, -1, &formatter);

	if (status != I18N_ERROR_NONE) {
		ERR("i18n_udate_create() failed");
		return NULL;
	}

	return formatter;
}

static int __util_time_formatted_time_get(i18n_udate_format_h formatter, time_t tt, char *buf, int buf_len)
{
	if (!formatter) return -1;

	i18n_udate u_time = (i18n_udate)tt * 1000;
	i18n_uchar u_formatted_str[64] = {0,};
	int32_t u_formatted_str_capacity, buf_needed;
	int status;

	u_formatted_str_capacity = (int32_t)(sizeof(u_formatted_str) / sizeof((u_formatted_str)[0]));

	status = i18n_udate_format_date(formatter, u_time, u_formatted_str, u_formatted_str_capacity, NULL, &buf_needed);
	if (status != I18N_ERROR_NONE) {
		ERR("i18n_udate_format_date() failed");
		return -1;
	}

	i18n_ustring_copy_au_n(buf,u_formatted_str, buf_len - 1);
	DBG("time(%d) formatted(%s)", tt, buf);

	return (int)i18n_ustring_get_length(u_formatted_str);
}

bool util_time_formatted_time_get(time_t time, const char *locale, const char *timezone, bool use24hformat, char **str_time, char **str_meridiem)
{
	struct tm st;
	char buf_time[512] = {0,};
	char buf_ampm[512] = {0,};
	localtime_r(&time, &st);

	i18n_udate_format_h timef, ampmf;

	timef = __util_time_time_formatter_get(use24hformat, locale, timezone);
	__util_time_formatted_time_get(timef, time, buf_time, sizeof(buf_time)-1);

	if (!use24hformat) {
		ampmf = __util_time_ampm_formatter_get(locale, timezone);
		int ampm_len = __util_time_formatted_time_get(ampmf, time, buf_ampm, sizeof(buf_ampm)-1);
		if (ampm_len > 4) {
			if (st.tm_hour >= 0 && st.tm_hour < 12) {
				snprintf(buf_ampm, sizeof(buf_ampm)-1, "AM");
			} else {
				snprintf(buf_ampm, sizeof(buf_ampm)-1, "PM");
			}
		}
	}

	if (str_time) *str_time = strdup(buf_time);
	if (str_meridiem) *str_meridiem = strdup(buf_ampm);

	return true;
}

bool util_time_formatted_date_get(time_t time, const char *locale, const char *timezone, const char *skeleton, char **str_date)
{
	struct tm st;
	char buf_date[512] = {0,};
	localtime_r(&time, &st);
	i18n_udate_format_h datef;

	datef = __util_time_date_formatter_get(locale, timezone, skeleton ? skeleton : "MMMMEd");

	__util_time_formatted_time_get(datef, time, buf_date, sizeof(buf_date));
	if (str_date != NULL) {
		*str_date = strdup(buf_date);
		return true;
	}
	return false;
}
