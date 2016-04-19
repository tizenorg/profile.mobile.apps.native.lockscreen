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

#include <minicontrol-viewer.h>
#include <Ecore.h>
#include <Eina.h>

#include "log.h"
#include "minicontrollers.h"

typedef struct {
	const char *name;
	int width, height;
} minicontroller_info_t;

static Eina_List *active_minicontroller;
int LOCKSCREEN_EVENT_MINICONTROLLERS_CHANGED, init_count;

static int _lockscreen_minicontroller_search(const void *data1, const void *data2);

static minicontroller_info_t *_minicontroller_create(const char *name, int w, int h)
{
	minicontroller_info_t *ret = calloc(1, sizeof(minicontroller_info_t));

	ret->name = eina_stringshare_add(name);
	ret->width = w;
	ret->height = h;

	return ret;
}

static void _minicontroller_destroy(minicontroller_info_t *info)
{
	eina_stringshare_del(info->name);
	free(info);
}

static void _minicontroller_start_handle(const char *name, int w, int h)
{
	/** FIXME Since minicontroller API do not allow to filter minicontrollers
	 * targeted for lockscreen we just asume that interesting minicontrollers
	 * has proper suffix in its name */
	if (name && strstr(name, "LOCKSCREEN")) {
		minicontroller_info_t *info = eina_list_search_unsorted(active_minicontroller, _lockscreen_minicontroller_search, name);
		if (name) {
			info = _minicontroller_create(name, w, h);
			active_minicontroller = eina_list_append(active_minicontroller, info);
			ecore_event_add(LOCKSCREEN_EVENT_MINICONTROLLERS_CHANGED, NULL, NULL, NULL);
		}
	}
}

static void _minicontroller_stop_handle(const char *name)
{
	if (name) {
		minicontroller_info_t *info = eina_list_search_unsorted(active_minicontroller, _lockscreen_minicontroller_search, name);
		if (info) {
			active_minicontroller = eina_list_remove(active_minicontroller, info);
			_minicontroller_destroy(info);
			ecore_event_add(LOCKSCREEN_EVENT_MINICONTROLLERS_CHANGED, NULL, NULL, NULL);
		}
	}
}

static void _minicontroller_geometry_from_bundle_get(bundle *event_arg, int *width, int *height)
{
	int *val;
	size_t val_size;
	if (!event_arg)
		return;
	int ret = bundle_get_byte(event_arg, "width", (void**)&val, &val_size);
	if (ret == BUNDLE_ERROR_NONE) {
		if (width) *width = *val;
	}
	ret = bundle_get_byte(event_arg, "height", (void**)&val, &val_size);
	if (ret == BUNDLE_ERROR_NONE) {
		if (height) *height = *val;
	}
}

static void _minicontroler_event(minicontrol_event_e event, const char *minicontrol_name, bundle *event_arg, void *data)
{
	int w = 0, h = 0;
	if (!minicontrol_name)
		return;

	DBG("Available minicontroller: %s", minicontrol_name);

	switch (event) {
		case MINICONTROL_EVENT_START:
			_minicontroller_geometry_from_bundle_get(event_arg, &w, &h);
			_minicontroller_start_handle(minicontrol_name, w, h);
			break;
		case MINICONTROL_EVENT_STOP:
			_minicontroller_stop_handle(minicontrol_name);
			break;
		default:
			DBG("Unahandled minicontroller event: %d for %s", event, minicontrol_name);
			break;
	}
}

int lockscreen_minicontrollers_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_MINICONTROLLERS_CHANGED = ecore_event_type_new();
		int ret = minicontrol_viewer_set_event_cb(_minicontroler_event, NULL);
		if (ret != MINICONTROL_ERROR_NONE) {
			ERR("minicontrol_viewer_set_event_cb failed: %s", get_error_message(ret));
			return 1;
		}
	}
	init_count++;
	return 0;
}

void lockscreen_minicontrollers_shutdown(void)
{
	minicontroller_info_t *info;
	if (init_count) {
		init_count--;
		if (!init_count) {
			int ret = minicontrol_viewer_unset_event_cb();
			if (ret != MINICONTROL_ERROR_NONE) {
				ERR("minicontrol_viewer_unset_event_cb failed: %s", get_error_message(ret));
			}
			EINA_LIST_FREE(active_minicontroller, info) {
				_minicontroller_destroy(info);
			}
			active_minicontroller = NULL;
		}
	}
}

static int _lockscreen_minicontroller_search(const void *data1, const void *data2)
{
	const minicontroller_info_t *info = data1;
	const char *name = data2;
	return strcmp(name, info->name);
}

Evas_Object *lockscreen_minicontrollers_minicontroller_create(const char *name, Evas_Object *parent)
{
	minicontroller_info_t *info = eina_list_search_unsorted(active_minicontroller, _lockscreen_minicontroller_search, name);
 
	if (!info) {
		ERR("Invalid minicontroller name: %s", name);
		return NULL;
	}

	Evas_Object *ret = minicontrol_viewer_add(parent, info->name);
	evas_object_size_hint_min_set(ret, info->width, info->height);
	evas_object_show(ret);
	return ret;
}

Eina_List *lockscreen_minicontrollers_list_get(void)
{
	Eina_List *ret = NULL, *l;
	minicontroller_info_t *info;

	EINA_LIST_FOREACH(active_minicontroller, l, info) {
		ret = eina_list_append(ret, info->name);
	}
	return ret;
}

bool lockscreen_minicontrollers_minicontroller_stop(const char *name)
{
	int ret = minicontrol_viewer_send_event(name, MINICONTROL_VIEWER_EVENT_HIDE, NULL);
	if (ret != MINICONTROL_ERROR_NONE) {
		ERR("minicontrol_viewer_send_event failed: %s", get_error_message(ret));
		return false;
	}
	return true;
}
