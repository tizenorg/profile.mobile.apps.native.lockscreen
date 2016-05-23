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

#include <Elementary.h>

#define SCROLLER_QUICKFIX

static void
_lockscreen_events_view_scroller_thaw(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	DBG("Thaw scroller");
	Evas_Object *scroller = elm_object_part_content_get(data, "sw.scroller");
	elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
}

static void
_lockscreen_events_view_scroller_freeze2(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	Evas_Object *scroller = elm_object_part_content_get(data, "sw.scroller");
	if (!evas_object_data_get(scroller, "_scroll_lock")) {
		DBG("Freeze scroller");
		elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL | ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	}
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
_lockscreen_events_view_scroller_del(void *data, Evas *e, Evas_Object *eo, void *event_info)
{
	evas_object_del(data);
}

static void _lockscreen_events_view_scroller_hit_rectangle_add(Evas_Object *obj, Evas_Object *scroller)
{
	Evas_Object *track = evas_object_rectangle_add(evas_object_evas_get(obj));
	evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _lockscreen_events_view_scroller_hit_rectangle_resize, track);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _lockscreen_events_view_scroller_hit_rectangle_resize, track);
	evas_object_color_set(track, 0, 0, 0, 0);
	evas_object_event_callback_add(track, EVAS_CALLBACK_MOUSE_DOWN, _lockscreen_events_view_scroller_thaw, scroller);
	evas_object_event_callback_add(track, EVAS_CALLBACK_MOUSE_UP, _lockscreen_events_view_scroller_freeze2, scroller);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _lockscreen_events_view_scroller_del, track);
	evas_object_repeat_events_set(track, EINA_TRUE);
	evas_object_show(track);
}

static void
_lockscreen_events_view_scroller_freeze(void *data, Evas_Object *eo, void *event_info)
{
	DBG("Freeze scroller");
	Evas_Object *scroller = data;
	elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL | ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	evas_object_data_set(data, "_scroll_lock", NULL);
}

static void
_lockscreen_events_view_scroller_start(void *data, Evas_Object *eo, void *event_info)
{
	DBG("scroller drag start");
	evas_object_data_set(data, "_scroll_lock", (void*)1);
}

