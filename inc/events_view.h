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

#define WIDGET_ITEM_STYLE "one_icon"
#define WIDGET_ITEM_CONTENT "elm.swallow.icon"

/**
 * @brief Smart signal emitted when close button is clicked.
 */
#define SIGNAL_CLOSE_BUTTON_CLICKED "btn,close,clicked"

/**
 * @brief Creates camera view object.
 * @note parent should be elementary widget.
 */
Evas_Object *lockscreen_events_view_create(Evas_Object *parent);

/**
 * @brief Gets internall genlist object
 * @note should not be del manually
 */
Evas_Object *lockscreen_events_genlist_get(Evas_Object *events_view);

#endif

