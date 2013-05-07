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

#include <dlog.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "lockscreen.h"
#include "util.h"
#include "simple-password.h"
#include "complex-password.h"

static double _get_move(Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2)
{
	return ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

static void _flick_event_process(struct appdata *ad)
{
	int status = 0;
	int locktype = 0;

	vconf_get_int(VCONFKEY_SETAPPL_SCREEN_LOCK_TYPE_INT, &locktype);
	if(locktype == SETTING_SCREEN_LOCK_TYPE_SIMPLE_PASSWORD){
		simple_password_layout_create(ad);
	} else if(locktype == SETTING_SCREEN_LOCK_TYPE_PASSWORD){
		complex_password_layout_create(ad);
	}
}

void _mouse_down_cb_s(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	LOGD("[ == %s == ]", __func__);
	struct appdata *ad = data;
	if (ad == NULL)
		return;

	evas_pointer_output_xy_get(evas_object_evas_get(obj), &ad->posx[POS_DOWN], &ad->posy[POS_DOWN]);
	if(ad->posy[POS_DOWN] < FLICK_LINE) {
		return;
	}else {
		ad->bFlick = 1;
		ad->posx[POS_REC] = ad->posx[POS_DOWN];
		ad->posy[POS_REC] = ad->posy[POS_DOWN];
	}
}

void _mouse_move_cb_s(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = data;
	if (ad == NULL)
		return;

	if(ad->bFlick == 1) {
		evas_pointer_output_xy_get(evas_object_evas_get(obj), &ad->posx[POS_MOVE], &ad->posy[POS_MOVE]);
		if(ad->posy[POS_MOVE] < ad->posy[POS_REC]) {
			double d = _get_move(ad->posx[POS_DOWN], ad->posy[POS_DOWN], ad->posx[POS_MOVE], ad->posy[POS_MOVE]);
			if(d > (_X(84100)*1.3)) {
				ad->bFlick = 0;
				LOCK_SCREEN_TRACE_DBG("====move info up====");
				_flick_event_process(ad);
			}
			ad->posx[POS_REC] = ad->posx[POS_MOVE];
			ad->posy[POS_REC] = ad->posy[POS_MOVE];
		}
	}
}

void _mouse_up_cb_s(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = data;
	if (ad == NULL)
		return;

	if(ad->bFlick == 1) {
		ad->bFlick = 0;
		evas_pointer_output_xy_get(evas_object_evas_get(obj), &ad->posx[POS_UP], &ad->posy[POS_UP]);
		if(ad->posy[POS_UP] < ad->posy[POS_REC]) {
			double d = _get_move(ad->posx[POS_DOWN], ad->posy[POS_DOWN], ad->posx[POS_UP], ad->posy[POS_UP]);
			if(d > (_X(84100)*1.3)) {
				LOCK_SCREEN_TRACE_DBG("====move info up====");
				_flick_event_process(ad);
			}
		}
	}
}

void _slider_down_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	Evas_Event_Mouse_Down *ev = event_info;
	struct appdata *ad = (struct appdata *) data;

	if (ad == NULL) {
		LOCK_SCREEN_TRACE_DBG("appdata is NULL");
		return;
	}

	if (ev == NULL) {
		LOCK_SCREEN_TRACE_DBG("event_info is NULL");
		return;
	}

	if (ad->slider_rel1.x <= ev->canvas.x && ad->slider_rel1.y <= ev->canvas.y
			&& (ad->slider_rel1.x + ad->slider_rel1.w) >= ev->canvas.x
			&& (ad->slider_rel1.y + ad->slider_rel1.h) >= ev->canvas.y) {
		ad->bDrag= 1;

		edje_object_signal_emit(_EDJ(ad->slider), "press",
				"lock.image.l");
		edje_object_signal_emit(_EDJ(ad->slider), "press02",
				"lock.image.r");
	} else {
		LOCK_SCREEN_TRACE_DBG("sliding is canceled");
		ad->bDrag= 0;
	}
}

