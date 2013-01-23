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

#include <Ecore_X.h>
#include <Elementary.h>
#include <utilX.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <ail.h>
#include <ui-gadget.h>

#include "util.h"
#include "info.h"
#include "bg.h"
#include "noti.h"

#define DEFAULT_BG_PATH     "/opt/share/settings/Wallpapers/Home_default.jpg"

static Evas_Coord pos_down_y = 0;

void _set_win_property(Evas_Object * win)
{
	Ecore_X_Window xwin;

	if (win == NULL) {
		LOGD("[Error:%s] Invalid argument: win is NULL", __func__);
		return;
	}
	xwin = elm_win_xwindow_get(win);
	if (xwin == 0) {
		LOGD("[Error:%s] Cannot get xwindow", __func__);
		return;
	}

	ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);

	utilx_set_system_notification_level(ecore_x_display_get(), xwin, UTILX_NOTIFICATION_LEVEL_NORMAL);

	utilx_set_window_opaque_state(ecore_x_display_get(), xwin, UTILX_OPAQUE_STATE_ON);
}

Evas_Object *_add_layout(Evas_Object *parent, const char *file, const char *group)
{
	if (parent == NULL)
		return NULL;

	Evas_Object *ly;
	int r;

	ly = elm_layout_add(parent);
	if (ly == NULL)
		return NULL;

	r = elm_layout_file_set(ly, file, group);
	if (!r) {
		evas_object_del(ly);
		return NULL;
	}
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_show(ly);

	return ly;
}

Evas_Object *_make_top_layout(struct appdata *ad)
{
	if (ad == NULL)
		return NULL;

	Evas_Object *eo = NULL;
	Evas_Object *conform = NULL;

	conform = elm_conformant_add(ad->win);
	if(conform == NULL) {
		return NULL;
	}
	elm_object_style_set(conform, "indicator_overlap");

	eo = _add_layout(conform, EDJEFILE, "lock-main");
	if (eo == NULL)
		return NULL;

	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, conform);
	elm_object_content_set(conform, eo);
	evas_object_show(conform);
	evas_object_resize(eo, ad->win_w, ad->win_h);

	return eo;
}

Evas_Object *_make_slider(Evas_Object *parent)
{
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		LOCK_SCREEN_TRACE_DBG("The parent of slider is null");
		return NULL;
	}

	layout = _add_layout(parent, EDJEFILE, "lock-slider");
	if (layout == NULL) {
		return NULL;
	}

	return layout;
}

Evas_Object *_get_bg_image(Evas_Object *parent)
{
	Evas_Object *bg = NULL;
	char *file = NULL;
	Eina_Bool ret = EINA_FALSE;

	if (parent == NULL)
		return NULL;

	if ((file = vconf_get_str(VCONFKEY_IDLE_LOCK_BGSET)) ==  NULL) {
		file = vconf_get_str(VCONFKEY_BGSET);
	}

	bg = elm_icon_add(parent);
	if (bg == NULL)
		return NULL;
	elm_icon_aspect_fixed_set(bg, EINA_FALSE);

	if (file) {
		ret = elm_icon_file_set(bg, file, NULL);
		if (ret == EINA_FALSE) {
			elm_icon_file_set(bg, DEFAULT_BG_PATH, NULL);
		}
		free(file);
	} else {
		elm_icon_file_set(bg, DEFAULT_BG_PATH, NULL);
	}
	return bg;
}

void lockscreen_info_show(struct appdata *ad)
{
	if(ad == NULL){
		return;
	}

	edje_object_signal_emit(_EDJ(ad->ly_main), "show,text.area", "text.area");
	edje_object_signal_emit(_EDJ(ad->ly_main), "show,sw.noti", "sw.noti");
	edje_object_signal_emit(_EDJ(ad->ly_main), "show,sim.state", "sim.state");
	edje_object_signal_emit(_EDJ(ad->ly_main), "show,rect.info", "rect.info");
	if(ad->slider != NULL) {
		evas_object_show(ad->slider);
	}
}

