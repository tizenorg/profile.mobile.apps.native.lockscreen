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

#include "camera_view.h"
#include "util.h"
#include "log.h"
#include "lockscreen.h"
#include "util_time.h"

#include <Elementary.h>

static void _camera_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	evas_object_smart_callback_call(data, SIGNAL_CAMERA_SELECTED, NULL);
}

Evas_Object *lockscreen_camera_view_create(Evas_Object *parent)
{
	Evas_Object *cam_ly = elm_layout_add(parent);
	if (!elm_layout_file_set(cam_ly, util_get_res_file_path(LOCK_EDJE_FILE), "camera-layout")) {
		FAT("elm_layout_file_set failed");
		return false;
	}
	elm_object_signal_callback_add(cam_ly, "camera,icon,clicked", "camera-layout", _camera_clicked, cam_ly);

	evas_object_show(cam_ly);
	return cam_ly;
}
