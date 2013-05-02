/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <appcore-common.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <dlog.h>
#include <app_service.h>
#include <unicode/udat.h>
#include <unicode/udatpg.h>

#include "lockscreen.h"
#include "log.h"

#define BUFFER_LENGTH 256

static Ecore_Timer *timer = NULL;
static int clock_font = 130;
static int am_pm_font = 46;

static void _lock_time_set(void *data, Eina_Bool is_pre, const char *clock, const char *am_pm)
{
	Evas_Object *info = data;
	if (info == NULL)
		return;

	char time[BUFFER_LENGTH] = {0};
	time[BUFFER_LENGTH-1] = '\0';

	if(is_pre) {
		snprintf(time, BUFFER_LENGTH - 1, "<font_size=%d>%s <font_size=%d>%s", am_pm_font, am_pm, clock_font, clock);
	}else {
		snprintf(time, BUFFER_LENGTH - 1, "<font_size=%d>%s <font_size=%d>%s", clock_font, clock, am_pm_font, am_pm);
	}
	LOCK_SCREEN_TRACE_DBG("time is %s", time);
	edje_object_part_text_set(_EDJ(info), "txt.clock", time);
}

static bool get_formatted_ampm_from_utc_time(char* date_str, int date_size, int* str_length, Eina_Bool* is_pre)
{
	UChar customSkeleton[BUFFER_LENGTH] = { 0 };
	UErrorCode status = U_ZERO_ERROR;
	UDateFormat *formatter = NULL;

	UChar bestPattern[BUFFER_LENGTH] = { 0 };
	UChar formatted[BUFFER_LENGTH] = { 0 };

	char bestPatternString[BUFFER_LENGTH] = { 0 };
	char formattedString[BUFFER_LENGTH] = { 0 };

	UDateTimePatternGenerator *pattern_generator = NULL;

	char *time_skeleton = "hhmm";

	char *locale = vconf_get_str(VCONFKEY_REGIONFORMAT);
	if (locale == NULL) {
		LOCK_SCREEN_TRACE_ERR("[Error] get value of VCONFKEY_REGIONFORMAT fail.");
		return false;
	}

	u_uastrncpy(customSkeleton, time_skeleton, strlen(time_skeleton));

	pattern_generator = udatpg_open(locale, &status);

	int32_t bestPatternCapacity = (int32_t) (sizeof(bestPattern) / sizeof((bestPattern)[0]));
	(void)udatpg_getBestPattern(pattern_generator, customSkeleton,
				    u_strlen(customSkeleton), bestPattern,
				    bestPatternCapacity, &status);

	u_austrcpy(bestPatternString, bestPattern);
	u_uastrcpy(bestPattern,"a");

	if(bestPatternString[0] == 'a')
	{
		(*is_pre) = EINA_TRUE;
	}
	else
	{
		(*is_pre) = EINA_FALSE;
	}

	UDate date = ucal_getNow();
	formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, locale, NULL, -1, bestPattern, -1, &status);
	int32_t formattedCapacity = (int32_t) (sizeof(formatted) / sizeof((formatted)[0]));
	(void)udat_format(formatter, date, formatted, formattedCapacity, NULL, &status);
	u_austrcpy(formattedString, formatted);

	LOCK_SCREEN_TRACE_DBG("DATE & TIME is %s %s %d %s", locale, formattedString, u_strlen(formatted), bestPatternString);

	(*str_length) = u_strlen(formatted);

	udatpg_close(pattern_generator);

	udat_close(formatter);

	if(strlen(formattedString) < date_size)	{
		strncpy(date_str, formattedString, strlen(formattedString));
	} else {
		strncpy(date_str, formattedString, date_size - 1);
	}

	return true;
}

