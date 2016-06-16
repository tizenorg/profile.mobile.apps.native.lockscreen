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

#ifndef _LOCKSCREEN_EVENTS_VIEW_H_
#define _LOCKSCREEN_EVENTS_VIEW_H_

#include <Elementary.h>

#define NOTI_ITEM_STYLE "noti"
#define NOTI_ITEM_TEXT "elm.text"
#define NOTI_ITEM_TEXT_SUB "elm.text.sub"
#define NOTI_ITEM_TEXT_TIME "elm.text.time"
#define NOTI_ITEM_ICON "elm.swallow.icon"
#define NOTI_ITEM_ICON_SUB "elm.swallow.sub.icon"

#define WIDGET_ITEM_STYLE "minicontroller"
#define WIDGET_ITEM_CONTENT "elm.swallow.content"

#define NOTI_MORE_ITEM_STYLE "noti_more"

/**
 * @brief The expand gesture has finished
 */
#define SIGNAL_PAGE_EXPAND_GESTURE "expand,gesture"

typedef enum {
	LOCKSCREEN_EVENTS_VIEW_TYPE_NOTIFICATIONS,
	LOCKSCREEN_EVENTS_VIEW_TYPE_MEDIA,
} lockscreen_events_view_type_e;

/**
 * @brief Smart signal emitted when close button is clicked.
 */
#define SIGNAL_PAGE_CANCEL_BUTTON_CLICKED "btn,cancel,clicked"

/**
 * @brief Smart signal emitted when clear button is clicked.
 */
#define SIGNAL_PAGE_CLEAR_BUTTON_CLICKED "btn,clear,clicked"

/**
 * @brief Creates camera view object.
 * @note parent should be elementary widget.
 */
Evas_Object *lockscreen_events_view_create(Evas_Object *parent);

/**
 * @brief Creates new page object and appends it to internal scroller
 *
 * @note Object Should be deleted with lockscreen_events_view_page_del
 */
Evas_Object *lockscreen_events_view_page_append(Evas_Object *events_view);

/**
 * @brief Creates new notification page and prepends it to internal scroller
 *
 * @note Object Should be deleted with lockscreen_events_view_page_del
 */
Evas_Object *lockscreen_events_view_page_prepend(Evas_Object *events_view);

/**
 * @brief Destroys notification page
 */
void lockscreen_events_view_page_del(Evas_Object *events_view, Evas_Object *page);

/**
 * @brief Show/Hides page panel with clear and close buttons
 */
void lockscreen_events_view_page_panel_visible_set(Evas_Object *page, Eina_Bool visible);

/**
 * @brief Check if page panel is visible
 */
Eina_Bool lockscreen_events_view_page_panel_visible_get(Evas_Object *page);

/**
 * @brief Gets internall genlist object
 *
 * @note should not be del manually
 */
Evas_Object *lockscreen_events_view_page_genlist_get(Evas_Object *page);

/**
 * @brief Scroll to given page.
 */
void lockscreen_events_view_page_bring_in(Evas_Object *events_view, Evas_Object *page);

#endif
