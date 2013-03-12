/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "lockscreen.h"

void _set_win_property(Evas_Object *win);

Evas_Object *_add_layout(Evas_Object *parent, const char *file, const char *group);
Evas_Object *_make_top_layout(struct appdata *ad);
Evas_Object *_get_bg_image(Evas_Object *parent);
void lockscreen_info_show(struct appdata *ad);
void lockscreen_info_hide(struct appdata *ad);
void launch_emgcall(struct appdata *ad);

int _app_create(struct appdata *ad);
int _app_reset(struct appdata *ad);
int _app_terminate(struct appdata *ad);
#endif
