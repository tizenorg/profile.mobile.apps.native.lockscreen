/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <app.h>
#include <vconf.h>
#include <Ecore_X.h>
#include <system_info.h>

#include "lockscreen.h"
#include "util.h"

#define QP_EMUL_STR      "Emulator"

static void win_del(void *data, Evas_Object * obj, void *event)
{
	elm_exit();
}

static Evas_Object *create_win(const char *name)
{
	if (name == NULL)
		return NULL;

	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (!eo) {
		LOGE("[%s:%d] eo is NULL", __func__, __LINE__);
		return NULL;
	}

	elm_win_title_set(eo, name);
	elm_win_borderless_set(eo, EINA_TRUE);
	evas_object_smart_callback_add(eo, "delete,request", win_del, NULL);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(eo, w, h);
	elm_win_alpha_set(eo, EINA_TRUE);

	return eo;
}

static void resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, NULL, NULL, &w, &h);
}

static int _check_emul()
{
	int is_emul = 0;
	char *info = NULL;

	if (system_info_get_value_string(SYSTEM_INFO_KEY_MODEL, &info) == 0) {
		if (info == NULL) return 0;
		if (!strncmp(QP_EMUL_STR, info, strlen(info))) {
			is_emul = 1;
		}
	}

	if (info != NULL) free(info);

	return is_emul;
}

static bool app_create(void *data)
{
	struct appdata *ad = data;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL)
		return false;

	ad->win = NULL;
	ad->ly_main = NULL;
	ad->info = NULL;
	ad->event_bg = NULL;
	ad->slider = NULL;
	ad->bDrag = 0;

	Evas_Object *win = NULL;

	ecore_x_window_size_get(ecore_x_window_root_first_get(), &ad->win_w, &ad->win_h);
	LOGD("[%s:%d] win_w : %d, win_h : %d", __func__, __LINE__, ad->win_w, ad->win_h);

	win = create_win(PACKAGE);
	if (win == NULL)
		return false;
	ad->win = win;
	elm_win_conformant_set(ad->win, EINA_TRUE);
	evas_object_resize(win, ad->win_w, ad->win_h);
	evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, resize_cb, NULL);

	int ret = _app_create(ad);
	if(ret == -1)
		return false;

	return true;
}

static void app_service(service_h service, void *data)
{
	struct appdata *ad = data;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL)
		return;

	if (ad->win)
		elm_win_activate(ad->win);

	_app_reset(ad);
}

static void app_terminate(void *data)
{
	struct appdata *ad = data;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL)
		return;

	if (ad->ly_main)
		evas_object_del(ad->ly_main);

	if (ad->win)
		evas_object_del(ad->win);
}

static void app_pause(void *data)
{
	struct appdata *ad = data;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL)
		return;
}

static void app_resume(void *data)
{
	struct appdata *ad = data;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL)
		return;
}

static void device_orientation(app_device_orientation_e orientation, void * data)
{
	struct appdata *ad = data;
	LOGD("[ == %s == ]", __func__);
	if (ad == NULL || ad->win == NULL)
		return;
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	app_event_callback_s event_callback;

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.service = app_service;
	event_callback.low_memory = NULL;
	event_callback.low_battery = NULL;
	event_callback.device_orientation = device_orientation;
	event_callback.language_changed = NULL;
	event_callback.region_format_changed = NULL;

	memset(&ad, 0x0, sizeof(struct appdata));

	if(!_check_emul()) {
		setenv("ELM_ENGINE", "gl", 1);
	}

	return app_efl_main(&argc, &argv, &event_callback, &ad);
}
