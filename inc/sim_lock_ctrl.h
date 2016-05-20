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

#ifndef _LOCKSCREEN_SIM_LOCK_CTRL_H_
#define _LOCKSCREEN_SIM_LOCK_CTRL_H_

#include <Elementary.h>

/**
 * @brief Intializes sim lock controller
 *
 * Sim lock controller is resposible for creating and updating main_view
 * information when sim lock status changes.
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_sim_lock_ctrl_init(Evas_Object *main_view);

/**
 * @brief Shutdonws sim lock controller
 */
void lockscreen_sim_lock_ctrl_shutdown(void);

#endif
