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

#ifndef __MISSED_EVENT_H__
#define __MISSED_EVENT_H__

#define LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY "__missed_event_type__"
#define LOCK_CONTEXTUAL_MISSED_EVENT_IS_SELECTED_KEY "__missed_event_is_selected_type__"
#define LOCK_CONTEXTUAL_MISSED_EVENT_BUNDLE_KEY "__missed_event_bundle__"

#define APP_NAME_CALL "org.tizen.phone"
#define APP_NAME_CONTACTS "org.tizen.contacts"
#define APP_NAME_MSG "org.tizen.message"

#define ICON_PATH_MISSED_EVENT_MSG "/usr/apps/org.tizen.quickpanel/shared/res/noti_icons/Contact/noti_contact_default.png"
#define ICON_PATH_MISSED_EVENT_CALL ICON_PATH_MISSED_EVENT_MSG

typedef enum {
	LOCK_MISSED_EVENT_TYPE_NONE = 0,
	LOCK_MISSED_EVENT_TYPE_CALL = 1,
	LOCK_MISSED_EVENT_TYPE_MSG = 2,
	LOCK_MISSED_EVENT_TYPE_MAX,
} missed_event_type_e;

Evas_Object *lock_missed_event_selected_item_get(void);

void lock_missed_event_item_launch(void);
void lock_missed_event_item_disabled_set(Eina_Bool disabled);
void lock_missed_event_item_selected_unset(void);
void lock_missed_event_item_destroy(Evas_Object *item);

lock_error_e lock_missed_event_noti_cb_register(void);
void lock_missed_event_noti_cb_unregister(void);

#endif
