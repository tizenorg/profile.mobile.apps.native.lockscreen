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

#include <device/battery.h>
#include <device/callback.h>

#include "lockscreen.h"
#include "log.h"
#include "battery.h"
#include "property.h"
#include "default_lock.h"

static struct _s_info {
	bool is_connected;
	bool is_charging;
} s_info = {
	.is_connected = false,
	.is_charging = false,
};

bool lock_battery_is_charging_get(void)
{
	return s_info.is_charging;
}

bool lock_battery_is_connected_get(void)
{
	return s_info.is_connected;
}

static char *_replaceString(char *strInput, const char *strTarget, const char *strChange)
{
	char* strResult;
	char* strTemp;
	int i = 0;
	int nCount = 0;
	int nTargetLength = strlen(strTarget);

	if (nTargetLength < 1) {
		_E("there is no target to chnage");
		return NULL;
	}

	int nChangeLength = strlen(strChange);

	if (nChangeLength != nTargetLength) {
		for (i = 0; strInput[i] != '\0';) {
			if (memcmp(&strInput[i], strTarget, nTargetLength) == 0) {
				nCount++;		//consider same string exist
				i += nTargetLength;
			} else {
				i++;
			}
		}
	} else {
		i = strlen(strInput);
	}

	strResult = (char *) malloc(i + 1 + nCount * (nChangeLength - nTargetLength));

	if (!strResult) {
		_E("fail malloc!!");
		return NULL;
	}


	strTemp = strResult;
	while (*strInput) {
		if (memcmp(strInput, strTarget, nTargetLength) == 0) {
			memcpy(strTemp, strChange, nChangeLength);
			strTemp += nChangeLength;	//move changed length
			strInput  += nTargetLength;	// move target length
		} else {
			*strTemp++ = *strInput++;		// original str cpy
		}
	}

	*strTemp = '\0';

	return strResult;
}

lock_error_e lock_battery_update(void)
{
	Evas_Object *swipe_layout = NULL;

	bool status = false;
	int capacity = 0;
	device_battery_level_e battery_level = 0;
	bool charger = false;
	int ret = 0;

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	ret = lock_property_get_bool(PROPERTY_TYPE_RUNTIME_INFO, (void *)RUNTIME_INFO_KEY_BATTERY_IS_CHARGING, &status);
	if (ret != LOCK_ERROR_OK) {
		_E("Failed to get runtime info : RUNTIME_INFO_KEY_BATTERY_IS_CHARGING");
		elm_object_part_text_set(swipe_layout, "txt.battery", "");
		return LOCK_ERROR_FAIL;
	} else {
		elm_object_signal_emit(swipe_layout, "show,txt,battery", "txt.battery");

		if (status == true) {
			ret = device_battery_get_percent(&capacity);
			if (ret != DEVICE_ERROR_NONE) {
				_E("Failed to get battery percent(%d)", ret);
			}

			ret = device_battery_get_level_status(&battery_level);
			if (ret != DEVICE_ERROR_NONE) {
				_E("Failed to get battery level status(%d)", ret);
			}

			if (capacity == 100) {
				_D("Fully charged");
				elm_object_part_text_set(swipe_layout, "txt.battery", _("IDS_SM_POP_FULLY_CHARGED"));
			} else {
				char buff[64];
				char *newString = NULL;
				newString = _replaceString(_("IDS_LCKSCN_BODY_CHARGING_C_PDP"), "%d%", "%d%%");

				if (newString != NULL) {
					snprintf(buff, sizeof(buff), newString , capacity);
					free(newString) ;
				} else {
					snprintf(buff, sizeof(buff), _("IDS_LCKSCN_BODY_CHARGING_C_PDP") , capacity);
				}

				elm_object_part_text_set(swipe_layout, "txt.battery", buff);
			}
		} else {
			elm_object_part_text_set(swipe_layout, "txt.battery", "");

			ret = lock_property_get_bool(PROPERTY_TYPE_RUNTIME_INFO, (void *)RUNTIME_INFO_KEY_CHARGER_CONNECTED , &charger);
			if (ret != LOCK_ERROR_OK) {
				_E("Failed to get runtime info : RUNTIME_INFO_KEY_CHARGER_CONNECTED");
			} else {
				ret = device_battery_get_percent(&capacity);
				if (ret != DEVICE_ERROR_NONE) {
					_E("Failed to get battery percent(%d)", ret);
				}

				ret = device_battery_get_level_status(&battery_level);
				if (ret != DEVICE_ERROR_NONE) {
					_E("Failed to get battery level status(%d)", ret);
				}

				if (capacity == 100 && charger == true) {
					elm_object_part_text_set(swipe_layout, "txt.battery", _("IDS_SM_POP_FULLY_CHARGED"));
				} else {
					elm_object_part_text_set(swipe_layout, "txt.battery", "");
				}
			}
		}
	}

	return LOCK_ERROR_OK;
}

