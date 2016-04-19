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

#include "log.h"
#include "battery.h"

#include <device/battery.h>
#include <device/callback.h>
#include <runtime_info.h>

#include <Ecore.h>

static bool is_connected, is_charging;
static int level, init_count;
int LOCKSCREEN_EVENT_BATTERY_CHANGED;

int _battery_status_update()
{
	bool model_changed = false;

	int percent, ret;
	bool charging, connected;

	ret = runtime_info_get_value_bool(RUNTIME_INFO_KEY_CHARGER_CONNECTED, &connected);
	if (ret != RUNTIME_INFO_ERROR_NONE) {
		ERR("runtime_info_get_value_bool failed: %s", get_error_message(ret));
		return -1;
	}
	if (is_connected != connected) {
		is_connected = connected;
		model_changed = true;
	}

	ret = device_battery_is_charging(&charging);
	if (ret != DEVICE_ERROR_NONE) {
		ERR("device_battery_is_charging failed: %s", get_error_message(ret));
		return -1;
	}
	if (is_charging != charging) {
		is_charging = charging;
		model_changed = true;
	}

	ret = device_battery_get_percent(&percent);
	if (ret != DEVICE_ERROR_NONE) {
		ERR("device_battery_get_percent failed: %s", get_error_message(ret));
		return -1;
	}
	if (level != percent) {
		level = percent;
		model_changed = true;
	}

	if (model_changed)
		ecore_event_add(LOCKSCREEN_EVENT_BATTERY_CHANGED, NULL, NULL, NULL);

	return 0;
}

static void _battery_changed_cb(device_callback_e type, void *value, void *user_data)
{
	if (type == DEVICE_CALLBACK_BATTERY_LEVEL || type == DEVICE_CALLBACK_BATTERY_CAPACITY)
		_battery_status_update();
}

static void _battery_charger_changed_cb(runtime_info_key_e key, void *data)
{
	if (key == RUNTIME_INFO_KEY_CHARGER_CONNECTED)
		_battery_status_update();
}

int lockscreen_battery_init()
{
	if (!init_count) {
		LOCKSCREEN_EVENT_BATTERY_CHANGED = ecore_event_type_new();
		int ret = runtime_info_set_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED, _battery_charger_changed_cb, NULL);
		if (ret != RUNTIME_INFO_ERROR_NONE) {
			ERR("runtime_info_set_changed_cb failed: %s", get_error_message(ret));
			return -1;
		}

		ret = device_add_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, _battery_changed_cb, NULL);
		if (ret != DEVICE_ERROR_NONE) {
			ERR("device_add_callback failed: %s", get_error_message(ret));
			runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED);
			return -1;
		}

		ret = device_add_callback(DEVICE_CALLBACK_BATTERY_CHARGING, _battery_changed_cb, NULL);
		if (ret != DEVICE_ERROR_NONE) {
			ERR("device_add_callback failed: %s", get_error_message(ret));
			device_remove_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, _battery_changed_cb);
			runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED);
			return -1;
		}

		_battery_status_update();
	}
	init_count++;
	return 0;
}

void lockscreen_battery_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count)
			runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED);
	}
}

bool lockscreen_battery_is_charging(void)
{
	return is_charging;
}

bool lockscreen_battery_is_connected(void)
{
	return is_connected;
}

int lockscreen_battery_level_get(void)
{
	return level;
}
