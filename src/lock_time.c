/*
 * Copyright (c) 2009-2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include <appcore-common.h>

#include <unicode/utypes.h>
#include <unicode/putil.h>
#include <unicode/uiter.h>
#include <unicode/udat.h>
#include <unicode/udatpg.h>
#include <unicode/ustring.h>

#include "lockscreen.h"
#include "log.h"
#include "lock_time.h"
#include "property.h"
#include "default_lock.h"

#define TIME_LOCALE_FILE "/opt/etc/localtime"
#define TIME_ZONEINFO_PATH      "/usr/share/zoneinfo/"
#define TIME_ZONEINFO_PATH_LEN  (strlen(TIME_ZONEINFO_PATH))

static struct _s_info {
	int is_initialized;
	Ecore_Timer *timer;
	int is_timer_enabled;
	UDateFormat *formatter_date;
	UDateFormat *formatter_time;
	UDateFormat *formatter_ampm;
	UDateTimePatternGenerator *generator;
	int timeformat;
	char *timeregion_format;
	char *timezone_id;
	Eina_Bool is_pre_meridiem;
	int is_roaming;

	int need_sync;
} s_info = {
	.is_initialized = 0,
	.timer = NULL,
	.is_timer_enabled = 0,
	.formatter_date = NULL,
	.formatter_time = NULL,
	.formatter_ampm = NULL,
	.generator = NULL,
	.timeformat = APPCORE_TIME_FORMAT_24,
	.timeregion_format = NULL,
	.timezone_id = NULL,
	.is_pre_meridiem = EINA_FALSE,
	.is_roaming = -1,

	.need_sync = 0,
};

static void _timer_add(void);

static UDateFormat *__util_time_ampm_formatter_get(void *data, const char *timezone_id)
{
	UErrorCode status = U_ZERO_ERROR;

	UChar u_best_pattern[64] = {0,};
	UDateFormat *formatter = NULL;

	u_uastrcpy(u_best_pattern, "a");

	UChar u_timezone_id[64] = {0,};
	if (!timezone_id) {
		u_uastrncpy(u_timezone_id, s_info.timezone_id, sizeof(u_timezone_id));
		formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, s_info.timeregion_format, u_timezone_id, -1,
				u_best_pattern, -1, &status);
	} else {
		u_uastrncpy(u_timezone_id, timezone_id, sizeof(u_timezone_id));
		formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, s_info.timeregion_format, u_timezone_id, -1,
				u_best_pattern, -1, &status);
	}
	if (U_FAILURE(status)) {
		_E("udat_open() failed");
		return NULL;
	}

	char a_best_pattern[64] = {0,};
	u_austrcpy(a_best_pattern, u_best_pattern);

	return formatter;
}

static UDateFormat *__util_time_time_formatter_get(void *data, int time_format, const char *timezone_id)
{
	char buf[64] = {0,};
	UErrorCode status = U_ZERO_ERROR;
	UChar u_pattern[64] = {0,};
	UChar u_best_pattern[64] = {0,};
	int32_t u_best_pattern_capacity;
	char a_best_pattern[64] = {0,};

	UDateFormat *formatter = NULL;

	retv_if(!s_info.generator, NULL);

	if (time_format == APPCORE_TIME_FORMAT_24) {
		snprintf(buf, sizeof(buf)-1, "%s", "HH:mm");
	} else {
		/* set time format 12 */
		snprintf(buf, sizeof(buf)-1, "%s", "h:mm");
	}

	if (!u_uastrncpy(u_pattern, buf, sizeof(u_pattern))) {
		_E("u_uastrncpy() is failed.");
		return NULL;
	}

	u_best_pattern_capacity =
		(int32_t) (sizeof(u_best_pattern) / sizeof((u_best_pattern)[0]));

	udatpg_getBestPattern(s_info.generator, u_pattern, u_strlen(u_pattern),
			u_best_pattern, u_best_pattern_capacity, &status);
	if (U_FAILURE(status)) {
		_E("udatpg_getBestPattern() failed");
		return NULL;
	}

	u_austrcpy(a_best_pattern, u_best_pattern);

	if (a_best_pattern[0] == 'a') {
		s_info.is_pre_meridiem = EINA_TRUE;
	} else {
		s_info.is_pre_meridiem = EINA_FALSE;
	}

	char *a_best_pattern_fixed = strtok(a_best_pattern, "a");
	a_best_pattern_fixed = strtok(a_best_pattern_fixed, " ");
	if (a_best_pattern_fixed) {
		u_uastrcpy(u_best_pattern, a_best_pattern_fixed);
	}

	UChar u_timezone_id[64] = {0,};
	if (!timezone_id) {
		u_uastrncpy(u_timezone_id, s_info.timezone_id, sizeof(u_timezone_id));
		formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, s_info.timeregion_format, u_timezone_id, -1,
				u_best_pattern, -1, &status);
	} else {
		u_uastrncpy(u_timezone_id, timezone_id, sizeof(u_timezone_id));
		formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, s_info.timeregion_format, u_timezone_id, -1,
				u_best_pattern, -1, &status);
	}
	if (U_FAILURE(status)) {
		_E("udat_open() failed");
		return NULL;
	}

	return formatter;
}

