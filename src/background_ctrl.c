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

#include "lockscreen.h"
#include "log.h"
#include "background_ctrl.h"
#include "background.h"
#include "main_view.h"

#include <Ecore.h>

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;

static void _lockscreen_background_ctrl_background_update(void)
{
	if (!lockscreen_main_view_background_set(main_view, LOCKSCREEN_BACKGROUND_TYPE_DEFAULT, lockscreen_background_file_get()))
		FAT("lockscreen_main_view_background_image_set failed");
}

static Eina_Bool
_lockscreen_background_ctrl_background_changed(void *data, int event, void *event_info)
{
	_lockscreen_background_ctrl_background_update();
	return EINA_TRUE;
}

int lockscreen_background_ctrl_init(Evas_Object *view)
{
	if (lockscreen_background_init()) {
		ERR("lockscreen_background_init failed");
		return 1;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_BACKGROUND_CHANGED, _lockscreen_background_ctrl_background_changed, NULL);
	if (!handler)
		FAT("ecore_event_handler_add failed on LOCKSCREEN_EVENT_BACKGROUND_CHANGED event");
	main_view = view;

	lockscreen_background_message_port_init();
	_lockscreen_background_ctrl_background_update();

	return 0;
}

void lockscreen_background_ctrl_shutdown(void)
{
	lockscreen_background_shutdown();
}
