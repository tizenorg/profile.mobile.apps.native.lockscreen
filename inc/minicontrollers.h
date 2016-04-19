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

#ifndef _LOCKSCREEN_MINICONTROLLERS_H_
#define _LOCKSCREEN_MINICONTROLLERS_H_

#include <Elementary.h>

/**
 * @brief Event fired when minicontroller changes.
 */
extern int LOCKSCREEN_EVENT_MINICONTROLLERS_CHANGED;

/**
 * @brief Initialize minicontroller support
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_minicontrollers_init(void);

/**
 * @brief Deinitialize minicontroller support.
 */
void lockscreen_minicontrollers_shutdown(void);

/**
 * @brief Creates minicontroller for name and parent
 */
Evas_Object *lockscreen_minicontrollers_minicontroller_create(const char *name, Evas_Object *parent);

/**
 * @brief Gets all minicontroller names targeted for lockscreen
 *
 * @note list should be free with eina_list_free
 * @note data contains const char *
 * @note list data should not be free
 */
Eina_List *lockscreen_minicontrollers_list_get(void);

/**
 * @brief Stops minicontroller of given name.
 */
bool lockscreen_minicontrollers_minicontroller_stop(const char *name);

#endif

