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
#include "events_ctrl.h"
#include "events_view.h"
#include "main_view.h"
#include "events.h"
#include "time_format.h"
#include "util_time.h"

#include <Ecore.h>
#include <time.h>
#include <app.h>

static Ecore_Event_Handler *events_handler;
static Evas_Object *main_view;

static Evas_Object *_lockscreen_events_view_ctrl_genlist_noti_content_get(void *data, Evas_Object *obj, const char *part);
static char *_lockscreen_events_view_ctrl_genlist_noti_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_lockscreen_events_view_ctrl_genlist_widget_content_get(void *data, Evas_Object *obj, const char *part);

static Elm_Genlist_Item_Class noti_itc = {
	.item_style = NOTI_ITEM_STYLE,
	.func.content_get = _lockscreen_events_view_ctrl_genlist_noti_content_get,
	.func.text_get = _lockscreen_events_view_ctrl_genlist_noti_text_get,
};

static Elm_Genlist_Item_Class widget_itc = {
	.item_style = WIDGET_ITEM_STYLE,
	.func.content_get = _lockscreen_events_view_ctrl_genlist_widget_content_get,
};

static Evas_Object *_lockscreen_events_view_ctrl_genlist_noti_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *ret = NULL;
	lockscreen_event_t *event = data;

	if (!strcmp(part, NOTI_ITEM_ICON)) {
		ret = elm_icon_add(obj);
		elm_image_file_set(ret, lockscreen_event_icon_get(event), NULL);
	}
	else if (!strcmp(part, NOTI_ITEM_ICON_SUB)) {
		ret = elm_icon_add(obj);
		elm_image_file_set(ret, lockscreen_event_sub_icon_get(event), NULL);
	}
	return ret;
}

static char *_lockscreen_events_view_ctrl_genlist_noti_text_time_get(time_t time)
{
	const char *locale = lockscreen_time_format_locale_get();
	const char *timezone = lockscreen_time_format_timezone_get();
	bool use24hformat = lockscreen_time_format_use_24h();
	char *str_time, *str_meridiem;
	char time_buf[PATH_MAX] = {0,};

	if (!util_time_formatted_time_get(time, locale, timezone, use24hformat, &str_time, &str_meridiem)) {
		ERR("util_time_formatted_time_get failed");
		return NULL;
	}

	if (use24hformat) {
		snprintf(time_buf, sizeof(time_buf), "%s", str_time);
	} else {
		snprintf(time_buf, sizeof(time_buf), "%s %s", str_time, str_meridiem);
	}

	free(str_time);
	free(str_meridiem);
	return strdup(time_buf);
}

static char *_lockscreen_events_view_ctrl_genlist_noti_text_get(void *data, Evas_Object *obj, const char *part)
{
	lockscreen_event_t *event = data;
	const char *val = NULL;

	if (!strcmp(part, NOTI_ITEM_TEXT)) {
		val = lockscreen_event_title_get(event);
	}
	else if (!strcmp(part, NOTI_ITEM_TEXT_SUB)) {
		val = lockscreen_event_content_get(event);
	}
	else if (!strcmp(part, NOTI_ITEM_TEXT_TIME)) {
		val = _lockscreen_events_view_ctrl_genlist_noti_text_time_get(lockscreen_event_time_get(event));
		return (char*)val;
	}
	return val ? strdup(val) : NULL;
}

static Evas_Object *_lockscreen_events_view_ctrl_genlist_widget_content_get(void *data, Evas_Object *obj, const char *part)
{
	lockscreen_event_t *event = data;

	if (!strcmp(part, WIDGET_ITEM_CONTENT)) {
		return lockscreen_event_minicontroller_create(event, obj);
	}
	return NULL;
}

static void _lockscreen_events_view_close_all_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_events_remove_all();
}

static void _lockscreen_events_ctrl_view_show()
{
	Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
	if (!events_view) {
		events_view = lockscreen_events_view_create(main_view);
		evas_object_smart_callback_add(events_view, SIGNAL_CLOSE_BUTTON_CLICKED, _lockscreen_events_view_close_all_button_clicked, NULL);
		lockscreen_main_view_part_content_set(main_view, PART_EVENTS, events_view);
	}
}

