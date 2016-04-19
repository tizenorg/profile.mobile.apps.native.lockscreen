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

#include "log.h"
#include "main_view.h"
#include "sim.h"

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;

static void _sim_state_view_update()
{
	const char *sim1, *sim2;
	char buf[128] = {0,};

	sim1 = lockscreen_sim_get_plmn(LOCKSCREEN_PRIMARY_SIM);
	sim2 = lockscreen_sim_get_plmn(LOCKSCREEN_SECONDARY_SIM);

	if (sim1 && sim2) {
		snprintf(buf, sizeof(buf), "%s / %s", sim1, sim2);
	} else if (sim1) {
		snprintf(buf, sizeof(buf), "%s", sim1);
	} else if (sim2) {
		snprintf(buf, sizeof(buf), "%s", sim2);
	}

	lockscreen_main_view_sim_status_text_set(main_view, buf);
}

static Eina_Bool _sim_status_changed(void *data, int type, void *event_info)
{
	_sim_state_view_update();
	return EINA_TRUE;
}

int lockscreen_sim_ctrl_init(Evas_Object *view)
{
	if (lockscreen_sim_init()) {
		ERR("lockscreen_sim_init failed");
		return 1;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_SIM_STATUS_CHANGED, _sim_status_changed, NULL);
	main_view = view;
	_sim_state_view_update();
	return 0;
}

void lockscreen_sim_ctrl_shutdown()
{
	ecore_event_handler_del(handler);
	lockscreen_sim_shutdown();
}