static bool get_formatted_date_from_utc_time(time_t* utc_time, char* date_str, int date_size)
{
	UErrorCode status = U_ZERO_ERROR;
	UDateTimePatternGenerator *generator = NULL;
	UDateFormat *formatter = NULL;
	UChar skeleton[BUFFER_LENGTH] = { 0 }
		, pattern[BUFFER_LENGTH] = { 0 }
		, formatted[BUFFER_LENGTH] = { 0 };
	int32_t patternCapacity, formattedCapacity;
	int32_t skeletonLength, patternLength, formattedLength;
	UDate date;
	const char *locale = NULL;
	const char customSkeleton[] = UDAT_MONTH_WEEKDAY_DAY;

	date = (UDate) (*utc_time) *1000;

	uloc_setDefault(__secure_getenv("LC_TIME"), &status);
	locale = vconf_get_str(VCONFKEY_REGIONFORMAT);
	if (locale == NULL) {
		LOCK_SCREEN_TRACE_ERR("[Error] get value of VCONFKEY_REGIONFORMAT fail.");
		return false;
	}

	generator = udatpg_open(locale, &status);
	if (generator == NULL)
		return false;

	patternCapacity = (int32_t) (sizeof(pattern) / sizeof((pattern)[0]));

	u_uastrcpy(skeleton, customSkeleton);

	skeletonLength = strlen(customSkeleton);

	patternLength =
		udatpg_getBestPattern(generator, skeleton, skeletonLength, pattern,
				patternCapacity, &status);

	formatter =
		udat_open(UDAT_IGNORE, UDAT_DEFAULT, locale, NULL, -1, pattern,
			patternLength, &status);
	if (formatter == NULL) {
		udatpg_close(generator);
		return false;
	}

	formattedCapacity =
		(int32_t) (sizeof(formatted) / sizeof((formatted)[0]));

	formattedLength =
		udat_format(formatter, date, formatted, formattedCapacity, NULL,
			&status);

	u_austrcpy(date_str, formatted);

	udatpg_close(generator);

	udat_close(formatter);

	return true;
}

static Eina_Bool _set_info_time(void *data)
{
	Evas_Object *info = (Evas_Object *) data;
	if (info == NULL)
		return false;

	struct tm st;
	time_t tt;
	char buf[512] = { 0, };
	char bf1[32] = { 0, };
	char bf2[32] = { 0, };
	int r, hour;
	enum appcore_time_format timeformat;

	tt = time(NULL);
	localtime_r(&tt, &st);

	if (timer != NULL) {
		ecore_timer_del(timer);
		timer = NULL;
	}

	char utc_date[256] = { 0, };
	get_formatted_date_from_utc_time(&tt, utc_date, sizeof(utc_date));
	edje_object_part_text_set(_EDJ(info), "txt.date", utc_date);

	timer = ecore_timer_add(60 - st.tm_sec, _set_info_time, info);

	r = appcore_get_timeformat(&timeformat);
	if (r == 0 && timeformat == APPCORE_TIME_FORMAT_24) {
		strftime(bf1, sizeof(bf1), "%H:%M", &st);
		snprintf(buf, sizeof(buf), "%s", bf1);
		edje_object_part_text_set(_EDJ(info), "txt.clock", bf1);
	} else {
		strftime(bf1, sizeof(bf1), "%l", &st);
		hour = atoi(bf1);
		strftime(bf1, sizeof(bf1), ":%M", &st);
		snprintf(buf, sizeof(buf), "%d%s", hour, bf1);

		char utc_ampm[BUFFER_LENGTH] = { 0 };
		int ampm_length = 0;
		Eina_Bool is_pre = EINA_FALSE;

		get_formatted_ampm_from_utc_time(utc_ampm, sizeof(utc_ampm), &ampm_length, &is_pre);
		LOCK_SCREEN_TRACE_DBG("utc_ampm = %s", utc_ampm);
		if(ampm_length > 0 && ampm_length <= 4) {
			snprintf(bf2, sizeof(bf2), "%s", utc_ampm);
		} else {
			if (st.tm_hour >= 0 && st.tm_hour < 12) {
				snprintf(bf2, sizeof(bf2), "%s", "AM");
			} else {
				snprintf(bf2, sizeof(bf2), "%s", "PM");
			}
		}
		_lock_time_set(info, is_pre, buf, bf2);
	}

	return 0;
}

static void _set_info_helptext(void *data)
{
	struct appdata *ad = data;
	if (ad == NULL) {
		return;
	}

	edje_object_part_text_set(_EDJ(ad->info), "txt.helptext", _NOT_LOCALIZED("Drag along the dots to unlock"));
}

void _set_info(void *data)
{
	LOGD("[ == %s == ]", __func__);
	struct appdata *ad = data;
	if (ad == NULL) {
		return;
	}

	int is_clock = -1;
	int is_helptext = -1;

	int retc = vconf_get_bool(VCONFKEY_LOCKSCREEN_CLOCK_DISPLAY, &is_clock);
	int retht = vconf_get_bool(VCONFKEY_LOCKSCREEN_HELP_TEXT_DISPLAY, &is_helptext);

	if(0 == retc) {
		if(is_clock) {
			_set_info_time(ad->info);
		}
	}
	if(0 == retht) {
		if(is_helptext) {
			_set_info_helptext(ad);
		}
	}
}

void update_time(void *data)
{
	struct appdata *ad = data;
	if (ad == NULL) {
		return;
	}

	int is_clock = -1;
	int retc = vconf_get_bool(VCONFKEY_LOCKSCREEN_CLOCK_DISPLAY, &is_clock);
	if(0 == retc) {
		if(is_clock) {
			_set_info_time(ad->info);
		}
	}
}
