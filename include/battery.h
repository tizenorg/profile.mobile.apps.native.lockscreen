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

#ifndef __BATTERY_H__
#define __BATTERY_H__

bool lock_battery_is_charging_get(void);
bool lock_battery_is_connected_get(void);

lock_error_e lock_battery_update(void);
lock_error_e lock_battery_show(void);
lock_error_e lock_battery_hide(void);

lock_error_e lock_battery_init(void);
void lock_battery_fini(void);

#endif
