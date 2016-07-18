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

#include "time_format.h"
#include "log.h"
#include <system_settings.h>
#include <stdlib.h>
#include <Ecore.h>

static bool use24hformat;
static char *locale, *tz_timezone;
static int init_count;
int LOCKSCREEN_EVENT_TIME_FORMAT_CHANGED;

static void _time_changed(system_settings_key_e key, void *user_data)
{
	int ret = SYSTEM_SETTINGS_ERROR_NOT_SUPPORTED;

	switch (key) {
		case SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR:
			ret = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &use24hformat);
			break;
		case SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE:
			free(tz_timezone);
			ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &tz_timezone);
			break;
		case SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE:
			free(locale);
			ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
			break;
		case SYSTEM_SETTINGS_KEY_TIME_CHANGED:
			ret = SYSTEM_SETTINGS_ERROR_NONE;
			break;
		default:
			ERR("Unhandled system_setting event: %d", key);
			break;
	}

	if (ret == SYSTEM_SETTINGS_ERROR_NONE) {
		ecore_event_add(LOCKSCREEN_EVENT_TIME_FORMAT_CHANGED, NULL, NULL, NULL);
	}
}

int lockscreen_time_format_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_TIME_FORMAT_CHANGED = ecore_event_type_new();
		int ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, _time_changed, NULL);
		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("system_settings_set_changed_cb failed: %s", get_error_message(ret));
			return 1;
		}

		ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, _time_changed, NULL);
		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("system_settings_set_changed_cb failed: %s", get_error_message(ret));
			return 1;
		}

		ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_TIME_CHANGED, _time_changed, NULL);
		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("system_settings_set_changed_cb failed: %s", get_error_message(ret));
			return 1;
		}
		ret = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &use24hformat);

		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("system_settings_get_value_bool failed: %s", get_error_message(ret));
			return 1;
		}

		ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("system_settings_get_value_string failed: %s", get_error_message(ret));
			return 1;
		}

		ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &tz_timezone);
		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			free(locale);
			ERR("system_settings_get_value_string failed: %s", get_error_message(ret));
			return 1;
		}
	}

	init_count++;
	return 0;
}

void lockscreen_time_format_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count) {
			free(locale);
			free(tz_timezone);
			system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR);
			system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE);
			system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE);
			system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_TIME_CHANGED);
			locale = tz_timezone = NULL;
		}
	}
}

const char *lockscreen_time_format_locale_get(void)
{
	return locale;
}

const char *lockscreen_time_format_timezone_get(void)
{
	return tz_timezone;
}

bool lockscreen_time_format_use_24h(void)
{
	return use24hformat;
}
