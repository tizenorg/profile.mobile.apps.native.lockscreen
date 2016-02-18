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

#ifndef __LOCKSCREEN_H__
#define __LOCKSCREEN_H__

#include <Elementary.h>
#include <Evas.h>
#include <stdbool.h>

#define EDJE_DIR "/usr/apps/org.tizen.lockscreen/res/edje/"
#define IMAGE_DIR "/usr/apps/org.tizen.lockscreen/res/images/"

#define LOCK_EDJE_FILE EDJE_DIR"lockscreen.edj"

#ifdef TIZEN_BUILD_EMULATOR
#define LOCK_DEFAULT_BG_PATH "/opt/share/settings/Wallpapers/Default.jpg"
#else
#define LOCK_DEFAULT_BG_PATH "/opt/share/settings/Wallpapers/Lock_default.png"
#endif

#define _EDJ(x) elm_layout_edje_get(x)
#define _X(x) ELM_SCALE_SIZE(x)

#define _NOT_LOCALIZED(str) (str)

#define BUF_SIZE_64 64
#define BUF_SIZE_512 512
#define BUF_SIZE_1024 1024

typedef enum {
	LOCK_TYPE_NONE = 0,
	LOCK_TYPE_SWIPE,
	LOCK_TYPE_MOTION,
	LOCK_TYPE_FINGERPRINT,
	LOCK_TYPE_FACE_AND_VOICE,
	LOCK_TYPE_SIMPLE_PASSWORD,
	LOCK_TYPE_PASSWORD,
	LOCK_TYPE_AUTO_LOCK,
	LOCK_TYPE_OTHER,
	LOCK_TYPE_MAX
} lock_type_e;

typedef enum {
	LOCK_ERROR_OK = 0,
	LOCK_ERROR_FAIL = -1,
	LOCK_ERROR_INVALID_PARAMETER = -2,
	LOCK_ERROR_NO_DATA = -3,
	LOCK_ERROR_MAX,
} lock_error_e;

void lockscreen_exit(void);
int lockscreen_setting_lock_type_get(void);
Ecore_Timer *lockscreen_lcd_off_timer_get(void);

void lockscreen_lcd_off_timer_set(void);
void lockscreen_lcd_off_timer_reset(void);
void lockscreen_lcd_off_timer_unset(void);
void lockscreen_lcd_off_count_raise(void);
void lockscreen_lcd_off_count_reset(void);

void lockscreen_feedback_tap_play(void);


#endif
