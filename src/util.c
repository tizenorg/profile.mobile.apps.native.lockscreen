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

#include <Elementary.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <ail.h>
#include <ui-gadget.h>
#include <heynoti.h>
#include <efl_util.h>

#include "util.h"
#include "sim-state.h"
#include "info.h"
#include "bg.h"
#include "noti.h"

#define DEFAULT_BG_PATH     "/opt/share/settings/Wallpapers/Home_default.jpg"
#define SYSTEM_RESUME       "system_wakeup"

static Evas_Coord pos_down_y = 0;

void _set_win_property(Evas_Object * win)
{
	if (win == NULL) {
		LOGD("[Error:%s] Invalid argument: win is NULL", __func__);
		return;
	}

	efl_util_set_system_notification_level(win, EFL_UTIL_NOTIFICATION_LEVEL_NORMAL);
	efl_util_set_window_opaque_state(win, EFL_UTIL_OPAQUE_STATE_ON);
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

#if 0
	elm_object_style_set(conform, "indicator_overlap");
#else
	elm_object_signal_emit(conform, "elm,state,virtualkeypad,disable", "");
	elm_object_signal_emit(conform, "elm,state,indicator,overlap", "");
	elm_object_signal_emit(conform, "elm,state,clipboard,disable", "");
#endif

	eo = _add_layout(conform, EDJEFILE, "lock-main");
	if (eo == NULL)
		return NULL;

	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, conform);
	elm_object_content_set(conform, eo);
	evas_object_show(conform);

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

static void _app_exit(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct appdata *ad = data;
	_app_terminate(ad);
}

static int _init_heynoti(void *data)
{
	struct appdata *ad = data;
	if (ad == NULL) {
		return EXIT_FAILURE;
	}
	int fd = -1, ret = -1;
	LOCK_SCREEN_TRACE_DBG("[ == %s == ]", __func__);

	fd = heynoti_init();
	if (fd == -1) {
		LOCK_SCREEN_TRACE_DBG("Heynoti init error\n");
		return EXIT_FAILURE;
	}

	ret = heynoti_subscribe(fd, SYSTEM_RESUME, update_time, ad);
	if (ret == -1) {
		LOCK_SCREEN_TRACE_DBG("[Error] heynoti_subscribe : system_wakeup\n");
		return EXIT_FAILURE;
	}

	ret = heynoti_attach_handler(fd);
	if (ret == -1) {
		LOCK_SCREEN_TRACE_DBG("[Error] heynoti_attach_handler failed.\n");
		return EXIT_FAILURE;
	}

	ad->heynoti_fd = fd;

	return EXIT_SUCCESS;
}

static void _fini_heynoti(void *data)
{
	struct appdata *ad = data;
	if (ad == NULL) {
		return;
	}
	LOCK_SCREEN_TRACE_DBG("[ == %s == ]", __func__);
	heynoti_unsubscribe(ad->heynoti_fd, SYSTEM_RESUME, update_time);
	heynoti_close(ad->heynoti_fd);
	ad->heynoti_fd = 0;
}

static void _pm_state_cb(keynode_t * node, void *data)
{
	LOCK_SCREEN_TRACE_DBG("_pm_state_cb");

	struct appdata *ad = data;
	int val = -1;

	if (vconf_get_int(VCONFKEY_PM_STATE, &val) < 0) {
		LOCK_SCREEN_TRACE_ERR("Cannot get VCONFKEY_PM_STATE");
		return;
	}

	if (val == VCONFKEY_PM_STATE_NORMAL) {
		LOCK_SCREEN_TRACE_DBG("LCD on");
		update_time(ad);
	}
}

static Eina_Bool _init_widget_cb(void *data)
{
	struct appdata *ad = data;
	int width, height;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL)
		return ECORE_CALLBACK_CANCEL;

	elm_win_screen_size_get(ad->win, NULL, NULL, &width, &height);

	ad->slider = _make_slider(ad->ly_main);
	evas_object_resize(ad->slider, width, SLIDER_RATIO_H * height);
	evas_object_move(ad->slider, 0, SLIDER_RATIO_Y * height);
	evas_object_show(ad->slider);
	evas_object_geometry_get(
			edje_object_part_object_get(_EDJ(ad->slider), "lock.wrapper.image.l"),
			&ad->slider_rel1.x, &ad->slider_rel1.y, &ad->slider_rel1.w,
			&ad->slider_rel1.h);
	evas_object_geometry_get(
			edje_object_part_object_get(_EDJ(ad->slider), "lock.wrapper.image.r"),
			&ad->slider_rel2.x, &ad->slider_rel2.y, &ad->slider_rel2.w,
			&ad->slider_rel2.h);

	set_sim_state(ad);

	ad->info = elm_layout_add(ad->win);
	elm_layout_file_set(ad->info, EDJEFILE, "lock-info");
	evas_object_show(ad->info);
	elm_object_part_content_set(ad->ly_main, "rect.info", ad->info);
	_set_info(ad);

	if(_init_heynoti(ad) != EXIT_SUCCESS) {
		LOCK_SCREEN_TRACE_DBG("heynoti ERR..!!");
	}

	if (vconf_notify_key_changed(VCONFKEY_PM_STATE, _pm_state_cb, ad) != 0) {
		LOCK_SCREEN_TRACE_ERR("Fail vconf_notify_key_changed : VCONFKEY_PM_STATE");
	}

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
	Ecore_Evas *ee;
	if (ad == NULL)
		return -1;
	LOGD("[ == %s == ]", __func__);

	ee = ecore_evas_object_ecore_evas_get(ad->win);
	ecore_evas_name_class_set(ee, "LOCK_SCREEN", "LOCK_SCREEN");

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
	if (ad == NULL) {
		LOGD("[_app_terminate] Invalid argument : struct appdata is NULL\n");
		return -1;
	}

	vconf_ignore_key_changed(VCONFKEY_PM_STATE, _pm_state_cb);
	fini_sim_state(ad);
	_fini_heynoti(ad);

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
