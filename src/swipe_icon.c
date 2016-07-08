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

#include "swipe_icon.h"
#include "util.h"
#include "log.h"
#include "lockscreen.h"
#include "util_time.h"

#include <Elementary.h>

static void _icon_selected(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	evas_object_smart_callback_call(data, SIGNAL_ICON_SELECTED, NULL);
}

static Evas_Object *_image_create(Evas_Object *parent, const char *icon_path)
{
	Evas_Object *image = elm_image_add(parent);

	if (!elm_image_file_set(image, icon_path, NULL)) {
		ERR("elm_image_file_set failed");
		evas_object_del(image);
		return NULL;
	}

	evas_object_size_hint_fill_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return image;
}

void lockscreen_swipe_icon_text_set(Evas_Object *ly, const char *text)
{
	elm_object_part_text_set(ly, "txt.main", text);
}

Evas_Object *lockscreen_swipe_icon_view_create(Evas_Object *parent, const char *icon_path)
{
	Evas_Object *image = NULL;
	Evas_Object *cam_ly = elm_layout_add(parent);

	image = _image_create(cam_ly, icon_path);
	if (!image) {
		ERR("_image_create failed");
		evas_object_del(cam_ly);
		return NULL;
	}

	if (!elm_layout_file_set(cam_ly, util_get_res_file_path(LOCK_EDJE_FILE), "swipe-icon-layout")) {
		FAT("elm_layout_file_set failed");
		return false;
	}

	elm_object_signal_callback_add(cam_ly, "swipe_icon,icon,clicked", "swipe-icon-layout",
			_icon_selected, cam_ly);


	elm_object_part_content_set(cam_ly, "sw.icon", image);
	evas_object_show(image);

	evas_object_show(cam_ly);
	return cam_ly;
}

void lockscreen_swipe_icon_view_reset(Evas_Object *view)
{
	elm_object_signal_emit(view, "swipe_icon,reset", "lockscreen");
}
