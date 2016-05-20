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
 * @brief Smart signal emitted when user has finished typing password.
 */
#define SIGNAL_ACCEPT_BUTTON_CLICKED "password,typed"

/**
 * @brief Smart signal emitted when user has clicked cancel button.
 */
#define SIGNAL_CANCEL_BUTTON_CLICKED "cancel,button,clicked"

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
 * @brief View cancel button text.
 */
#define PART_TEXT_CANCEL "text.cancel"

typedef enum {
	LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN,
	LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD,
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

#endif
