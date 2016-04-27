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

#include "password_view.h"
#include "log.h"
#include "lockscreen.h"
#include "util.h"

#include <Elementary.h>

static Evas_Object* _lockscreen_password_view_pin_create(Evas_Object *parent)
{
	Evas_Object *ly = elm_layout_add(parent);
	if (!elm_layout_file_set(ly, util_get_res_file_path(LOCK_EDJE_FILE), "lock-simple-password")) {
		ERR("elm_layout_file_set failed.");
		evas_object_del(ly);
		return NULL;
	}
	evas_object_show(ly);

	return ly;
}

static Evas_Object* _lockscreen_password_view_password_create(Evas_Object *parent)
{
	return NULL;
}

Evas_Object *lockscreen_password_view_create(lockscreen_password_view_type type, Evas_Object *parent)
{
	Evas_Object *ret = NULL;
	switch (type) {
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN:
			ret = _lockscreen_password_view_pin_create(parent);
			break;
		case LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD:
			ret = _lockscreen_password_view_password_create(parent);
			break;
	}
	return ret;
}
