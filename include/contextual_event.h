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

#ifndef __CONTEXTUAL_EVENT_H__
#define __CONTEXTUAL_EVENT_H__

#include <notification.h>

#include "music_player.h"
#include "missed_event.h"

#define LOCK_CONTEXTUAL_PAGE_TYPE_KEY "__contextual_page_type__"

typedef enum {
	LOCK_CONTEXTUAL_TYPE_NONE = 0,
	LOCK_CONTEXTUAL_TYPE_MISSED_EVENT = 1,
	LOCK_CONTEXTUAL_TYPE_MUSIC = 2,
	LOCK_CONTEXTUAL_TYPE_MAX,
} contextual_type_e;

Evas_Object *lock_contextual_event_layout_get(void);
Evas_Object *lock_contextual_event_scroller_get(void);
Evas_Object *lock_contextual_event_scroller_box_get(void);
Eina_Bool lock_contextual_event_layout_visible_get(void);

int lock_contextual_event_page_count_get(void);
int lock_contextual_event_current_page_get(void);
Evas_Object *lock_contextual_event_page_get(contextual_type_e type);
Evas_Object *lock_contextual_event_page_create(Evas_Object *parent, contextual_type_e type);

lock_error_e lock_contextual_event_music_add(music_state_e state, const char *name, int width, int height);
void lock_contextual_event_music_del(void);

Evas_Object *lock_contextual_event_missed_event_item_find(missed_event_type_e type);
lock_error_e lock_contextual_event_missed_event_item_add(Evas_Object *item, missed_event_type_e type);
void lock_contextual_event_missed_event_item_del(missed_event_type_e type);
void lock_contextual_event_missed_event_del(void);

Evas_Object *lock_contextual_event_layout_create(Evas_Object *parent);
lock_error_e lock_contextual_event_layout_del(void);

#endif
