/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the License);
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

static Ecore_Timer *timer = NULL;

static bool get_formatted_date_from_utc_time(time_t* utc_time, char* date_str, int date_size)
{
	UErrorCode status = U_ZERO_ERROR;
	UDateTimePatternGenerator *generator;
	UDateFormat *formatter;
	UChar skeleton[40] = { 0 }
		, pattern[40] = { 0 }
		, formatted[40] = { 0 };
	int32_t patternCapacity, formattedCapacity;
	int32_t skeletonLength, patternLength, formattedLength;
	UDate date;
	const char *locale;
	const char customSkeleton[] = UDAT_MONTH_WEEKDAY_DAY;

	date = (UDate) (*utc_time) *1000;

	uloc_setDefault(__secure_getenv("LC_TIME"), &status);
	locale = uloc_getDefault();

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
	} else {
		strftime(bf1, sizeof(bf1), "%l", &st);
		hour = atoi(bf1);
		strftime(bf1, sizeof(bf1), ":%M", &st);
		snprintf(buf, sizeof(buf), "%d%s", hour, bf1);
		if (st.tm_hour >= 0 && st.tm_hour < 12) {
			snprintf(bf2, sizeof(bf2), "%s", "AM");
			if ((st.tm_hour - 10) < 0 && st.tm_hour != 0) {
				edje_object_signal_emit(_EDJ(info), "digit,clock", "rect.clock.ampm");
			} else {
				edje_object_signal_emit(_EDJ(info), "default,clock", "rect.clock.ampm");
			}
		} else {
			snprintf(bf2, sizeof(bf2), "%s", "PM");
			if ((st.tm_hour - 12) < 10 && (st.tm_hour - 12) != 0) {
				edje_object_signal_emit(_EDJ(info), "digit,clock", "rect.clock.ampm");
			} else {
				edje_object_signal_emit(_EDJ(info), "default,clock", "rect.clock.ampm");
			}
		}
		edje_object_part_text_set(_EDJ(info), "txt.clock.ampm", bf2);
	}
	edje_object_part_text_set(_EDJ(info), "txt.clock", buf);

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
