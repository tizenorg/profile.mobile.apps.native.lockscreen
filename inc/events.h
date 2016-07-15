/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOCKSCREEN_EVENTS_H_
#define _LOCKSCREEN_EVENTS_H_

#include <Elementary.h>
#include <time.h>
#include <stdbool.h>

/**
 * @addtogroup Models
 * @{
 */

/**
 * @defgroup Events Events
 */

/**
 * @addtogroup Events
 * @{
 */

/**
 * @brief Event fired when lockscreen's event has been added.
 */
extern int LOCKSCREEN_EVENT_EVENT_ADDED;

/**
 * @brief Event fired when lockscreen's event has been removed.
 */
extern int LOCKSCREEN_EVENT_EVENT_REMOVED;

/**
 * @brief Event fired when lockscreen's event has been updated.
 */
extern int LOCKSCREEN_EVENT_EVENT_UPDATED;

/**
 * @brief lockscreen event handle
 */
typedef struct lockscreen_event lockscreen_event_t;

/**
 * @brief Initialize event support
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_events_init(void);

/**
 * @brief Denitialize event support.
 */
void lockscreen_events_shutdown(void);

/**
 * @brief Gets main event icon
 */
const char *lockscreen_event_icon_get(const lockscreen_event_t *event);

/**
 * @brief Get secondary event icon
 */
const char *lockscreen_event_sub_icon_get(const lockscreen_event_t *event);

/**
 * @brief Get event title
 */
const char *lockscreen_event_title_get(const lockscreen_event_t *event);

/**
 * @brief Get event textual content.
 */
const char *lockscreen_event_content_get(const lockscreen_event_t *event);

/**
 * @brief Gets time when event was posted.
 */
time_t lockscreen_event_time_get(const lockscreen_event_t *event);

/**
 * @brief Launch application which posted the event
 *
 * @return true on success launch request.
 * @return false if event do not support launching application or
 * an error occured.
 */
bool lockscreen_event_launch(lockscreen_event_t *event);

/**
 * @brief Gets list of all displayed events.
 *
 * @note list elements are valid until next LOCKSCREEN_EVENT_EVENT_REMOVED event is fired.
 * @note should be free with eina_list_free
 * @note if event 
 */
Eina_List *lockscreen_events_get(void);

/**
 * @brief Inticates if any events for lockscreen are currently posted.
 */
bool lockscreen_events_exists(void);

/**
 * @brief Removes event from system's notification database.
 *
 * @note fires LOCKSCREEN_EVENT_EVENT_REMOVED event.
 *
 * @return true on success, false otherwise
 */
bool lockscreen_event_remove(lockscreen_event_t *event);

/**
 * @brief Removes all events from system's notification database.
 *
 */
void lockscreen_events_remove_all(void);

/**
 * @brief Ref event
 */
void lockscreen_event_ref(lockscreen_event_t *event);

/**
 * @brief Unref event
 */
void lockscreen_event_unref(lockscreen_event_t *event);

/**
 * @brief set user data.
 */
void lockscreen_event_user_data_set(lockscreen_event_t *event, void *user_data);

/**
 * @brief get user data.
 */
void *lockscreen_event_user_data_get(lockscreen_event_t *event);

/**
 * @brief Gets previous (more recent) event.
 */
lockscreen_event_t *lockscreen_event_prev(lockscreen_event_t *event);

/**
 * @brief Gets next (older) event.
 */
lockscreen_event_t *lockscreen_event_next(lockscreen_event_t *event);

/**
 * @brief Returs current event count
 */
int lockscreen_events_count(void);

/**
 * @}
 */

/**
 * @}
 */

#endif