static UDateFormat *__util_time_date_formatter_get(void *data, const char *timezone_id, const char *skeleton)
{
	UErrorCode status = U_ZERO_ERROR;

	UChar u_skeleton[64] = {0,};
	int skeleton_len = 0;

	UChar u_best_pattern[64] = {0,};
	int32_t u_best_pattern_capacity;
	UDateFormat *formatter = NULL;

	retv_if(!s_info.generator, NULL);

	u_uastrncpy(u_skeleton, skeleton, strlen(skeleton));
	skeleton_len = u_strlen(u_skeleton);

	u_best_pattern_capacity =
		(int32_t) (sizeof(u_best_pattern) / sizeof((u_best_pattern)[0]));

	udatpg_getBestPattern(s_info.generator, u_skeleton, skeleton_len,
			u_best_pattern, u_best_pattern_capacity, &status);
	if (U_FAILURE(status)) {
		_E("udatpg_getBestPattern() failed");
		return NULL;
	}

	UChar u_timezone_id[64] = {0,};
	if (!timezone_id) {
		u_uastrncpy(u_timezone_id, s_info.timezone_id, sizeof(u_timezone_id));
		formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, s_info.timeregion_format, u_timezone_id, -1,
				u_best_pattern, -1, &status);
	} else {
		u_uastrncpy(u_timezone_id, timezone_id, sizeof(u_timezone_id));
		formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, s_info.timeregion_format, u_timezone_id, -1,
				u_best_pattern, -1, &status);
	}
	if (U_FAILURE(status)) {
		_E("udat_open() failed");
		return NULL;
	}

	char a_best_pattern[64] = {0,};
	u_austrcpy(a_best_pattern, u_best_pattern);

	return formatter;
}

static int __util_time_formatted_time_get(UDateFormat *formatter, time_t tt, char *buf, int buf_len)
{
	retv_if (!formatter, -1);

	UDate u_time = (UDate)tt * 1000;
	UChar u_formatted_str[64] = {0,};
	int32_t u_formatted_str_capacity;
	UErrorCode status = U_ZERO_ERROR;

	u_formatted_str_capacity = (int32_t)(sizeof(u_formatted_str) / sizeof((u_formatted_str)[0]));

	(void)udat_format(formatter, u_time, u_formatted_str, u_formatted_str_capacity, NULL, &status);
	if (U_FAILURE(status)) {
		_E("udat_format() failed");
		return -1;
	}

	u_austrncpy(buf, u_formatted_str, buf_len-1);
	_D("time(%d) formatted(%s)", tt, buf);

	return (int)u_strlen(u_formatted_str);
}