void lockscreen_info_hide(struct appdata *ad)
{
	if(ad == NULL){
		return;
	}

	edje_object_signal_emit(_EDJ(ad->ly_main), "hide,text.area", "text.area");
	edje_object_signal_emit(_EDJ(ad->ly_main), "hide,sw.noti", "sw.noti");
	edje_object_signal_emit(_EDJ(ad->ly_main), "hide,sim.state", "sim.state");
	edje_object_signal_emit(_EDJ(ad->ly_main), "hide,rect.info", "rect.info");
	if(ad->slider != NULL) {
		evas_object_hide(ad->slider);
	}
}

static void _set_sim_state(void *data)
{
	struct appdata *ad = data;
	if (ad == NULL)
		return;

	int state = 0;
	int ret = 0;
	char *buf = NULL;

	int service_type = VCONFKEY_TELEPHONY_SVCTYPE_NONE;

	if(vconf_get_int(VCONFKEY_TELEPHONY_SVCTYPE, &service_type) != 0) {
		LOGD("fail to get VCONFKEY_TELEPHONY_SVCTYPE");
	}

	ret = (vconf_get_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, &state));
	if (ret == 0) {
		LOGD("[%s:%d] VCONFKEY(%s) = %d", __func__, __LINE__, VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, state);
		if (state != VCONFKEY_TELEPHONY_DISP_INVALID
			&& service_type > VCONFKEY_TELEPHONY_SVCTYPE_SEARCH) {
			if (state & VCONFKEY_TELEPHONY_DISP_SPN) {
				buf = vconf_get_str(VCONFKEY_TELEPHONY_SPN_NAME);
				edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", buf);
			}

			if (state & VCONFKEY_TELEPHONY_DISP_PLMN) {
				buf = vconf_get_str(VCONFKEY_TELEPHONY_NWNAME);
				edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", buf);
			}
		} else {
			edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", "");
		}
	}
}

static void _app_exit(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct appdata *ad = data;
	_app_terminate(ad);
}

static Eina_Bool _init_widget_cb(void *data)
{
	struct appdata *ad = data;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL)
		return ECORE_CALLBACK_CANCEL;

	ad->slider = _make_slider(ad->ly_main);
	evas_object_resize(ad->slider, ad->win_w, SLIDER_RATIO_H * ad->win_h);
	evas_object_move(ad->slider, 0, SLIDER_RATIO_Y * ad->win_h);
	evas_object_show(ad->slider);
	evas_object_geometry_get(
			edje_object_part_object_get(_EDJ(ad->slider), "lock.wrapper.image.l"),
			&ad->slider_rel1.x, &ad->slider_rel1.y, &ad->slider_rel1.w,
			&ad->slider_rel1.h);
	evas_object_geometry_get(
			edje_object_part_object_get(_EDJ(ad->slider), "lock.wrapper.image.r"),
			&ad->slider_rel2.x, &ad->slider_rel2.y, &ad->slider_rel2.w,
			&ad->slider_rel2.h);

	_set_sim_state(ad);

	ad->info = elm_layout_add(ad->win);
	elm_layout_file_set(ad->info, EDJEFILE, "lock-info");
	evas_object_show(ad->info);
	elm_object_part_content_set(ad->ly_main, "rect.info", ad->info);
	_set_info(ad);

	int state = 0;
	vconf_get_bool(VCONFKEY_LOCKSCREEN_EVENT_NOTIFICATION_DISPLAY, &state);
	if(state){
		noti_process(ad);
	}

	evas_object_event_callback_add(ad->event_bg, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb_s, ad);
	evas_object_event_callback_add(ad->event_bg, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb_s, ad);
	evas_object_event_callback_add(ad->event_bg, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb_s, ad);

	evas_object_event_callback_add(ad->slider, EVAS_CALLBACK_MOUSE_DOWN, _slider_down_cb, ad);
	evas_object_event_callback_add(ad->slider, EVAS_CALLBACK_MOUSE_UP, _slider_up_cb, ad);
	evas_object_event_callback_add(ad->slider, EVAS_CALLBACK_MOUSE_MOVE, _slider_move_cb, ad);

	edje_object_signal_callback_add(_EDJ(ad->ly_main), "exit,app", "event", _app_exit, ad);

	vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_LOCK);

	return ECORE_CALLBACK_CANCEL;
}

