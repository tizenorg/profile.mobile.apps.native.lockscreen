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

#ifndef __BACKGROUND_VIEW_H__
#define __BACKGROUND_VIEW_H__

typedef enum {
	LOCK_BG_DEFAULT = 0,
	LOCK_BG_ALBUM_ART = 1,
	LOCK_BG_MAX,
} lock_bg_type_e;

Evas_Object *lock_background_view_bg_get(void);

lock_error_e lock_background_view_image_set(lock_bg_type_e type, char *file);
Evas_Object *lock_background_view_bg_create(Evas_Object *win);
void lock_background_view_bg_del(void);

#endif
