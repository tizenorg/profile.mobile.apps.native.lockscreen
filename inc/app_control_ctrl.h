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

#ifndef _LOCKSCREEN_APP_CONTROL_CTRL_H_
#define _LOCKSCREEN_APP_CONTROL_CTRL_H_

#include <app_control.h>

/**
 * @brief Initializes app_control controller.
 *
 * Battery controller manages app_control request.
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_app_control_ctrl_init(void);

/**
 * @brief Handle app_control request
 */
void lockscreen_app_control_ctrl_handle(app_control_h request);

/**
 * @brief Deinitializes app_control controller.
 */
void lockscreen_app_control_ctrl_fini(void);

#endif

