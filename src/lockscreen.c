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

#include <app.h>

#include "lockscreen.h"
#include "log.h"
#include "main_ctrl.h"
#include "app_control_ctrl.h"

bool _create_app(void *data)
{
	DBG("Lockscreen launch request.");
	elm_config_accel_preference_set("opengl");
	DBG("base scale : %f", elm_app_base_scale_get());
	DBG("edje scale : %f", edje_scale_get());

	lockscreen_main_ctrl_init();
	lockscreen_app_control_ctrl_init();

	return true;
}

void _terminate_app(void *data)
{
	DBG("Lockscreen terminated request.");
	lockscreen_app_control_ctrl_shutdown();
}

static void _app_control(app_control_h request, void *data)
{
	lockscreen_app_control_ctrl_handle(request);
}

int main(int argc, char *argv[])
{
	int ret = 0;

	ui_app_lifecycle_callback_s lifecycle_callback = {0,};

	lifecycle_callback.create = _create_app;
	lifecycle_callback.terminate = _terminate_app;
	lifecycle_callback.app_control = _app_control;

	ret = ui_app_main(argc, argv, &lifecycle_callback, NULL);
	if (ret != APP_ERROR_NONE) {
		ERR("ui_app_main failed: %s", get_error_message(ret));
	}

	return ret;
}