int _app_create(struct appdata *ad)
{
	if (ad == NULL)
		return -1;
	LOGD("[ == %s == ]", __func__);

	ecore_x_icccm_name_class_set(elm_win_xwindow_get(ad->win), "LOCK_SCREEN", "LOCK_SCREEN");
	evas_object_show(ad->win);
	_set_win_property(ad->win);

	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);

	ad->ly_main = _make_top_layout(ad);
	if (ad->ly_main == NULL)
		return -1;
	ad->event_bg = _get_bg_image(ad->ly_main);
	if (ad->event_bg == NULL)
		return -1;
	elm_object_part_content_set(ad->ly_main, "sw.bg", ad->event_bg);

	return 0;
}

int _app_reset(struct appdata *ad)
{
	if (ad == NULL)
		return -1;

	if (ad->emgc_ug) {
		ug_destroy(ad->emgc_ug);
		ad->emgc_ug = NULL;
	}

	static int initted = 0;
	if(initted == 0) {
		ecore_idler_add(_init_widget_cb, ad);
		initted = 1;
	}

	return 0;
}

int _app_resume(struct appdata *ad)
{
	if (ad == NULL)
		return -1;

	return 0;
}

int _app_terminate(struct appdata *ad)
{
	vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);

	if (ad == NULL) {
		LOGD("[_app_terminate] Invalid argument : struct appdata is NULL\n");
		return -1;
	}

	LOGD("[%s] app termiante", __func__);
	elm_exit();

	return 0;
}

static void __layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	struct appdata *ad;
	Evas_Object *base;

	if (!ug || !priv)
		return;

	ad = priv;
	base = ug_get_layout(ug);
	if (!base)
		return;

	switch(mode) {
	case UG_MODE_FULLVIEW:
		LOCK_SCREEN_TRACE_DBG("[%s:%d]", __func__, __LINE__);
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(ad->win, base);
		evas_object_show(base);
		break;
	case UG_MODE_FRAMEVIEW:
		break;
	default:
		break;
	}
}

static void __result_cb(ui_gadget_h ug, service_h service, void *priv)
{
	LOCK_SCREEN_TRACE_DBG("result_cb\n");
}

static void __destroy_cb(ui_gadget_h ug, void *priv)
{
	struct appdata *ad = (struct appdata *)priv;

	if (!ug || !ad) {
		return;
	}
	LOCK_SCREEN_TRACE_DBG("[%s:%d]", __func__, __LINE__);

	ug_destroy(ug);
	ug = NULL;
}

void launch_emgcall(struct appdata *ad)
{
	if (ad == NULL)
		return;

	service_h service;
	service_create(&service);
	UG_INIT_EFL(ad->win, UG_OPT_INDICATOR_ENABLE);
	struct ug_cbs *cbs = (struct ug_cbs *)calloc(1, sizeof(struct ug_cbs));
	if (cbs == NULL) {
		service_destroy(service);
		return;
	}
	cbs->layout_cb = __layout_cb;
	cbs->result_cb = __result_cb;
	cbs->destroy_cb = __destroy_cb;
	cbs->priv = (void *)ad;

	LOCK_SCREEN_TRACE_DBG("[%s:%d]", __func__, __LINE__);

	if (!service) {
		service_destroy(service);
		free(cbs);
		return;
	}

	service_add_extra_data(service, "emergency_dialer", "emergency");
	ad->emgc_ug = ug_create(NULL, "dialer-efl", UG_MODE_FULLVIEW, service, cbs);
	service_destroy(service);
	free(cbs);

	if (!ad->emgc_ug) {
		LOCK_SCREEN_TRACE_DBG("dialer ug failed");
		return;
	}
}
