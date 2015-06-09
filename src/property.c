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

#include <vconf.h>

#include "lockscreen.h"
#include "log.h"
#include "property.h"
#include "default_lock.h"

static struct _s_info {
	bool is_sound_lock;
	bool is_sound_touch;
	bool is_rotation_enabled;
} s_info = {
	.is_sound_lock = false,
	.is_sound_touch = false,
	.is_rotation_enabled = false,
};

bool lock_property_sound_lock_get(void)
{
	return s_info.is_sound_lock;
}

bool lock_property_sound_touch_get(void)
{
	return s_info.is_sound_touch;
}

bool lock_property_rotation_enabled_get(void)
{
	return s_info.is_rotation_enabled;
}

lock_error_e lock_property_get_string(property_type_e type, void *key, char **str)
{
	int ret = 0;

	switch(type) {
	case PROPERTY_TYPE_SYSTEM_SETTINGS :
		ret = system_settings_get_value_string((int)key, &(*str));
		if (SYSTEM_SETTINGS_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("SYSTEM SETTINGS : key(%d), val(%s)", (int)key, *str);
		break;
	case PROPERTY_TYPE_SYSTEM_INFO :
		ret = system_info_get_platform_string((char *)key, &(*str));
		if (SYSTEM_INFO_ERROR_NONE != ret) {
			ret = -1;
		}
		break;
		_D("SYSTEM INFO : key(%s), val(%s)", (char *)key, *str);
	case PROPERTY_TYPE_RUNTIME_INFO :
		ret = runtime_info_get_value_string((int)key, &(*str));
		if (RUNTIME_INFO_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("RUNTIME INFO : key(%d), val(%s)", (int)key, *str);
		break;
	case PROPERTY_TYPE_VCONFKEY :
		*str = vconf_get_str((char *)key);
		if (!(*str)) {
			ret = -1;
		}
		_D("vconfkey : key(%s), val(%s)", (char *)key, *str);
		break;
	default :
		_E("Failed to get property. type error(%d)", type);
		return LOCK_ERROR_FAIL;
	}

	if (ret == -1) {
		_E("Failed to get property : type(%d), ret(%d)", type, ret);
		return LOCK_ERROR_FAIL;
	}

	return LOCK_ERROR_OK;
}

lock_error_e lock_property_get_bool(property_type_e type, void *key, void *val)
{
	int ret = 0;

	switch(type) {
	case PROPERTY_TYPE_SYSTEM_SETTINGS :
		ret = system_settings_get_value_bool((int)key, val);
		if (SYSTEM_SETTINGS_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("SYSTEM SETTINGS : key(%d), val(%d)", (int)key, *((bool *)val));
		break;
	case PROPERTY_TYPE_SYSTEM_INFO :
		ret = system_info_get_platform_bool((char *)key, val);
		if (SYSTEM_INFO_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("SYSTEM INFO : key(%s), val(%d)", (char *)key, *((bool *)val));
		break;
	case PROPERTY_TYPE_RUNTIME_INFO :
		ret = runtime_info_get_value_bool((int)key, val);
		if (RUNTIME_INFO_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("RUNTIME INFO : key(%d), val(%d)", (int)key, *((bool *)val));
		break;
	case PROPERTY_TYPE_VCONFKEY :
		ret = vconf_get_bool((char *)key, val);
		if (ret < 0) {
			ret = -1;
		}
		_D("vconfkey : key(%s), val(%d)", (char *)key, *((int *)val));
		break;
	default :
		_E("Failed to get property. type error(%d)", type);
		return LOCK_ERROR_FAIL;
	}

	if (ret == -1) {
		_E("Failed to get property : type(%d), ret(%d)", type, ret);
		return LOCK_ERROR_FAIL;
	}

	return LOCK_ERROR_OK;
}

lock_error_e lock_property_get_int(property_type_e type, void *key, int *val)
{
	int ret = 0;

	switch(type) {
	case PROPERTY_TYPE_SYSTEM_SETTINGS :
		ret = system_settings_get_value_int((int)key, val);
		if (SYSTEM_SETTINGS_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("SYSTEM SETTINGS : key(%d), val(%d)", (int)key, *val);
		break;
	case PROPERTY_TYPE_SYSTEM_INFO :
		ret = system_info_get_platform_int((char *)key, val);
		if (SYSTEM_INFO_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("SYSTEM INFO : key(%s), val(%d)", (char *)key, *val);
		break;
	case PROPERTY_TYPE_RUNTIME_INFO :
		ret = runtime_info_get_value_int((int)key, &(*val));
		if (RUNTIME_INFO_ERROR_NONE != ret) {
			ret = -1;
		}
		_D("RUNTIME INFO : key(%d), val(%d)", (int)key, *val);
		break;
	case PROPERTY_TYPE_VCONFKEY :
		ret = vconf_get_int((char *)key, &(*val));
		if (ret < 0) {
			ret = -1;
		}
		_D("vconfkey : key(%s), val(%d)", (char *)key, *val);
		break;
	default :
		_E("Failed to get property. type error(%d)", type);
		return LOCK_ERROR_FAIL;
	}

	if (ret == -1) {
		_E("Failed to get property : type(%d), ret(%d)", type, ret);
		return LOCK_ERROR_FAIL;
	}

	return LOCK_ERROR_OK;
}

void lock_property_register(void *data)
{
	bool val = false;

	s_info.is_sound_lock = val;
	s_info.is_sound_touch = val;
	s_info.is_rotation_enabled = val;

	_D("sound_lock(%d), sound_touch(%d), rotation(%d)", s_info.is_sound_lock, s_info.is_sound_touch, s_info.is_rotation_enabled);
}

void lock_property_unregister(void)
{
	_D("unregister property cb");
}
