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

#ifndef _LOCKSCREEN_BATTERY_H_
#define _LOCKSCREEN_BATTERY_H_

#include <stdbool.h>

/**
 * @brief Event fired when battery status changes.
 * @note register via ecore_event_handler_add
 * @note can be triggered after lockscreen_battery_init
 */
extern int LOCKSCREEN_EVENT_BATTERY_CHANGED;

/**
 * @brief Initializes battery notification changes.
 * @return: 0 on success, other value on failure.
 */
int lockscreen_battery_init(void);

/**
 * @brief Deinitialize battery notification changes.
 */
void lockscreen_battery_shutdown(void);

/**
 * @brief true is battery is charging.
 */
bool lockscreen_battery_is_charging(void);

/**
 * @brief true is battery charger is connected.
 */
bool lockscreen_battery_is_connected(void);

/**
 * @brief Returns battery level state 0 - 100
 */
int lockscreen_battery_level_get(void);

#endif
