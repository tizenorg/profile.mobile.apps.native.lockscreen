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

#include <stdio.h>
#include <stdlib.h>

#include <Evas.h>
#include <Elementary.h>
#include <Eina.h>
#include <efl_extension.h>

#include <app.h>
#include <vconf.h>
#include <aul.h>
#include <feedback.h>
#include <dd-display.h>

#include "lockscreen.h"
#include "log.h"
#include "property.h"
#include "window.h"
#include "background_view.h"
#include "default_lock.h"
#include "dbus.h"

#define LOCK_CONTROL_TYPE_KEY "lock_type"
#define LOCK_CONTROL_TYPE_VALUE_RECOVERY "recovery_lock"
#define LOCK_CONTROL_KEY "lock_op"
#define LOCK_CONTROL_VALUE_START_READY "start_ready"

#define LOCK_LCD_OFF_TIMEOUT_TIME 10

static struct _s_info {
	Elm_Theme *theme;
	Ecore_Timer *lcd_off_timer;
	int lock_type;
	int lcd_off_count;

} s_info = {
	.theme = NULL,
	.lcd_off_timer = NULL,
	.lock_type = 0,
	.lcd_off_count = 0,
};

Elm_Theme *lockscreen_theme_get(void)
{
	return s_info.theme;
}

int lockscreen_setting_lock_type_get(void)
{
	return s_info.lock_type;
}

Ecore_Timer *lockscreen_lcd_off_timer_get(void)
{
	return s_info.lcd_off_timer;
}

int lockscreen_lcd_off_count_get(void)
{
	return s_info.lcd_off_count;
}

void lockscreen_feedback_tap_play(void)
{
	if (!lock_property_sound_touch_get()) {
		return;
	}

	feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
}

static Eina_Bool _lcd_off_timer_cb(void *data)
{
	int ret = 0;

	ret = display_change_state(LCD_OFF);
	if (ret != 0) {
		_E("Failed to change LCD state : LCD_OFF(%d)", ret);
	} else {
		_I("lcd off : %dsec", LOCK_LCD_OFF_TIMEOUT_TIME);
	}

	return ECORE_CALLBACK_CANCEL;
}

void lockscreen_lcd_off_timer_set(void)
{
	if (s_info.lcd_off_timer) {
		ecore_timer_del(s_info.lcd_off_timer);
		s_info.lcd_off_timer = NULL;
	}

	s_info.lcd_off_timer = ecore_timer_add(LOCK_LCD_OFF_TIMEOUT_TIME, _lcd_off_timer_cb, NULL);
}

void lockscreen_lcd_off_timer_reset(void)
{
	if (s_info.lcd_off_timer) {
		ecore_timer_reset(s_info.lcd_off_timer);
	}
}

void lockscreen_lcd_off_timer_unset(void)
{
	if (s_info.lcd_off_timer) {
		ecore_timer_del(s_info.lcd_off_timer);
		s_info.lcd_off_timer = NULL;
		_I("unset lcd off timer");
	}
}

void lockscreen_lcd_off_count_raise(void)
{
	if (s_info.lcd_off_count < 3) {
		_D("count for lcd off timer : %d", s_info.lcd_off_count);
		lockscreen_lcd_off_timer_reset();
		s_info.lcd_off_count++;
	}
}

void lockscreen_lcd_off_count_reset(void)
{
	_D("lcd off count reset : %d -> 0", s_info.lcd_off_count);
	s_info.lcd_off_count = 0;
}

static void _init_theme(void)
{
	s_info.theme = elm_theme_new();
	elm_theme_ref_set(s_info.theme, NULL);
	elm_theme_extension_add(s_info.theme, EDJE_DIR"index.edj");
}

static void _fini_theme(void)
{
	elm_theme_extension_del(s_info.theme, EDJE_DIR"index.edj");
	elm_theme_free(s_info.theme);
	s_info.theme = NULL;
}

static Eina_Bool _lock_idler_cb(void *data)
{
	_init_theme();

	if (LOCK_ERROR_OK != lock_default_lock_init()) {
		_E("Failed to initialize default lockscreen");
		return ECORE_CALLBACK_CANCEL;
	}

	/* register callback func. : key sound, touch sound, rotation */
	lock_property_register(NULL);

	/* initialize dbus */
	lock_dbus_init(NULL);

	feedback_initialize();

	lockscreen_lcd_off_timer_set();

#if 0
	/* set rotation changed cb */
	if (elm_win_wm_rotation_supported_get(win)) {
		int rots[4] = { 0, };
		elm_win_wm_rotation_available_rotations_set(win, rots, 0);
	}
#endif

	return ECORE_CALLBACK_CANCEL;
}

static void _back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	_D("%s", __func__);

	lockscreen_feedback_tap_play();
}

bool _create_app(void *data)
{
	_D("%s", __func__);

	elm_config_accel_preference_set("opengl");

	Evas_Object *win = NULL;
	Evas_Object *bg = NULL;
	int locktype = 0;
	int ret = 0;

	_D("base scale : %f", elm_app_base_scale_get());

	/* Get lockscreen type */
	ret = lock_property_get_int(PROPERTY_TYPE_VCONFKEY, VCONFKEY_SETAPPL_SCREEN_LOCK_TYPE_INT, &locktype);
	if (ret != LOCK_ERROR_OK) {
		_E("Failed to get lockscreen type. Set default lockscreen.");
		locktype = SETTING_SCREEN_LOCK_TYPE_SWIPE;
	}
	_D("lockscreen type : %d", locktype);
	s_info.lock_type = locktype;

	/* Create lockscreen window */
	win = lock_window_create(locktype);
	retv_if(!win, false);

	/* Create lockscreen BG */
	bg = lock_background_view_bg_create(win);
	if (!bg) {
		_E("Failed to create BG");
	}

	evas_object_show(win);

	ecore_idler_add(_lock_idler_cb, NULL);

	eext_object_event_callback_add(win, EEXT_CALLBACK_BACK, _back_key_cb, NULL);

	return true;
}

static void _app_control(app_control_h control, void *data)
{
	char *control_val = NULL;

	app_control_get_extra_data(control, LOCK_CONTROL_TYPE_KEY, &control_val);
	_I("control value : %s", control_val);

	if (control_val) {
		free(control_val);
		control_val = NULL;
	}
}

void _terminate_app(void *data)
{
	_D("%s", __func__);

	lock_default_lock_fini();

	lock_property_unregister();
	feedback_deinitialize();
	lock_dbus_fini(NULL);
	lock_window_destroy();

	_fini_theme();
}

void _pause_app(void *user_data)
{
	_D("%s", __func__);
}

void _resume_app(void *user_data)
{
	_D("%s", __func__);
}

static void _language_changed(void *data)
{
	_D("%s", __func__);
}

int main(int argc, char *argv[])
{
	int ret = 0;

	ui_app_lifecycle_callback_s lifecycle_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	lifecycle_callback.create = _create_app;
	lifecycle_callback.terminate = _terminate_app;
	lifecycle_callback.pause = _pause_app;
	lifecycle_callback.resume = _resume_app;
	lifecycle_callback.app_control = _app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, NULL, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, NULL, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, NULL, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, (void *)_language_changed, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, NULL, NULL);

	ret = ui_app_main(argc, argv, &lifecycle_callback, NULL);
	if (ret != APP_ERROR_NONE) {
		_E("app_main() is failed. err = %d", ret);
	}

	return ret;
}
