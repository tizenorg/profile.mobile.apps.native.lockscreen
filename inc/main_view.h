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

#ifndef _LOCKSCREEN_MAIN_VIEW_H_
#define _LOCKSCREEN_MAIN_VIEW_H_

#include <Elementary.h>
#include <stdbool.h>

typedef enum {
	LOCKSCREEN_BACKGROUND_TYPE_DEFAULT,
	LOCKSCREEN_BACKGROUND_TYPE_ALBUM_ART,
} lockscreen_main_view_background_type;

/**
 * @brief Smart signal emitted when swipe gesture gesture has been performed.
 */
#define SIGNAL_SWIPE_GESTURE_FINISHED "swipe,gesture,finished"

/**
 * @brief Smart signal emitted unlock animation finished.
 */
#define SIGNAL_UNLOCK_ANIMATION_FINISHED "unlock,anim,finished"

/**
 * @brief Accessible via lockscreen_main_view_part_content_get/set/unset
 */
#define PART_SHORTCUT "sw.shortcut"

/**
 * @brief Accessible via lockscreen_main_view_part_content_get/set/unset
 */
#define PART_CALL "sw.call"
/**
 * @brief Accessible via lockscreen_main_view_part_content_get/set/unset
 */
#define PART_EVENTS "sw.contextual_event"

/**
 * @brief Part accessible via lockscreen_main_view_part_content_get/set/unset
 */
#define PART_PASSWORD "sw.password_layout"

/**
 * @brief Part accessible via lockscreen_main_view_part_content_get/set/unset
 */
#define PART_SIMLOCK "sw.simlock_layout"
/**
 * @brief Creates main view object
 */
Evas_Object *lockscreen_main_view_create(Evas_Object *parent);

/**
 * @brief Set sub view of main view object
 */
void lockscreen_main_view_part_content_set(Evas_Object *view, const char *part, Evas_Object *content);

/**
 * @brief Get sub view of main view object
 */
Evas_Object *lockscreen_main_view_part_content_get(Evas_Object *view, const char *part);

/**
 * @brief Unsets sub view of main view object
 */
Evas_Object *lockscreen_main_view_part_content_unset(Evas_Object *view, const char *part);

/**
 * @brief Set main view background image
 */
bool lockscreen_main_view_background_set(Evas_Object *view, lockscreen_main_view_background_type type, const char *file);

/**
 * @brief Sets battery status displayed text
 */
void lockscreen_main_view_battery_status_text_set(Evas_Object *view, const char *battery);

/**
 * @brief Plays unlock animation and runs @animation_end_cb on end.
 * @note animation_end_cb will be called only once.
 */
void lockscreen_main_view_unlock(Evas_Object *obj);

/**
 * @brief Sets main view time information
 *
 * @note time information will be displayed in format suitable for given locale,
 * timezone and 24h format option.
 */
void lockscreen_main_view_time_set(Evas_Object *view, const char *locale, const char *timezone, bool use24hformat, time_t time);

/**
 * @brief Sets sim status textual information.
 */
void lockscreen_main_view_sim_status_text_set(Evas_Object *view, const char *text);

/**
 * @brief Sets fullscreen status of contextual view
 */
void lockscreen_main_view_contextual_view_fullscreen_set(Evas_Object *view, bool fullscreen);

/**
 * @brief Sets top and bottom panel state
 *
 * @param top true if upper part should be unlocked (moved out of screen), false otherwise
 * @param bottom true if upper part should be unlocked (moved out of screen), false otherwise
 */
void lockscreen_main_view_unlock_state_set(Evas_Object *view, bool top, bool bottom);

#endif
