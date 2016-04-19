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

static void _lockscreen_events_view_close_all_clicked(void *date, Evas_Object *obj, const char *emission, const char *source)
{
	evas_object_smart_callback_call(obj, SIGNAL_CLOSE_BUTTON_CLICKED, NULL);
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

	Evas_Object *genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_EXPAND);
	elm_scroller_bounce_set(genlist, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_object_tree_focus_allow_set(genlist, EINA_TRUE);
	evas_object_show(genlist);

	/* Defined in elm-theme-tizen.edj */
	elm_layout_theme_set(genlist, "scroller", "base", "effect");
	elm_object_part_content_set(layout, "sw.genlist", genlist);

	/* Load theme extension */
	util_lockscreen_theme_get();

	/* Register close button clicked callback */
	elm_object_signal_callback_add(layout, "btn,close,clicked", "contextual-event", _lockscreen_events_view_close_all_clicked, NULL);

	return layout;
}

Evas_Object *lockscreen_events_genlist_get(Evas_Object *events_view)
{
	return elm_object_part_content_get(events_view, "sw.genlist");
}
