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

#include "log.h"
#include "time_format.h"
#include "display.h"
#include "main_view.h"

#include <Ecore.h>
#include <time.h>

static Ecore_Event_Handler *handler, *display_handler;
static Ecore_Timer *update_timer;
static Evas_Object *main_view;

static void _time_update(void)
{
	lockscreen_main_view_time_set(main_view, lockscreen_time_format_locale_get(),
			lockscreen_time_format_timezone_get(), lockscreen_time_format_use_24h(), time(NULL));
}

static Eina_Bool _timer_cb(void *data)
{
	_time_update();
	ecore_timer_interval_set(update_timer, 60.0);
	return ECORE_CALLBACK_RENEW;
}

static void _time_spawn_align(void)
{
	time_t tt;
	struct tm st;

	tt = time(NULL);
	localtime_r(&tt, &st);

	ecore_timer_interval_set(update_timer, 60 - st.tm_sec);
}

static Eina_Bool _time_changed(void *data, int event, void *event_info)
{
	_time_update();
	_time_spawn_align();
	return EINA_TRUE;
}

static Eina_Bool _display_status_changed(void *data, int event, void *event_info)
{
	if (lockscreen_display_is_off()) {
		if (update_timer) ecore_timer_freeze(update_timer);
	}
	else {
		_time_update();
		_time_spawn_align();
	}
	return EINA_TRUE;
}

int lockscreen_time_format_ctrl_init(Evas_Object *view)
{
	if (lockscreen_display_init()) {
		FAT("lockscreen_display_init failed");
		return 1;
	}

	if (lockscreen_time_format_init()) {
		lockscreen_display_shutdown();
		FAT("lockscreen_time_format_init failed");
		return 1;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_TIME_FORMAT_CHANGED, _time_changed, NULL);
	if (!handler)
		FAT("ecore_event_handler_add failed on LOCKSCREEN_DATA_MODEL_EVENT_TIME_FORMAT_CHANGED event");
	display_handler = ecore_event_handler_add(LOCKSCREEN_EVENT_DISPLAY_STATUS_CHANGED, _display_status_changed, NULL);
	if (!display_handler)
		FAT("ecore_event_handler_add failed on LOCKSCREEN_DATA_MODEL_EVENT_LCD_STATUS_CHANGED event");
	main_view = view;
	update_timer = ecore_timer_add(60.0, _timer_cb, NULL);
	_time_update();
	_time_spawn_align();

	return 0;
}

void lockscreen_time_ctrl_shutdown(void)
{
	ecore_timer_del(update_timer);
	ecore_event_handler_del(handler);
	ecore_event_handler_del(display_handler);
	lockscreen_display_shutdown();
	lockscreen_time_format_shutdown();
}

void lockscreen_time_format_ctrl_time_update(void)
{
	_time_update();
}

