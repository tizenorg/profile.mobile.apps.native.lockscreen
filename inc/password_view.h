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
#define SIGNAL_CLOSE_BUTTON_CLICKED "password,typed"

typedef enum {
	LOCKSCREEN_PASSWORD_VIEW_TYPE_PIN,
	LOCKSCREEN_PASSWORD_VIEW_TYPE_PASSWORD,
} lockscreen_password_view_type;

/**
 * @brief Creates password view object.
 * @note parent should be elementary widget.
 */
Evas_Object *lockscreen_password_view_create(lockscreen_password_view_type type, Evas_Object *parent);

#endif
