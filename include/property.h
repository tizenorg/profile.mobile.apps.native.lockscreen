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

#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <runtime_info.h>
#include <system_info.h>
#include <system_settings.h>

#include <app_preference.h>

#include <vconf.h>

typedef enum {
	PROPERTY_TYPE_SYSTEM_SETTINGS = 0,
	PROPERTY_TYPE_SYSTEM_INFO = 1,
	PROPERTY_TYPE_RUNTIME_INFO = 2,
	PROPERTY_TYPE_PREFERENCE = 3,
	PROPERTY_TYPE_VCONFKEY = 4,
	PROPERTY_TYPE_MAX,
} property_type_e;

bool lock_property_sound_lock_get(void);
bool lock_property_sound_touch_get(void);
bool lock_property_rotation_enabled_get(void);

lock_error_e lock_property_get_string(property_type_e type, void *key, char **str);
lock_error_e lock_property_get_bool(property_type_e type, void *key, void *val);
lock_error_e lock_property_get_int(property_type_e type, void *key, int *val);

void lock_property_register(void *data);
void lock_property_unregister(void);

#endif
