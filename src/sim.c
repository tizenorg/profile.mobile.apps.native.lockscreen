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
#include "util.h"
#include "sim.h"

#include <telephony/telephony.h>
#include <Ecore.h>

static telephony_handle_list_s handle_list;
static char *sim_plmn[LOCKSCREEN_SIM_MAX];
static int init_count;
int LOCKSCREEN_EVENT_SIM_STATUS_CHANGED;

static const telephony_noti_e notis[] = {
	TELEPHONY_NOTI_SIM_STATUS,
	TELEPHONY_NOTI_NETWORK_NETWORK_NAME,
	TELEPHONY_NOTI_NETWORK_SERVICE_STATE,
};

static char *_sim_plmn_get(telephony_h handle)
{
	char *network_name;

	telephony_sim_lock_state_e
	int ret = telephony_network_get_network_name(handle, &network_name);
	if (ret != TELEPHONY_ERROR_NONE) {
		ERR("telephony_network_get_network_name failed: %s", get_error_message(ret));
		return NULL;
	}

	return network_name;
}

static char *_sim_spn_get(telephony_h handle)
{
	char *spn_name;

	int ret = telephony_sim_get_spn(handle, &spn_name);
	if (ret != TELEPHONY_ERROR_NONE) {
		ERR("telephony_sim_get_spn failed: %s", get_error_message(ret));
		return NULL;
	}
	return spn_name;
}

static char *_sim_state_text_for_sim_get(telephony_h handle)
{
	int ret;
	telephony_network_service_state_e service_state;
	telephony_network_name_option_e name_option;

	char *plmn = NULL;
	char *spn = NULL;
	char buf[1024] = { 0, };

	/* get service state */
	ret = telephony_network_get_service_state(handle, &service_state);
	if (ret != TELEPHONY_ERROR_NONE) {
		ERR("telephony_network_get_service_state failed: %s", get_error_message(ret));
		return NULL;
	}

	switch (service_state) {
	case TELEPHONY_NETWORK_SERVICE_STATE_IN_SERVICE:
		/* get network name option */
		ret = telephony_network_get_network_name_option(handle, &name_option);
		if (ret != TELEPHONY_ERROR_NONE) {
			ERR("telephony_network_get_network_name_option failed: %s", get_error_message(ret));
			return NULL;
		}

		switch (name_option) {
		case TELEPHONY_NETWORK_NAME_OPTION_SPN:
			spn = _sim_spn_get(handle);
			if (spn != NULL && spn[0] != 0) {
				INF("PLMN/SPN - Sim %p using SPN: %s", handle, spn);
				snprintf(buf, sizeof(buf), "%s", spn);
			}
			break;
		case TELEPHONY_NETWORK_NAME_OPTION_NETWORK:
			plmn = _sim_plmn_get(handle);
			if (plmn != NULL && plmn[0] != 0) {
				INF("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		case TELEPHONY_NETWORK_NAME_OPTION_ANY:
			spn = _sim_spn_get(handle);
			plmn = _sim_plmn_get(handle);
			if (spn != NULL && spn[0] != 0 && plmn != NULL && plmn[0] != 0) {
				INF("PLMN/SPN - Sim %p using SPN: %s, PLMN: %s", handle, spn, plmn);
				snprintf(buf, sizeof(buf), "%s - %s", plmn, spn);
			} else if (spn != NULL && spn[0] != 0) {
				INF("PLMN/SPN - Sim %p using SPN: %s", handle, spn);
				snprintf(buf, sizeof(buf), "%s", spn);
			} else if (plmn != NULL && plmn[0] != 0) {
				INF("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		default:
			ERR("Invalid name option[%d]", name_option);
			plmn = _sim_plmn_get(handle);
			if (plmn != NULL && plmn[0] != 0) {
				INF("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		}
		break;
	case TELEPHONY_NETWORK_SERVICE_STATE_OUT_OF_SERVICE:
		snprintf(buf, sizeof(buf), "%s", _("IDS_COM_BODY_NO_SERVICE"));
		break;
	case TELEPHONY_NETWORK_SERVICE_STATE_EMERGENCY_ONLY:
		snprintf(buf, sizeof(buf), "%s", _("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"));
		break;
	default:
		snprintf(buf, sizeof(buf), "%s", _("IDS_COM_BODY_NO_SERVICE"));
		break;
	}

	return strdup(buf);
}

static void _update_sim_info(bool emit)
{
	int i;
	telephony_sim_state_e state;

	for (i = 0; (i < handle_list.count) && (i < LOCKSCREEN_SIM_MAX); i++)
	{
		char *out = NULL;

		int ret = telephony_sim_get_state(handle_list.handle[i], &state);
		if (ret != TELEPHONY_ERROR_NONE)
			continue;

		if (state == TELEPHONY_SIM_STATE_UNAVAILABLE)
			out = NULL;
		else {
			out = _sim_state_text_for_sim_get(handle_list.handle[i]);
		}

		free(sim_plmn[i]);
		sim_plmn[i] = out;
	}

	if (emit) ecore_event_add(LOCKSCREEN_EVENT_SIM_STATUS_CHANGED, NULL, NULL, NULL);
}

static void _on_telephony_state_changed_cb(telephony_state_e state, void *user_data)
{
	_update_sim_info(true);
}

static void _on_sim_info_changed_cb(telephony_h handle, telephony_noti_e noti_id, void *data, void *user_data)
{
	_update_sim_info(true);
}

int lockscreen_sim_init(void)
{
	int i;
	if (!init_count) {
		LOCKSCREEN_EVENT_SIM_STATUS_CHANGED = ecore_event_type_new();
		int ret = telephony_init(&handle_list);
		if (ret != TELEPHONY_ERROR_NONE)
		{
			ERR("telephony_init failed: %s", get_error_message(ret));
			return -1;
		}

		ret = telephony_set_state_changed_cb(_on_telephony_state_changed_cb, NULL);
		if (ret != TELEPHONY_ERROR_NONE) {
			telephony_deinit(&handle_list);
			ERR("telephony_set_state_changed_cb failed: %s", get_error_message(ret));
			return -1;
		}

		for (i = 0; (i < handle_list.count) && (i < LOCKSCREEN_SIM_MAX); i++)
		{
			int j;
			for (j = 0; j < SIZE(notis); j++)
			{
				ret = telephony_set_noti_cb(handle_list.handle[i], notis[j], _on_sim_info_changed_cb, NULL);
				if (ret != TELEPHONY_ERROR_NONE) {
					ERR("telephony_set_noti_cb failed: %s", get_error_message(ret));
				}
			}
		}

		_update_sim_info(false);
	}
	init_count++;
	return 0;
}

void lockscreen_sim_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count) {
			int i;
			for (i = 0; i < LOCKSCREEN_SIM_MAX; i++) {
				free(sim_plmn[i]);
				sim_plmn[i] = NULL;
			}
			telephony_deinit(&handle_list);
			telephony_unset_state_changed_cb(_on_telephony_state_changed_cb);
		}
	}
}

const char *lockscreen_sim_get_plmn(lockscreen_sim_num_e num)
{
	return sim_plmn[num];
}