static void _util_time_get(int is_current_time, time_t tt_a, char *timezone, char *skeleton, char **str_date, char **str_time, char **str_meridiem)
{
	time_t tt;
	struct tm st;
	char buf_date[512] = {0,};
	char buf_time[512] = {0,};
	char buf_ampm[512] = {0,};

	if (is_current_time == 1) {
		tt = time(NULL);
	} else {
		tt = tt_a;
	}
	localtime_r(&tt, &st);

	UDateFormat *formatter_date = NULL;
	UDateFormat *formatter_time = NULL;
	UDateFormat *formatter_ampm = NULL;

	if (timezone != NULL) {
		if (!skeleton) {
			formatter_date = __util_time_date_formatter_get(NULL, timezone, "MMMMEd");
		} else {
			formatter_date = __util_time_date_formatter_get(NULL, timezone, skeleton);
		}
		formatter_time = __util_time_time_formatter_get(NULL, s_info.timeformat, timezone);
		if (s_info.timeformat == APPCORE_TIME_FORMAT_12) {
			formatter_ampm = __util_time_ampm_formatter_get(NULL, timezone);
		}
	} else {
		if (!skeleton) {
			formatter_date = s_info.formatter_date;
		} else {
			formatter_date = __util_time_date_formatter_get(NULL, timezone, skeleton);
		}
		formatter_time = s_info.formatter_time;
		formatter_ampm = s_info.formatter_ampm;
	}

	if (!s_info.formatter_time) {
		s_info.formatter_time = __util_time_time_formatter_get(NULL, s_info.timeformat, NULL);
	}

	__util_time_formatted_time_get(formatter_date, tt, buf_date, sizeof(buf_date));

	/* time */
	if (s_info.timeformat == APPCORE_TIME_FORMAT_24) {
		__util_time_formatted_time_get(formatter_time, tt, buf_time, sizeof(buf_time)-1);
	} else {
		__util_time_formatted_time_get(formatter_time, tt, buf_time, sizeof(buf_time)-1);
		int ampm_len = __util_time_formatted_time_get(formatter_ampm, tt, buf_ampm, sizeof(buf_ampm)-1);
		if (ampm_len > 4) {
			if (st.tm_hour >= 0 && st.tm_hour < 12) {
				snprintf(buf_ampm, sizeof(buf_ampm)-1, "AM");
			} else {
				snprintf(buf_ampm, sizeof(buf_ampm)-1, "PM");
			}
		}
	}

	if (str_date != NULL) {
		*str_date = strdup(buf_date);
	}

	if (str_time != NULL) {
		*str_time = strdup(buf_time);
	}

	if (str_meridiem != NULL) {
		*str_meridiem = strdup(buf_ampm);
	}

	if (timezone != NULL) {
		if (formatter_date != NULL) udat_close(formatter_date);
		if (formatter_time != NULL) udat_close(formatter_time);
		if (formatter_ampm != NULL) udat_close(formatter_ampm);
	}
}

static char *_get_locale(void)
{
	return strdup("en_US.UTF-8");
}

static int _is_korea_locale()
{
	int ret = 0;
	char *locale = _get_locale();
	if (locale) {
		if (strstr(locale,"ko_KR")) {
			ret = 1;
		}
		free(locale);
	}

	return ret;
}

lock_error_e lock_time_update(void)
{
	Evas_Object *swipe_layout = NULL;

	struct tm st;
	time_t tt = time(NULL);
	localtime_r(&tt, &st);

	char *str_date = NULL;
	char *str_time = NULL;
	char *str_meridiem = NULL;
	char time_buf[PATH_MAX] = {0,};
	char date_buf[PATH_MAX] = {0,};

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	_util_time_get(1, 0, NULL, "MMMMEd", &str_date, &str_time, &str_meridiem);
	if (s_info.timeformat == APPCORE_TIME_FORMAT_12) {
		if (_is_korea_locale()) {
			snprintf(time_buf, sizeof(time_buf), "<%s>%s </>%s", "small_font", str_meridiem, str_time);
		} else {
			snprintf(time_buf, sizeof(time_buf), "%s<%s> %s</>", str_time, "small_font", str_meridiem);
		}
	} else {
		if (_is_korea_locale()) {
			snprintf(time_buf, sizeof(time_buf), "%s", str_time);
		} else {
			snprintf(time_buf, sizeof(time_buf), "%s", str_time);
		}
	}

	snprintf(date_buf, sizeof(time_buf), "<%s>%s</>", "small_font", str_date);

	elm_object_part_text_set(swipe_layout, "txt.time", time_buf);
	elm_object_part_text_set(swipe_layout, "txt.date", str_date);

	free(str_date);
	free(str_time);
	free(str_meridiem);

	return LOCK_ERROR_OK;
}

