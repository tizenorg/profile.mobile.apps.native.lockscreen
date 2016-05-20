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

#ifndef _LOCKSCREEN_SIM_LOCK_H_
#define _LOCKSCREEN_SIM_LOCK_H_

typedef enum {
	SIM_LOCK_PIN_TYPE_NONE,
	SIM_LOCK_PIN_TYPE_PIN,
	SIM_LOCK_PIN_TYPE_PUK,
	SIM_LOCK_PIN_TYPE_CARD_BLOCKED,
} pin_type_e;

int lockscreen_sim_lock_pin_required(int *locked_num, pin_type_e *type);

int lockscreen_sim_lock_init(void);

void lockscreen_sim_lock_shutdown(void);

void lockscreen_sim_lock_unlock(int card_no, const char *pass);

int lockscreen_sim_lock_get_attempts_left(void);

int lockscreen_sim_lock_get_pin_type(void);

#endif
