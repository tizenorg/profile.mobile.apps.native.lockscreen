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

#ifndef _LOCKSCREEN_MAIN_CTRL_H_
#define _LOCKSCREEN_MAIN_CTRL_H_

/**
 * @addtogroup Controllers
 * @{
 */

/**
 * @defgroup Main Main
 */

/**
 * @addtogroup Main
 * @{
 */

/**
 * @brief Initializes main controller.
 *
 * Application's root controller. Manages all other controllers.
 * Creates main_view.
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_main_ctrl_init();

/**
 * @brief Shutdown main controller.
 */
void lockscreen_main_ctrl_shutdown();

/**
 * @brief Application is being paused (according to Tizen app lifecycle)
 */
void lockscreen_main_ctrl_app_paused(void);

/**
 * @}
 */

/**
 * @}
 */
#endif
