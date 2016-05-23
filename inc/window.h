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

#ifndef _LOCKSCREEN_WINDOW_H_
#define _LOCKSCREEN_WINDOW_H_

#include <stdbool.h>

/**
 * @brief Smart signal emitted when users start touching window.
 */
#define SIGNAL_TOUCH_STARTED "win,touch,started"

/**
 * @brief Smart signal emitted when users ended touching window.
 */
#define SIGNAL_TOUCH_ENDED "win,touch,ended"

/**
 * @brief Gets main window width
 */
int lock_window_width_get(void);

/**
 * @brief Gets main window height
 */
int lock_window_height_get(void);

/**
 * @brief Creates default lockscreen fullscreen window
 */
Evas_Object *lockscreen_window_create(void);

/**
 * @brief Sets lockscreen content view
 */
void lockscreen_window_content_set(Evas_Object *content);

/**
 * @brief Block/Unblock indicator access on main window
 */
void lockscreen_window_quickpanel_block_set(Eina_Bool block);

#endif