static UDateTimePatternGenerator *__util_time_generator_get(void *data)
{
	UErrorCode status = U_ZERO_ERROR;
	UDateTimePatternGenerator *generator = NULL;

	retv_if(!s_info.timeregion_format, NULL);

	generator = udatpg_open(s_info.timeregion_format, &status);
	if (U_FAILURE(status)) {
		_E("udatpg_open() failed");
		generator = NULL;
		return NULL;
	}
	return generator;
}

static void _util_time_formatters_create(void *data)
{
	if (!s_info.generator) {
		s_info.generator = __util_time_generator_get(NULL);
	}

	if (!s_info.formatter_date) {
		s_info.formatter_date = __util_time_date_formatter_get(NULL, NULL, "MMMMEd");
	}

	if (s_info.timeformat == APPCORE_TIME_FORMAT_12) {
		if (!s_info.formatter_ampm) {
			s_info.formatter_ampm = __util_time_ampm_formatter_get(NULL, NULL);
		}
	}

	if (!s_info.formatter_time) {
		s_info.formatter_time = __util_time_time_formatter_get(NULL, s_info.timeformat, NULL);
	}
}

static char *_util_time_timezone_id_get(void)
{
	char tz[1024] = {0,};
	char *timezone = NULL;

	memcpy(tz, "Asia/Seoul",strlen("Asia/Seoul"));
	timezone = strdup(tz);

	_D("timezone is %s ", timezone);
	return timezone;
}

static char *_util_time_regionformat_get(void)
{
	return strdup("en_US");
}

static void _formatter_create(void)
{
	bool timeformat_24_bool = false;

	timeformat_24_bool = true;

	if (timeformat_24_bool) {
		_D("TIMEFORMAT : 24");
		s_info.timeformat = APPCORE_TIME_FORMAT_24;
	} else {
		_D("TIMEFORMAT : 12");
		s_info.timeformat = APPCORE_TIME_FORMAT_12;
	}

	if (!s_info.timeregion_format) {
		s_info.timeregion_format = _util_time_regionformat_get();
	}

	if (!s_info.timezone_id) {
		s_info.timezone_id = _util_time_timezone_id_get();
	}

	_util_time_formatters_create(NULL);

	s_info.is_initialized = 1;
	_D("%d %s %s", s_info.timeformat, s_info.timeregion_format, s_info.timezone_id);
}

static void _util_time_formatters_destroy(void)
{
	if (s_info.generator) {
		udat_close(s_info.generator);
		s_info.generator = NULL;
	}
	if (s_info.formatter_date) {
		udat_close(s_info.formatter_date);
		s_info.formatter_date = NULL;
	}
	if (s_info.formatter_time) {
		udat_close(s_info.formatter_time);
		s_info.formatter_time = NULL;
	}
	if (s_info.formatter_ampm) {
		udat_close(s_info.formatter_ampm);
		s_info.formatter_ampm = NULL;
	}
}

static void _formatter_destroy(void)
{
	if (s_info.timeregion_format) {
		free(s_info.timeregion_format);
		s_info.timeregion_format = NULL;
	}
	if (s_info.timezone_id) {
		free(s_info.timezone_id);
		s_info.timezone_id = NULL;
	}

	_util_time_formatters_destroy();

	s_info.is_initialized = 0;
}

static void _util_time_vconf_changed_cb(keynode_t *key, void *data)
{
	int index = (int)data;

	_formatter_destroy();
	_formatter_create();

	if (index == 1) {
		s_info.need_sync = 1;
	}
}

static void _time_event_attach(void)
{
	int ret = 0;

	/* register vconf cbs */
	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_SVC_ROAM, _util_time_vconf_changed_cb, (void*)3);
	ret_if(ret != 0);
	ret = vconf_notify_key_changed(VCONFKEY_SETAPPL_TIMEZONE_INT, _util_time_vconf_changed_cb, (void*)4);
	ret_if(ret != 0);
}

static void _time_event_deattach(void)
{
	int ret = 0;

	/* unregister vconf cbs */
	ret = vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SVC_ROAM, _util_time_vconf_changed_cb);
	ret_if(ret != 0);
	ret = vconf_ignore_key_changed(VCONFKEY_SETAPPL_TIMEZONE_INT, _util_time_vconf_changed_cb);
	ret_if(ret != 0);
}