static Evas_Object*
_lockscreen_events_view_index_create(Evas_Object *parent)
{
	Evas_Object *ret = elm_index_add(parent);

	util_lockscreen_theme_get();

	evas_object_size_hint_weight_set(ret, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ret, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_style_set(ret, "pagecontrol");

	elm_index_horizontal_set(ret, EINA_TRUE);
	elm_index_autohide_disabled_set(ret, EINA_TRUE);
	evas_object_show(ret);

	elm_index_level_go(ret, 0);

	return ret;
}

Evas_Object *lockscreen_events_view_create(Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);
	if (!elm_layout_file_set(layout, util_get_res_file_path(LOCK_EDJE_FILE), "contextual-event")) {
		FAT("elm_layout_file_set failed for contextual-event");
		evas_object_del(layout);
		return NULL;
	}
	evas_object_show(layout);

	/* Load theme extension */
	util_lockscreen_theme_get();

	Evas_Object *scroller = elm_scroller_add(layout);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL | ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_page_size_set(scroller, 720, 0);
	elm_scroller_page_scroll_limit_set(scroller, 1, 1);
	evas_object_smart_callback_add(scroller, "scroll,anim,stop", _lockscreen_events_view_scroller_freeze, scroller);
	evas_object_smart_callback_add(scroller, "scroll,drag,start", _lockscreen_events_view_scroller_start, scroller);
	elm_layout_theme_set(scroller, "scroller", "base", "pass_effect");
	evas_object_show(scroller);

	Evas_Object *box = elm_box_add(scroller);
	elm_box_horizontal_set(box, EINA_TRUE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_align_set(box, 0, 0);
	elm_box_homogeneous_set(box, EINA_TRUE);
	evas_object_show(box);

	/* Defined in elm-theme-tizen.edj */
	elm_object_part_content_set(layout, "sw.scroller", scroller);
	elm_object_content_set(scroller, box);

	elm_scroller_page_show(scroller, 0, 0);

	Evas_Object *index = _lockscreen_events_view_index_create(layout);
	elm_object_part_content_set(layout, "sw.index", index);

	return layout;
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

static Evas_Object*
_lockscreen_events_view_page_create(Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);
	if (!elm_layout_file_set(layout, util_get_res_file_path(LOCK_EDJE_FILE), "context-page")) {
		FAT("elm_layout_file_set failed");
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *genlist = elm_genlist_add(layout);
	evas_object_size_hint_align_set(genlist, 0.5, 0);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_bounce_set(genlist, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_tree_focus_allow_set(genlist, EINA_TRUE);
	elm_layout_theme_set(genlist, "scroller", "base", "pass_effect");

	/* Load theme extension */
	util_lockscreen_theme_get();

	Evas_Object *btn = elm_button_add(layout);
	evas_object_show(btn);
	elm_layout_theme_set(btn, "button", "panel", "default");
	elm_object_text_set(btn, "CLEAR ALL");
	evas_object_smart_callback_add(btn, "clicked", _lockscreen_events_view_page_clear_button_clicked, layout);
	elm_object_part_content_set(layout, "sw.left", btn);

	btn = elm_button_add(layout);
	evas_object_show(btn);
	elm_layout_theme_set(btn, "button", "panel", "default");
	elm_object_text_set(btn, "CANCEL");
	evas_object_smart_callback_add(btn, "clicked", _lockscreen_events_view_page_cancel_button_clicked, layout);
	elm_object_part_content_set(layout, "sw.right", btn);

	evas_object_show(genlist);
	evas_object_show(layout);

	elm_object_part_content_set(layout, "sw.content", genlist);

	return layout;
}

void lockscreen_events_view_page_panel_visible_set(Evas_Object *page, Eina_Bool visible)
{
	if (visible) {
		elm_object_scroll_lock_x_set(page, 1);
		elm_object_signal_emit(page, "panel,show", "lockscreen");
	} else {
		elm_object_scroll_lock_x_set(page, 0);
		elm_object_signal_emit(page, "panel,hide", "lockscreen");
	}
}

void lockscreen_events_view_page_show(Evas_Object *events_view, Evas_Object *page)
{
	//FIXME calculate and move scroller
}

void lockscreen_events_view_page_bring_in(Evas_Object *events_view, Evas_Object *page)
{
	//FIXME calculate and move scroller
}

static void
__del_idx(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("Delte index item!!!");
	Evas_Object *index = elm_object_item_widget_get(data);
	elm_object_item_del(data);
	elm_index_level_go(index, 0);
}

static void
__page_changed(void *data, Evas_Object *obj, void *event_info)
{
	int h;
	elm_scroller_current_page_get(obj, &h, NULL);

	Evas_Object *box = elm_object_content_get(obj);
	Eina_List *childs = elm_box_children_get(box);

	Evas_Object *page = eina_list_nth(childs, h);

	Elm_Object_Item *idx = evas_object_data_get(page, "__idx");
	elm_index_item_selected_set(idx, EINA_TRUE);
	eina_list_free(childs);

	DBG("H page: %d", h);
}

static void
_lockscreen_events_view_page_append(Evas_Object *events_view, Evas_Object *page)
{
#ifdef SCROLLER_QUICKFIX
	DBG("Using SCROLLE QUICKFIX");
	// FIXME - Quickfix for elm_scroller behaviour change
	Evas_Object *scroller_old = elm_object_part_content_unset(events_view, "sw.scroller");
	if (!scroller_old) {
		FAT("elm_object_part_content_get failed");
	}

	Evas_Object *box = elm_object_content_unset(scroller_old);
	if (!box) {
		FAT("elm_object_content_get failed");
		return;
	}

	elm_box_pack_end(box, page);

	Evas_Object *scroller = elm_scroller_add(events_view);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_movement_block_set(scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL | ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_page_size_set(scroller, 720, 0);
	elm_scroller_page_scroll_limit_set(scroller, 1, 1);
	evas_object_smart_callback_add(scroller, "scroll,anim,stop", _lockscreen_events_view_scroller_freeze, scroller);
	evas_object_smart_callback_add(scroller, "scroll,drag,start", _lockscreen_events_view_scroller_start, scroller);
	elm_layout_theme_set(scroller, "scroller", "base", "pass_effect");

	elm_object_content_set(scroller, box);
	elm_object_part_content_set(events_view, "sw.scroller", scroller);
	elm_scroller_page_show(scroller, 0, 0);
	evas_object_show(scroller);

	//evas_object_del(scroller_old);
#else
	Evas_Object *scroller = elm_object_part_content_get(events_view, "sw.scroller");
	if (!scroller) {
		FAT("elm_object_part_content_get failed");
		return;
	}

	Evas_Object *box = elm_object_content_get(scroller);
	if (!box) {
		FAT("elm_object_content_get failed");
		return;
	}

	elm_box_pack_end(box, page);
	elm_box_recalculate(box);
#endif

	Evas_Object *swipe_area = (Evas_Object*)edje_object_part_object_get(elm_layout_edje_get(page), "swipe.area");
	_lockscreen_events_view_scroller_hit_rectangle_add(swipe_area, events_view);


	Evas_Object *index = elm_object_part_content_get(events_view, "sw.index");
	DBG("Append index item");
	Elm_Object_Item *idx = elm_index_item_append(index, NULL, NULL, page);
	elm_index_level_go(index, 0);
	evas_object_data_set(page, "__idx", idx);
	evas_object_event_callback_add(page, EVAS_CALLBACK_DEL, __del_idx, idx);

	evas_object_smart_callback_del(scroller, "scroll,page,changed", __page_changed);
	evas_object_smart_callback_add(scroller, "scroll,page,changed", __page_changed, NULL);
}

Evas_Object *lockscreen_events_view_page_add(Evas_Object *events_view)
{
	Evas_Object *page = _lockscreen_events_view_page_create(events_view);
	if (!page) {
		FAT("_lockscreen_events_view_page_create failed");
		return NULL;
	}
	_lockscreen_events_view_page_append(events_view, page);
	return page;
}

Evas_Object *lockscreen_events_view_page_genlist_get(Evas_Object *page)
{
	return elm_object_part_content_get(page, "sw.content");
}

void lockscreen_events_view_page_del(Evas_Object *events_view, Evas_Object *page)
{
	Evas_Object *scroller = elm_object_part_content_get(events_view, "sw.scroller");
	if (!scroller) {
		FAT("elm_object_part_content_get failed");
	}

	Evas_Object *box = elm_object_content_get(scroller);
	if (!box) {
		FAT("elm_object_content_get failed");
		return;
	}

	elm_box_unpack(box, page);
	elm_box_recalculate(box);
	evas_object_del(page);
}
