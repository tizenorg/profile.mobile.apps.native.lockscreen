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

#include <Elementary.h>

/**
 * @brief Initializes call controller
 *
 * Call controller is resnponsible for updating call icon on main
 * lockscreen view when call status changes.
 *
 * @return: 0 on success, other value on failure
 */
int lockscreen_call_ctrl_init(Evas_Object *view);

/**
 * @brief Shutdonws call controller
 */
void lockscreen_call_ctrl_shutdown(void);