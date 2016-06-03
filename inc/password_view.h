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

#ifndef _LOCKSCREEN_PASSWORD_VIEW_H
#define _LOCKSCREEN_PASSWORD_VIEW_H

#include <Elementary.h>

/**
 * @addtogroup Views
 * @{
 */

/**
 * @defgroup Password Password
 */

/**
 * @addtogroup Password
 * @{
 */

/**
 * @brief Smart signal emitted when user has finished typing password.
 */
#define SIGNAL_ACCEPT_BUTTON_CLICKED "password,typed"

/**
 * @brief Smart signal emitted when user is typing password.
 */
#define SIGNAL_PASSWORD_TYPING "password,typing"

/**
 * @brief Smart signal emitted when user has clicked cancel button.
 */
#define SIGNAL_CANCEL_BUTTON_CLICKED "cancel,button,clicked"

/**
 * @brief Smart signal emitted when user has clicked return to call button.
 */
#define SIGNAL_RETURN_TO_CALL_BUTTON_CLICKED "return_to_call,button,clicked"

/**
 * @brief Smart signal sent to show default sim card icon.
 */
#define SIGNAL_SIM_ICON_SHOW "layout,pin_icon,show"

/**
 * @brief Smart signal sent to show sim1 card icon.
 */
#define SIGNAL_SIM_ICON_SHOW_SIM1 "layout,pin_icon,show,1"

/**
 * @brief Smart signal sent to show sim2 card icon.
 */
#define SIGNAL_SIM_ICON_SHOW_SIM2 "layout,pin_icon,show,2"

/**
 * @brief View title text part.
 */
#define PART_TEXT_TITLE "text.title"

/**
 * @brief View sub title text part.
 */
#define PART_TEXT_SUBTITLE "text.subtitle"

/**
 * @brief Event swallow part
 */
#define PART_CONTENT_EVENT "sw.event"

/**
 * @brief Type of password view.
 */
typedef enum {
	LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN,
	LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD,
	LOCKSCREEN_PASSWORD_VIEW_TYPE_SWIPE,
} lockscreen_password_view_type;

/**
 * @brief Creates password view object.
 * @note parent should be elementary widget.
 */
Evas_Object *lockscreen_password_view_create(lockscreen_password_view_type type, Evas_Object *parent);

/**
 * @brief Clears currently entered password.
 */
void lockscreen_password_view_clear(Evas_Object *view);

/**
 * @brief Sets an password length. When user type required pin length the SIGNAL_ACCEPT_BUTTON_CLICKED will be triggered.
 *
 * @note default value is 4
 */
void lockscreen_password_view_pin_password_length_set(Evas_Object *view, int length);

/**
 * @brief Hides cancel button
 */
void lockscreen_password_view_btn_cancel_hide(Evas_Object *view);
/**
 * @brief Shows virual keybard
 */
void lockscreen_password_view_keyboard_show(Evas_Object *view);

/**
 * @brief Displays event minature on view
 */
void lockscreen_password_view_event_set(Evas_Object *view, Evas_Object *event);

/**
 * @brief Hides return to call button
 */
void lockscreen_password_view_btn_return_to_call_hide(Evas_Object *view);

/**
 * @brief Shows return to call button
 */
void lockscreen_password_view_btn_return_to_call_show(Evas_Object *view);

/**
 * @}
 */

/**
 * @}
 */

#endif
