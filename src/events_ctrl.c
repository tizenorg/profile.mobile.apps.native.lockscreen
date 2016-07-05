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
#include "minicontrollers.h"
#include "display.h"
#include "device_lock_ctrl.h"

#include <Ecore.h>
#include <time.h>
#include <Elementary.h>

#define MAX_EVENTS_SHOW_COUNT 3

static Ecore_Event_Handler *events_handler[2];
static Evas_Object *main_view, *noti_page, *media_page;

static Evas_Object *_lockscreen_events_view_ctrl_genlist_noti_content_get(void *data, Evas_Object *obj, const char *part);
static char *_lockscreen_events_view_ctrl_genlist_noti_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_lockscreen_events_view_ctrl_genlist_widget_content_get(void *data, Evas_Object *obj, const char *part);
static void _lockscreen_events_ctrl_events_reload();

static Elm_Genlist_Item_Class noti_itc = {
	.item_style = NOTI_ITEM_STYLE,
	.func.content_get = _lockscreen_events_view_ctrl_genlist_noti_content_get,
	.func.text_get = _lockscreen_events_view_ctrl_genlist_noti_text_get,
};

static Elm_Genlist_Item_Class widget_itc = {
	.item_style = WIDGET_ITEM_STYLE,
	.func.content_get = _lockscreen_events_view_ctrl_genlist_widget_content_get,
};

static Elm_Genlist_Item_Class noti_more_itc = {
	.item_style = NOTI_MORE_ITEM_STYLE,
};

static Evas_Object *_lockscreen_events_view_ctrl_genlist_noti_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *ret = NULL;
	lockscreen_event_t *event = eina_list_data_get(data);

	if (!strcmp(part, NOTI_ITEM_ICON)) {
		ret = elm_icon_add(obj);
		elm_image_fill_outside_set(ret, EINA_TRUE);
		elm_image_file_set(ret, lockscreen_event_icon_get(event), NULL);
	}
	else if (!strcmp(part, NOTI_ITEM_ICON_SUB)) {
		ret = elm_icon_add(obj);
		elm_image_file_set(ret, lockscreen_event_sub_icon_get(event), NULL);
	}
	return ret;
}

static char *_lockscreen_events_view_ctrl_genlist_noti_text_get(void *data, Evas_Object *obj, const char *part)
{
	lockscreen_event_t *event = eina_list_data_get(data);
	const char *val = NULL;

	if (!strcmp(part, NOTI_ITEM_TEXT)) {
		val = lockscreen_event_title_get(event);
	}
	else if (!strcmp(part, NOTI_ITEM_TEXT_SUB)) {
		val = lockscreen_event_content_get(event);
	}
	else if (!strcmp(part, NOTI_ITEM_TEXT_TIME)) {
		const char *locale = lockscreen_time_format_locale_get();
		const char *timezone = lockscreen_time_format_timezone_get();
		bool use24hformat = lockscreen_time_format_use_24h();
		val = util_time_string_get(lockscreen_event_time_get(event), locale, timezone, use24hformat);
		return (char*)val;
	}
	return val ? strdup(val) : NULL;
}

static Evas_Object *_lockscreen_events_view_ctrl_genlist_widget_content_get(void *data, Evas_Object *obj, const char *part)
{
	const char *name = data;

	if (!strcmp(part, WIDGET_ITEM_CONTENT)) {
		return lockscreen_minicontrollers_minicontroller_create(name, obj);
	}
	return NULL;
}


static void _lockscreen_events_view_cancel_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_events_view_page_panel_visible_set(noti_page, EINA_FALSE);
	lockscreen_main_view_contextual_view_fullscreen_set(main_view, EINA_FALSE);
	lockscreen_main_view_unlock_state_set(main_view, false, false);
	_lockscreen_events_ctrl_events_reload();
}