static void _lockscreen_events_ctrl_view_hide()
{
	Evas_Object *events_view = lockscreen_main_view_part_content_unset(main_view, PART_EVENTS);
	if (events_view) {
		evas_object_del(events_view);
	}
}

static int _lockscreen_events_ctrl_sort(const void *data1, const void *data2)
{
	const lockscreen_event_t *event1 = data1;
	const lockscreen_event_t *event2 = data2;

	lockscreen_event_type_e type1 = lockscreen_event_type_get(event1);
	lockscreen_event_type_e type2 = lockscreen_event_type_get(event2);

	if (type1 != type2)
		return type1 > type2 ? -1 : 1;

	time_t time1 = lockscreen_event_time_get(event1);
	time_t time2 = lockscreen_event_time_get(event2);

	return time1 > time2 ? -1 : 1;
}

static void _lockscreen_events_ctrl_main_view_unlocked(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void _lockscreen_events_ctrl_launch_result(bool result)
{
	if (result) {
		evas_object_smart_callback_add(main_view, SIGNAL_UNLOCK_ANIMATION_FINISHED, _lockscreen_events_ctrl_main_view_unlocked, NULL);
		lockscreen_main_view_unlock(main_view);
	} else {
		INF("Failed to launch application");
	}
}

static void _lockscreen_events_ctrl_item_selected(void *data, Evas_Object *obj, void *info)
{
	lockscreen_event_t *event = data;
	lockscreen_event_launch(event, _lockscreen_events_ctrl_launch_result);
}

static void _lockscreen_events_ctrl_events_load()
{
	lockscreen_event_t *event;

	Evas_Object *genlist = lockscreen_events_genlist_get(lockscreen_main_view_part_content_get(main_view, PART_EVENTS));
	if (!genlist) {
		FAT("lockscreen_events_genlist_get failed");
		return;
	}

	Eina_List *events = lockscreen_events_get();
	events = eina_list_sort(events, -1, _lockscreen_events_ctrl_sort);
	EINA_LIST_FREE(events, event) {
		lockscreen_event_type_e type = lockscreen_event_type_get(event);

		switch (type) {
			case LOCKSCREEN_EVENT_TYPE_NOTIFICATION:
				elm_genlist_item_append(genlist, &noti_itc, event, NULL, ELM_GENLIST_ITEM_NONE, _lockscreen_events_ctrl_item_selected, event);
				break;
			case LOCKSCREEN_EVENT_TYPE_MINICONTROLLER:
				elm_genlist_item_append(genlist, &widget_itc, event, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}
}

static void _lockscreen_events_ctrl_events_unload()
{
	Evas_Object *genlist = lockscreen_events_genlist_get(lockscreen_main_view_part_content_get(main_view, PART_EVENTS));
	elm_genlist_clear(genlist);
}

static Eina_Bool _lockscreen_events_ctrl_events_changed(void *data, int event, void *event_info)
{
	/* Improve load/unload in the future */
	_lockscreen_events_ctrl_events_unload();

	if (lockscreen_events_exists()) {
		_lockscreen_events_ctrl_view_show();
		_lockscreen_events_ctrl_events_load();
	}
	else {
		_lockscreen_events_ctrl_view_hide();
	}

	return EINA_TRUE;
}

int lockscreen_events_ctrl_init(Evas_Object *mv)
{
	if (lockscreen_events_init()) {
		FAT("lockscreen_events_init failed.");
		return 1;
	}

	if (lockscreen_time_format_init()) {
		FAT("lockscreen_time_format_init failed.");
		lockscreen_events_shutdown();
	}

	main_view = mv;

	events_handler = ecore_event_handler_add(LOCKSCREEN_EVENT_EVENTS_CHANGED, _lockscreen_events_ctrl_events_changed, NULL);

	if (lockscreen_events_exists()) {
		_lockscreen_events_ctrl_view_show();
	}
	else
		_lockscreen_events_ctrl_view_hide();

	return 0;
}

void lockscreen_events_ctrl_shutdown()
{
	ecore_event_handler_del(events_handler);
	lockscreen_events_shutdown();
	lockscreen_time_format_shutdown();
}