static void _battery_changed_cb(device_callback_e type, void *value, void *user_data)
{
	_D("%s", __func__);

	if (LOCK_ERROR_OK != lock_battery_update()) {
		_E("Failed to update battery information");
	}
}

lock_error_e lock_battery_show(void)
{
	Evas_Object *swipe_layout = NULL;

	int ret = 0;

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	elm_object_signal_emit(swipe_layout, "show,txt,battery", "txt.battery");

	ret = device_add_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, _battery_changed_cb, NULL);
	if (ret != DEVICE_ERROR_NONE) {
		_E("Failed to add device callback : DEVICE_CALLBACK_BATTERY_CAPACITY(%d)", ret);
	}

	ret = device_add_callback(DEVICE_CALLBACK_BATTERY_LEVEL, _battery_changed_cb, NULL);
	if (ret != DEVICE_ERROR_NONE) {
		_E("Failed to add device callback : DEVICE_CALLBACK_BATTERY_LEVEL(%d)", ret);
	}

	return LOCK_ERROR_OK;
}

lock_error_e lock_battery_hide(void)
{
	Evas_Object *swipe_layout = NULL;

	int ret = 0;

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	elm_object_signal_emit(swipe_layout, "hide,txt,battery", "txt.battery");

	ret = device_remove_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, _battery_changed_cb);
	if (ret != DEVICE_ERROR_NONE) {
		_E("Failed to remove device callback : DEVICE_CALLBCK_BATTERY_CAPACITY(%d)", ret);
	}

	ret = device_remove_callback(DEVICE_CALLBACK_BATTERY_LEVEL, _battery_changed_cb);
	if (ret != DEVICE_ERROR_NONE) {
		_E("Failed to remove device callback : DEVICE_CALLBCK_BATTERY_LEVEL(%d)", ret);
	}

	return LOCK_ERROR_OK;
}

static void _battery_charger_changed_cb(runtime_info_key_e key, void *data)
{
	_D("%s", __func__);

	int ret = 0;
	bool is_connected = 0;

	ret = lock_property_get_bool(PROPERTY_TYPE_RUNTIME_INFO, (void *)RUNTIME_INFO_KEY_CHARGER_CONNECTED , &is_connected);
	if (ret != LOCK_ERROR_OK) {
		_E("Failed to get runtime info : RUNTIME_INFO_KEY_CHARGER_CONNECTED");
	}

	_D("charger connected : %d", is_connected);
	s_info.is_connected = is_connected;

	if (is_connected) {
		_D("show battery information");
		if (LOCK_ERROR_OK != lock_battery_show()) {
			_E("Failed to show battery infomation");
		}

		if (LOCK_ERROR_OK != lock_battery_update()) {
			_E("Failed to update battery information");
		}
	} else {
		_D("hide battery inforamtion");
		if (LOCK_ERROR_OK != lock_battery_hide()) {
			_E("Failed to hide battery information");
		}
	}
}

lock_error_e lock_battery_init(void)
{
	int ret = 0;

	ret = runtime_info_set_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED, _battery_charger_changed_cb, NULL);
	if (ret != RUNTIME_INFO_ERROR_NONE) {
		_E("Failed to set changed cb : RUNTIME_INFO_KEY_CHANGER_CONNECTED(%d)", ret);
	}

	ret = runtime_info_set_changed_cb(RUNTIME_INFO_KEY_BATTERY_IS_CHARGING, _battery_charger_changed_cb, NULL);
	if (ret != RUNTIME_INFO_ERROR_NONE) {
		_E("Failed to set changed cb : RUNTIME_INFO_KEY_BATTERY_IS_CHARGING(%d)", ret);
	}

	_battery_charger_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED, NULL);

	return LOCK_ERROR_OK;
}

void lock_battery_fini(void)
{
	int ret = 0;

	ret = runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED);
	if (ret != RUNTIME_INFO_ERROR_NONE) {
		_E("Failed to set changed cb : RUNTIME_INFO_KEY_CHANGER_CONNECTED(%d)", ret);
	}

	ret = runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_BATTERY_IS_CHARGING);
	if (ret != RUNTIME_INFO_ERROR_NONE) {
		_E("Failed to set changed cb : RUNTIME_INFO_KEY_BATTERY_IS_CHARGING(%d)", ret);
	}
}