static void _lockscreen_events_view_clear_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	lockscreen_events_remove_all();
	lockscreen_main_view_contextual_view_fullscreen_set(main_view, EINA_FALSE);
	lockscreen_main_view_unlock_state_set(main_view, false, false);
}

static void _lockscreen_events_ctrl_view_show()
{
	Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
	if (!events_view) {
		events_view = lockscreen_events_view_create(main_view);
		lockscreen_main_view_part_content_set(main_view, PART_EVENTS, events_view);
	}
}

static void _lockscreen_events_ctrl_view_hide()
{
	Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
	if (events_view) {
		if (noti_page || media_page)
			return;
		lockscreen_main_view_part_content_unset(main_view, PART_EVENTS);
		evas_object_del(events_view);
		events_view = NULL;
	}
}

static int _lockscreen_events_ctrl_sort(const void *data1, const void *data2)
{
	const lockscreen_event_t *event1 = data1;
	const lockscreen_event_t *event2 = data2;

	time_t time1 = lockscreen_event_time_get(event1);
	time_t time2 = lockscreen_event_time_get(event2);

	return time1 > time2 ? -1 : 1;
}

static void _lockscreen_events_ctrl_item_selected(void *data, Evas_Object *obj, void *info)
{
	lockscreen_event_t *event = eina_list_data_get(data);
	lockscreen_main_view_contextual_view_fullscreen_set(main_view, false);
	//lockscreen_device_lock_ctrl_unlock_and_launch_request(event);

	elm_genlist_item_selected_set(info, EINA_FALSE);
	lockscreen_events_view_page_panel_visible_set(noti_page, EINA_FALSE);

	_lockscreen_events_ctrl_events_reload();
}

static void _lockscreen_events_ctrl_item_expand_request(void *data, Evas_Object *obj, void *info)
{
	Eina_List *l;

	// get last displayed notification
	Elm_Object_Item *prev = elm_genlist_item_prev_get(info);

	lockscreen_main_view_contextual_view_fullscreen_set(main_view, true);
	lockscreen_events_view_page_panel_visible_set(noti_page, EINA_TRUE);
	lockscreen_main_view_unlock_state_set(main_view, false, true);

	l = elm_object_item_data_get(prev);
	while ((l = eina_list_next(l)))
		elm_genlist_item_append(obj, &noti_itc, l, NULL, ELM_GENLIST_ITEM_NONE, _lockscreen_events_ctrl_item_selected, l);
	elm_object_item_del(info);
}

static void _lockscreen_events_ctrl_item_expand_selected(void *data, Evas_Object *obj, void *info)
{
	elm_genlist_item_expanded_set(info, EINA_TRUE);
}

static void _lockscreen_events_ctrl_expand_gesture_finished(void *data, Evas_Object *obj, void *info)
{
	Elm_Object_Item *it = elm_genlist_first_item_get(data);
	do {
		if (elm_genlist_item_type_get(it) == ELM_GENLIST_ITEM_TREE)
			elm_genlist_item_expanded_set(it, EINA_TRUE);
	} while ((it = elm_genlist_item_next_get(it)));
}

