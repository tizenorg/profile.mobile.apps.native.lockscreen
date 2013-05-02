/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#ifndef __LOCKSCREEN_H__
#define __LOCKSCREEN_H__

#include <Elementary.h>
#include "log.h"

#if !defined(PACKAGE)
#define PACKAGE "lockscreen"
#endif

#if !defined(EDJDIR)
#define EDJDIR "/usr/apps/org.tizen."PACKAGE"/res/edje/lockscreen"
#endif

#define EDJEFILE EDJEDIR"/"PACKAGE".edj"

#define _S(str)	dgettext("sys_string", str)
#define _L(str) dgettext("lockscreen", str)
#define _NOT_LOCALIZED(str) (str)
#define _(str) gettext(str)

#define _EDJ(x) elm_layout_edje_get(x)
#define _X(x) (x*elm_config_scale_get())

#define FLICK_LINE 898
#define SLIDER_RATIO_H 0.089
#define SLIDER_RATIO_Y 0.872

enum {
	POS_DOWN = 0,
	POS_MOVE,
	POS_UP,
	POS_REC,//to record the pre position
	STEP,
	POS_MAX
};

struct appdata {
	Evas_Object *win;
	Evas_Object *ly_main;//top_layout

	Evas_Coord win_w;
	Evas_Coord win_h;
	Evas_Coord posx[POS_MAX];
	Evas_Coord posy[POS_MAX];

	Evas_Object *info;//time
	Evas_Object *noti;
	Evas_Object *ly_simple_password;
	Evas_Object *ly_complex_password;
	Evas_Object *c_password_entry;
	Evas_Object *event_bg;
	Evas_Object *slider;

	Evas_Coord_Rectangle slider_rel1;
	Evas_Coord_Rectangle slider_rel2;

	Eina_Bool bFlick;//flick to launch shorcuts
	Eina_Bool bDrag;//for drag lock

	int heynoti_fd;

	void *h_password_policy;
	Ecore_Timer *password_timer;
	int block_seconds;
	Eina_Bool is_disabled;

	struct ui_gadget *emgc_ug;
};

#endif
