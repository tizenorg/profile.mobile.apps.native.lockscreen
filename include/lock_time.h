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

#ifndef __LOCK_TIME_H__
#define __LOCK_TIME_H__

lock_error_e lock_time_update(void);
void lock_time_timer_enable_set(int is_enable);

char *lock_time_formatted_noti_time_get(time_t ts);

void lock_time_resume(void);
void lock_time_pause(void);
void lock_time_init(void);
void lock_time_fini(void);

#endif
