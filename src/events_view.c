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

#include "events_view.h"
#include "util.h"
#include "log.h"
#include "lockscreen.h"
#include "util_time.h"
#include "window.h"

#include <Elementary.h>

#define DATA_KEY "__events_view_data_key"

struct Events_View_Data {
	Evas_Object *layout;
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *index;
	Eina_Bool scroller_is_animating : 1;
};

struct Event_Page_Data {
	Evas_Object *layout;
	Evas_Object *genlist;
	Evas_Object *events_view;
	Evas_Object *gesture_layer;
	Eina_Bool panel_shown : 1;
};

static void
_lockscreen_events_view_scroller_thaw(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	struct Events_View_Data *ed = data;
	elm_scroller_movement_block_set(ed->scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
}

static void
_lockscreen_events_view_scroller_hit_rectangle_mouse_up(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	struct Events_View_Data *ed = data;
	if (!ed->scroller_is_animating)
		elm_scroller_movement_block_set(ed->scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL | ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
}

static void
_lockscreen_events_view_scroller_hit_rectangle_resize(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	int x, y, w, h;
	Evas_Object *track = data;
	evas_object_geometry_get(eo, &x, &y, &w, &h);
	evas_object_geometry_set(track, x, y, w, h);
	evas_object_layer_set(track, evas_object_layer_get(eo) + 1);
}

static void
_lockscreen_events_view_del(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	evas_object_del(data);
}

static void
_lockscreen_events_view_free_on_del(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	free(data);
}

static void _lockscreen_events_view_scroller_hit_rectangle_add(Evas_Object *obj, struct Events_View_Data *data)
{
	Evas_Object *track = evas_object_rectangle_add(evas_object_evas_get(obj));
	evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _lockscreen_events_view_scroller_hit_rectangle_resize, track);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _lockscreen_events_view_scroller_hit_rectangle_resize, track);
	evas_object_color_set(track, 0, 0, 0, 0);
	evas_object_event_callback_add(track, EVAS_CALLBACK_MOUSE_DOWN, _lockscreen_events_view_scroller_thaw, data);
	evas_object_event_callback_add(track, EVAS_CALLBACK_MOUSE_UP, _lockscreen_events_view_scroller_hit_rectangle_mouse_up, data);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _lockscreen_events_view_del, track);
	evas_object_repeat_events_set(track, EINA_TRUE);
	evas_object_show(track);
}