void _slider_move_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	Evas_Event_Mouse_Move *ev = event_info;
	struct appdata *ad = (struct appdata *) data;
	int step_width = 0;
	int step_number = 0;
	int alpha = 0;

	if (ad == NULL) {
		LOCK_SCREEN_TRACE_DBG("appdata is NULL");
		return;
	}

	if (ev == NULL) {
		LOCK_SCREEN_TRACE_DBG("event_info is NULL");
		return;
	}

	if (ad->bDrag == 0)
	{
		return;
	}

	if (ad->slider_rel1.x <= ev->cur.canvas.x
			&& ad->slider_rel1.y <= ev->cur.canvas.y
			&& (ad->slider_rel2.x + ad->slider_rel2.w) >= ev->cur.canvas.x
			&& (ad->slider_rel2.y + ad->slider_rel2.h) >= ev->cur.canvas.y) {
		ad->bDrag = 1;

		step_width = (ad->slider_rel2.x + ad->slider_rel2.w - ad->slider_rel1.x)
				/ 14;
		step_number = (ev->cur.canvas.x - ad->slider_rel1.x) / step_width;

		alpha = 255 - (2.55 * step_number * 3);

		if (step_number < 1) {
			edje_object_signal_emit(_EDJ(ad->slider), "press02",
					"lock.image.r");
		} else if (step_number < 2) {
			edje_object_signal_emit(_EDJ(ad->slider), "press03",
					"lock.image.r");
		} else if (step_number < 3) {
			edje_object_signal_emit(_EDJ(ad->slider), "press04",
					"lock.image.r");
		} else if (step_number < 4) {
			edje_object_signal_emit(_EDJ(ad->slider), "press05",
					"lock.image.r");
		} else if (step_number < 5) {
			edje_object_signal_emit(_EDJ(ad->slider), "press06",
					"lock.image.r");
		} else if (step_number < 6) {
			edje_object_signal_emit(_EDJ(ad->slider), "press07",
					"lock.image.r");
		} else if (step_number < 7) {
			edje_object_signal_emit(_EDJ(ad->slider), "press08",
					"lock.image.r");
		} else if (step_number < 8) {
			edje_object_signal_emit(_EDJ(ad->slider), "press09",
					"lock.image.r");
		} else if (step_number < 9) {
			edje_object_signal_emit(_EDJ(ad->slider), "press10",
					"lock.image.r");
		} else if (step_number < 10) {
			edje_object_signal_emit(_EDJ(ad->slider), "press11",
					"lock.image.r");
		} else if (step_number < 11) {
			edje_object_signal_emit(_EDJ(ad->slider), "press12",
					"lock.image.r");
		} else if (step_number < 12) {
			edje_object_signal_emit(_EDJ(ad->slider), "press13",
					"lock.image.r");
		} else if (step_number < 13) {
			edje_object_signal_emit(_EDJ(ad->slider), "press14",
					"lock.image.r");
		} else {
			edje_object_signal_emit(_EDJ(ad->slider), "press15",
					"lock.image.r");
		}
		evas_object_color_set(ad->ly_main, alpha, alpha, alpha, alpha);
	} else {
		LOCK_SCREEN_TRACE_DBG("sliding is canceled");
		ad->bDrag = 0;

		evas_object_color_set(ad->ly_main, 255, 255, 255, 255);

		edje_object_signal_emit(_EDJ(ad->slider), "release",
				"lock.image.l");
		edje_object_signal_emit(_EDJ(ad->slider), "release",
				"lock.image.r");
	}

}

void _slider_up_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	struct appdata *ad = (struct appdata *) data;

	if (ad == NULL) {
		LOCK_SCREEN_TRACE_DBG("appdata is NULL");
		return;
	}

	if (ev == NULL) {
		LOCK_SCREEN_TRACE_DBG("event_info is NULL");
		return;
	}

	if (ad->bDrag == 1 && ad->slider_rel2.x <= ev->canvas.x
			&& ad->slider_rel2.y <= ev->canvas.y
			&& (ad->slider_rel2.x + ad->slider_rel2.w) >= ev->canvas.x
			&& (ad->slider_rel2.y + ad->slider_rel2.h) >= ev->canvas.y) {
		ad->bDrag = 1;
	} else {
		LOCK_SCREEN_TRACE_DBG("sliding is canceled");
		ad->bDrag= 0;
	}

	edje_object_signal_emit(_EDJ(ad->slider), "release", "lock.image.l");
	edje_object_signal_emit(_EDJ(ad->slider), "release", "lock.image.r");

	evas_object_color_set(ad->ly_main, 255, 255, 255, 255);

	if (ad->bDrag == 1) {
		ad->bDrag = 0;
		if(ad->slider) {
			evas_object_del(ad->slider);
			ad->slider = NULL;
		}
		LOCK_SCREEN_TRACE_DBG("unlock the lock-screen");
		elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_HIDE);
		elm_object_signal_emit(ad->ly_main, "transit,clipper", "clipper");
		vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);
	}
}