static void _lockscreen_events_ctrl_events_reload()
{
	Eina_List *l;
	Evas_Object *genlist = NULL;
	static Eina_List *events;
	int i, max;

	lockscreen_display_timer_renew();

	if (!noti_page) {
		Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
		noti_page = lockscreen_events_view_page_prepend(events_view);
		genlist = lockscreen_events_view_page_genlist_get(noti_page);
		evas_object_smart_callback_add(genlist, "expanded", _lockscreen_events_ctrl_item_expand_request, NULL);
		evas_object_smart_callback_add(noti_page, SIGNAL_PAGE_EXPAND_GESTURE, _lockscreen_events_ctrl_expand_gesture_finished, genlist);
		evas_object_smart_callback_add(noti_page, SIGNAL_PAGE_CANCEL_BUTTON_CLICKED, _lockscreen_events_view_cancel_button_clicked, NULL);
		evas_object_smart_callback_add(noti_page, SIGNAL_PAGE_CLEAR_BUTTON_CLICKED, _lockscreen_events_view_clear_button_clicked, NULL);
	}

	if (!genlist)
		genlist = lockscreen_events_view_page_genlist_get(noti_page);

	elm_genlist_clear(genlist);

	if (events) eina_list_free(events);
	events = lockscreen_events_get();
	events = eina_list_sort(events, -1, _lockscreen_events_ctrl_sort);

	if (lockscreen_events_view_page_panel_visible_get(noti_page))
		max = INT_MAX;
	else
		max = MAX_EVENTS_SHOW_COUNT;

	for (i = 0, l = events; l && (i < max); i++, l = eina_list_next(l))
		elm_genlist_item_append(genlist, &noti_itc, l, NULL, ELM_GENLIST_ITEM_NONE, _lockscreen_events_ctrl_item_selected, l);
	if (eina_list_count(events) > max)
		elm_genlist_item_append(genlist, &noti_more_itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, _lockscreen_events_ctrl_item_expand_selected, genlist);
}

static Eina_Bool _lockscreen_events_ctrl_events_changed(void *data, int event, void *event_info)
{
	if (lockscreen_events_exists()) {
		_lockscreen_events_ctrl_view_show();
		_lockscreen_events_ctrl_events_reload();
	}
	else {
		Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
		if (noti_page) lockscreen_events_view_page_del(events_view, noti_page);
		noti_page = NULL;
		_lockscreen_events_ctrl_view_hide();
	}

	return EINA_TRUE;
}

static void _lockscreen_events_ctrl_minicontrollers_reload(const Eina_List *minis)
{
	const char *name;
	const Eina_List *l;

	Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
	if (!media_page) media_page = lockscreen_events_view_page_append(events_view);

	Evas_Object *genlist = lockscreen_events_view_page_genlist_get(media_page);
	if (!genlist) {
		FAT("lockscreen_events_genlist_get failed");
		return;
	}

	elm_genlist_clear(genlist);

	EINA_LIST_FOREACH(minis, l, name) {
		elm_genlist_item_append(genlist, &widget_itc, name, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}
}

static Eina_Bool _lockscreen_events_ctrl_minicontrollers_changed(void *data, int event, void *event_info)
{
	Eina_List *minicontrollers = lockscreen_minicontrollers_list_get();
	if (minicontrollers) {
		_lockscreen_events_ctrl_view_show();
		_lockscreen_events_ctrl_minicontrollers_reload(minicontrollers);
		eina_list_free(minicontrollers);
	} else {
		Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
		if (media_page) lockscreen_events_view_page_del(events_view, media_page);
		media_page = NULL;
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

	if (lockscreen_minicontrollers_init()) {
		FAT("lockscreen_minicontrollers_init failed.");
		goto mini_failed;
	}

	if (lockscreen_time_format_init()) {
		FAT("lockscreen_time_format_init failed.");
		goto time_failed;
	}

	if (lockscreen_display_init()) {
		FAT("lockscreen_display_init failed.");
		goto display_failed;
	}

	main_view = mv;

	events_handler[0] = ecore_event_handler_add(LOCKSCREEN_EVENT_EVENTS_CHANGED, _lockscreen_events_ctrl_events_changed, NULL);
	events_handler[1] = ecore_event_handler_add(LOCKSCREEN_EVENT_MINICONTROLLERS_CHANGED, _lockscreen_events_ctrl_minicontrollers_changed, NULL);

	return 0;

display_failed:
	lockscreen_time_format_shutdown();
time_failed:
	lockscreen_minicontrollers_shutdown();
mini_failed:
	lockscreen_events_shutdown();

	return 1;
}

void lockscreen_events_ctrl_shutdown()
{
	ecore_event_handler_del(events_handler[0]);
	ecore_event_handler_del(events_handler[1]);
	lockscreen_events_remove_all();
}