static void
_lockscreen_events_view_scroller_anim_stopped(void *data, Evas_Object *eo, void *event_info)
{
	struct Events_View_Data *ed = data;
	elm_scroller_movement_block_set(ed->scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL | ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	ed->scroller_is_animating = EINA_FALSE;
}

static void
_lockscreen_events_view_scroller_drag_start(void *data, Evas_Object *eo, void *event_info)
{
	struct Events_View_Data *ed = data;
	ed->scroller_is_animating = EINA_TRUE;
}

static Evas_Object*
_lockscreen_events_view_index_create(Evas_Object *parent)
{
	Evas_Object *ret = elm_index_add(parent);

	util_lockscreen_theme_get();

	evas_object_size_hint_weight_set(ret, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ret, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Define in tizen-theme */
	elm_object_style_set(ret, "pagecontrol");

	elm_index_horizontal_set(ret, EINA_TRUE);
	elm_index_autohide_disabled_set(ret, EINA_TRUE);
	evas_object_show(ret);

	elm_index_level_go(ret, 0);

	return ret;
}

static void
_lockscreen_events_view_scroller_page_index_update(struct Events_View_Data *data)
{
	int h;
	elm_scroller_current_page_get(data->scroller, &h, NULL);

	Eina_List *childs = elm_box_children_get(data->box);
	Evas_Object *page = eina_list_nth(childs, h);
	if (page) {
		Elm_Object_Item *idx = evas_object_data_get(page, "__idx");
		elm_index_item_selected_set(idx, EINA_TRUE);
	}
	eina_list_free(childs);
}

static void
_lockscreen_events_view_scroller_page_changed(void *data, Evas_Object *obj, void *event_info)
{
	_lockscreen_events_view_scroller_page_index_update(data);
}

Evas_Object *lockscreen_events_view_create(Evas_Object *parent)
{
	struct Events_View_Data *data = calloc(1, sizeof(struct Events_View_Data));

	data->layout = elm_layout_add(parent);
	if (!elm_layout_file_set(data->layout, util_get_res_file_path(LOCK_EDJE_FILE), "contextual-event")) {
		FAT("elm_layout_file_set failed for contextual-event");
		evas_object_del(data->layout);
		free(data);
		return NULL;
	}
	evas_object_data_set(data->layout, DATA_KEY, data);
	evas_object_show(data->layout);
	evas_object_event_callback_add(data->layout, EVAS_CALLBACK_DEL, _lockscreen_events_view_free_on_del, data);

	/* Load theme extension */
	util_lockscreen_theme_get();

	data->scroller = elm_scroller_add(data->layout);
	evas_object_size_hint_weight_set(data->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(data->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_movement_block_set(data->scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL | ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	elm_scroller_content_min_limit(data->scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_page_size_set(data->scroller, lock_window_width_get(), 0);
	elm_scroller_page_scroll_limit_set(data->scroller, 1, 1);
	evas_object_smart_callback_add(data->scroller, "scroll,anim,stop", _lockscreen_events_view_scroller_anim_stopped, data);
	evas_object_smart_callback_add(data->scroller, "scroll,drag,start", _lockscreen_events_view_scroller_drag_start, data);
	elm_layout_theme_set(data->scroller, "scroller", "base", "pass_effect");
	evas_object_show(data->scroller);

	data->box = elm_box_add(data->scroller);
	elm_box_horizontal_set(data->box, EINA_TRUE);
	evas_object_size_hint_weight_set(data->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(data->box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_align_set(data->box, 0, 0);
	elm_box_homogeneous_set(data->box, EINA_TRUE);
	evas_object_show(data->box);

	elm_object_part_content_set(data->layout, "sw.scroller", data->scroller);
	elm_object_content_set(data->scroller, data->box);

	elm_scroller_page_show(data->scroller, 0, 0);

	/* Mark scroller as locked (scrolling freezed) */
	data->scroller_is_animating = EINA_FALSE;

	data->index = _lockscreen_events_view_index_create(data->layout);
	elm_object_part_content_set(data->layout, "sw.index", data->index);

	evas_object_smart_callback_add(data->scroller, "scroll,page,changed", _lockscreen_events_view_scroller_page_changed, data);

	return data->layout;
}

static void
_lockscreen_events_view_page_cancel_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_PAGE_CANCEL_BUTTON_CLICKED, NULL);
}

static void
_lockscreen_events_view_page_clear_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_PAGE_CLEAR_BUTTON_CLICKED, NULL);
}

static Evas_Event_Flags _swipe_state_end(void *data, void *event_info)
{
	struct Event_Page_Data *pd = data;
	Elm_Gesture_Line_Info *li = event_info;

	// Emit callback only if flick originated above any genlist item
	if (!pd->panel_shown && elm_genlist_at_xy_item_get(pd->genlist, li->momentum.x1, li->momentum.y1, NULL)) {
		evas_object_smart_callback_call(pd->layout, SIGNAL_PAGE_EXPAND_GESTURE, NULL);
	}
	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Object*
_lockscreen_events_view_page_create(Evas_Object *events_view)
{
	struct Event_Page_Data *pd = calloc(1, sizeof(struct Event_Page_Data));

	pd->events_view = events_view;
	pd->layout = elm_layout_add(events_view);
	if (!elm_layout_file_set(pd->layout, util_get_res_file_path(LOCK_EDJE_FILE), "context-page")) {
		FAT("elm_layout_file_set failed");
		evas_object_del(pd->layout);
		free(pd);
		return NULL;
	}

	evas_object_event_callback_add(pd->layout, EVAS_CALLBACK_DEL, _lockscreen_events_view_free_on_del, pd);
	evas_object_data_set(pd->layout, DATA_KEY, pd);
	evas_object_size_hint_align_set(pd->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pd->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	pd->genlist = elm_genlist_add(pd->layout);
	evas_object_size_hint_align_set(pd->genlist, 0.5, 0);
	evas_object_size_hint_weight_set(pd->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_bounce_set(pd->genlist, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(pd->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_tree_focus_allow_set(pd->genlist, EINA_TRUE);
	elm_layout_theme_set(pd->genlist, "scroller", "base", "pass_effect");

	/* Load theme extension */
	util_lockscreen_theme_get();

	Evas_Object *btn = elm_button_add(pd->layout);
	evas_object_show(btn);
	elm_layout_theme_set(btn, "button", "panel", "default");
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CLEAR_ALL"));
	evas_object_smart_callback_add(btn, "clicked", _lockscreen_events_view_page_clear_button_clicked, pd->layout);
	elm_object_part_content_set(pd->layout, "sw.left", btn);

	btn = elm_button_add(pd->layout);
	evas_object_show(btn);
	elm_layout_theme_set(btn, "button", "panel", "default");
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL"));
	evas_object_smart_callback_add(btn, "clicked", _lockscreen_events_view_page_cancel_button_clicked, pd->layout);
	elm_object_part_content_set(pd->layout, "sw.right", btn);

	evas_object_show(pd->genlist);
	evas_object_show(pd->layout);

	elm_object_part_content_set(pd->layout, "sw.content", pd->genlist);
	lockscreen_events_view_page_panel_visible_set(pd->layout, EINA_FALSE);

	return pd->layout;
}

void lockscreen_events_view_page_panel_visible_set(Evas_Object *page, Eina_Bool visible)
{
	struct Event_Page_Data *data = evas_object_data_get(page, DATA_KEY);

	if (visible) {
		elm_object_scroll_lock_x_set(page, 1);
		elm_object_signal_emit(page, "panel,show", "lockscreen");
		data->panel_shown = EINA_TRUE;
		elm_object_signal_emit(data->events_view, "index,hide", "lockscreen");
		if (data->gesture_layer) {
			evas_object_del(data->gesture_layer);
			data->gesture_layer = NULL;
		}
	} else {
		elm_object_scroll_lock_x_set(page, 0);
		elm_object_signal_emit(page, "panel,hide", "lockscreen");
		data->panel_shown = EINA_FALSE;
		elm_object_signal_emit(data->events_view, "index,show", "lockscreen");
		if (!data->gesture_layer) {
			data->gesture_layer = elm_gesture_layer_add(data->layout);
			elm_gesture_layer_attach(data->gesture_layer, data->genlist);
			elm_gesture_layer_flick_time_limit_ms_set(data->gesture_layer, 500);
			elm_gesture_layer_cb_set(data->gesture_layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _swipe_state_end, data);
			elm_gesture_layer_line_min_length_set(data->gesture_layer, 140 * edje_scale_get() / edje_object_scale_get(elm_layout_edje_get(data->layout)));
			evas_object_show(data->gesture_layer);
		}

	}
}

static void
_lockscreen_events_view_page_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *index = elm_object_item_widget_get(data);
	elm_object_item_del(data);
	elm_index_level_go(index, 0);
}

static void
_lockscreen_events_view_page_add(struct Events_View_Data *data, Evas_Object *page, Eina_Bool append)
{
	Elm_Object_Item *idx;
	Evas_Object *swipe_area;

	if (append) {
		elm_box_pack_end(data->box, page);
		idx = elm_index_item_append(data->index, NULL, NULL, page);
	} else {
		elm_box_pack_start(data->box, page);
		idx = elm_index_item_prepend(data->index, NULL, NULL, page);
	}

	swipe_area = (Evas_Object*)edje_object_part_object_get(elm_layout_edje_get(page), "swipe.area");
	_lockscreen_events_view_scroller_hit_rectangle_add(swipe_area, data);

	elm_index_level_go(data->index, 0);
	evas_object_data_set(page, "__idx", idx);
	evas_object_event_callback_add(page, EVAS_CALLBACK_DEL, _lockscreen_events_view_page_del, idx);
	_lockscreen_events_view_scroller_page_index_update(data);
}

Evas_Object *lockscreen_events_view_page_append(Evas_Object *events_view)
{
	struct Events_View_Data *data = evas_object_data_get(events_view, DATA_KEY);
	Evas_Object *page = _lockscreen_events_view_page_create(events_view);
	if (!page) {
		FAT("_lockscreen_events_view_page_create failed");
		return NULL;
	}
	_lockscreen_events_view_page_add(data, page, EINA_TRUE);
	return page;
}

Evas_Object *lockscreen_events_view_page_prepend(Evas_Object *events_view)
{
	struct Events_View_Data *data = evas_object_data_get(events_view, DATA_KEY);

	Evas_Object *page = _lockscreen_events_view_page_create(events_view);
	if (!page) {
		FAT("_lockscreen_events_view_page_create failed");
		return NULL;
	}
	_lockscreen_events_view_page_add(data, page, EINA_FALSE);
	return page;
}

Evas_Object *lockscreen_events_view_page_genlist_get(Evas_Object *page)
{
	struct Event_Page_Data *data = evas_object_data_get(page, DATA_KEY);
	return data->genlist;
}

void lockscreen_events_view_page_del(Evas_Object *events_view, Evas_Object *page)
{
	struct Events_View_Data *data = evas_object_data_get(events_view, DATA_KEY);

	elm_object_scroll_lock_x_set(page, 0);
	elm_box_unpack(data->box, page);
	evas_object_del(page);
}

Eina_Bool lockscreen_events_view_page_panel_visible_get(Evas_Object *page)
{
	struct Event_Page_Data *data = evas_object_data_get(page, DATA_KEY);
	return data->panel_shown;
}
