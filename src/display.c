/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#include <device/display.h>
#include <device/callback.h>
#include <Ecore.h>

#include "display.h"
#include "log.h"

#define LOCK_LCD_OFF_TIMEOUT_TIME 10

static Ecore_Timer *lcd_off_timer;
int LOCKSCREEN_EVENT_DISPLAY_STATUS_CHANGED;
static int init_count;
static int display_off;

static Eina_Bool _time_elapsed(void *data)
{
	int ret = device_display_change_state(DISPLAY_STATE_SCREEN_OFF);
	if (ret != DEVICE_ERROR_NONE) {
		ERR("device_display_change_state failed: %s", get_error_message(ret));
	}

	lcd_off_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void _timer_reset(void)
{
	if (lcd_off_timer) {
		ecore_timer_reset(lcd_off_timer);
	} else {
		lcd_off_timer = ecore_timer_add(LOCK_LCD_OFF_TIMEOUT_TIME, _time_elapsed, NULL);
	}
}

static void _display_status_changed(device_callback_e type, void *value, void *user_data)
{
	if (type != DEVICE_CALLBACK_DISPLAY_STATE)
		return;

	display_state_e state = (display_state_e)value;

	switch (state) {
		case DISPLAY_STATE_NORMAL:
		case DISPLAY_STATE_SCREEN_DIM:
			INF("Display on");
			_timer_reset();
			display_off = false;
		break;
		case DISPLAY_STATE_SCREEN_OFF:
			INF("Display off");
			display_off = true;
		break;
	}

	ecore_event_add(LOCKSCREEN_EVENT_DISPLAY_STATUS_CHANGED, NULL, NULL, NULL);
}

int lockscreen_display_init(void)
{
	display_state_e state;

	if (!init_count) {
		LOCKSCREEN_EVENT_DISPLAY_STATUS_CHANGED = ecore_event_type_new();
		int ret = device_add_callback(DEVICE_CALLBACK_DISPLAY_STATE, _display_status_changed, NULL);
		if (ret != DEVICE_ERROR_NONE) {
			ERR("device_add_callback failed: %s", get_error_message(ret));
			return 1;
		}
		ret = device_display_get_state(&state);
		if (ret != DEVICE_ERROR_NONE) {
			ERR("device_display_get_state failed: %s", get_error_message(ret));
			device_remove_callback(DEVICE_CALLBACK_DISPLAY_STATE, _display_status_changed);
			return 1;
		}

		switch (state) {
			case DISPLAY_STATE_NORMAL:
			case DISPLAY_STATE_SCREEN_DIM:
				display_off = false;
				break;
			case DISPLAY_STATE_SCREEN_OFF:
				display_off = true;
				break;
		}

		_timer_reset();
	}

	init_count++;
	return 0;
}

void lockscreen_display_shutdown(void)
{
	if (init_count) {
		init_count--;

		if (!init_count) {
			if (lcd_off_timer) ecore_timer_del(lcd_off_timer);
			device_remove_callback(DEVICE_CALLBACK_DISPLAY_STATE, _display_status_changed);
		}
	}
}

void lockscreen_display_timer_freeze(void)
{
	if (lcd_off_timer) {
		ecore_timer_freeze(lcd_off_timer);
	}
}

void lockscreen_display_timer_renew(void)
{
	if (lcd_off_timer) {
		ecore_timer_thaw(lcd_off_timer);
		ecore_timer_reset(lcd_off_timer);
	}
}

bool lockscreen_display_is_off(void)
{
	return display_off;
}
