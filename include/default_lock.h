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

#ifndef __DEFAULT_LOCK_H__
#define __DEFAULT_LOCK_H__

typedef enum {
	LOCK_EXIT_STATE_NORMAL = 0,
	LOCK_EXIT_STATE_EXIT = 1,
	LOCK_EXIT_STATE_CAMERA = 2,
	LOCK_EXIT_STATE_MAX,
} lock_exit_state_e;

Evas_Object *lock_default_conformant_get(void);
Evas_Object *lock_default_lock_layout_get(void);
Evas_Object *lock_default_swipe_layout_get(void);

lock_error_e lock_default_lock_init(void);
void lock_default_lock_fini(void);

#endif