static Eina_Bool _timer_cb(void *data)
{
	s_info.timer = NULL;

	if (LOCK_ERROR_OK != lock_time_update()) {
		_E("Failed to update time & date");
	}

	if (s_info.is_timer_enabled == 1) {
		_timer_add();
	}
	return ECORE_CALLBACK_CANCEL;
}

static void _timer_add(void)
{
	time_t tt;
	struct tm st;

	tt = time(NULL);
	localtime_r(&tt, &st);

	s_info.timer = ecore_timer_add(60 - st.tm_sec, _timer_cb, NULL);
}

static void _timer_del(void)
{
	if (s_info.timer != NULL) {
		ecore_timer_del(s_info.timer);
		s_info.timer = NULL;
	}
}

void lock_time_timer_enable_set(int is_enable)
{
	_timer_del();
	s_info.is_timer_enabled = is_enable;

	if (is_enable == 1) {
		_timer_add();
	}
}

static void _util_time_reset_view(void)
{
	Evas_Object *swipe_layout = lock_default_swipe_layout_get();
	ret_if(!swipe_layout);

	elm_object_part_text_set(swipe_layout, "txt.time", "");
	elm_object_part_text_set(swipe_layout, "txt.date", "");
}

char *lock_time_formatted_noti_time_get(time_t ts)
{
	char *time_str = NULL;
	char *curr_date = NULL;
	char *noti_date = NULL;

	_util_time_get(0, time(NULL), NULL, UDAT_YEAR_NUM_MONTH_DAY, &curr_date, NULL, NULL);
	_util_time_get(0, ts, NULL, UDAT_YEAR_NUM_MONTH_DAY, &noti_date, NULL, NULL);

	if (curr_date != NULL && noti_date != NULL) {
		if (strcmp(curr_date, noti_date)) {
			char *date = NULL;
			_util_time_get(0, ts, NULL, UDAT_ABBR_MONTH_DAY, &date, NULL, NULL);
			free(curr_date);
			free(noti_date);
			return date;
		}
	}

	if (s_info.timeformat == APPCORE_TIME_FORMAT_24) {
		_util_time_get(0, ts, NULL, UDAT_HOUR_MINUTE , NULL, &time_str, NULL);
		if (time_str) {
			return time_str;
		}
	} else {
		struct tm st;
		localtime_r(&ts, &st);
		_util_time_get(0, ts, NULL, UDAT_HOUR_MINUTE , NULL, &time_str, NULL);

		char buf_ampm[512] = {0,};
		int ampm_len = __util_time_formatted_time_get(s_info.formatter_ampm, ts, buf_ampm, sizeof(buf_ampm) - 1);
		if (ampm_len > 4) {
			if (st.tm_hour >= 0 && st.tm_hour < 12) {
				snprintf(buf_ampm, sizeof(buf_ampm)-1, "AM");
			} else {
				snprintf(buf_ampm, sizeof(buf_ampm)-1, "PM");
			}
		}

		char time[PATH_MAX];

		if (_is_korea_locale()) {
			snprintf(time, sizeof(time), "%s %s", buf_ampm, time_str);
		} else {
			snprintf(time, sizeof(time), "%s %s", time_str, buf_ampm);
		}

		free(time_str);

		return strdup(time);
	}

	return NULL;
}

void lock_time_resume(void)
{
	if (s_info.need_sync == 1) {
		_formatter_destroy();
		_formatter_create();
		s_info.need_sync = 0;
	}

	if (LOCK_ERROR_OK != lock_time_update()) {
		_E("Failed to update time & date");
	}

	lock_time_timer_enable_set(1);
}

void lock_time_pause(void)
{
	_util_time_reset_view();
	lock_time_timer_enable_set(0);
}

void lock_time_init(void)
{
	_formatter_create();
	_time_event_attach();

	if (LOCK_ERROR_OK != lock_time_update()) {
		_E("Failed to update time & date");
	}

	lock_time_timer_enable_set(1);
}

void lock_time_fini(void)
{
	_formatter_destroy();
	_time_event_deattach();

	lock_time_timer_enable_set(0);
}
