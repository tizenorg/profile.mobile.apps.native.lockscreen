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
#include "sim_lock.h"

#include <telephony-client/tapi_common.h>
#include <telephony-client/ITapiSim.h>

typedef struct {
	char *cp_name;
	TapiHandle *handle;
	TelSimCardStatus_t status;
	int card_changed;
} sim_card_info_t;

static sim_card_info_t *sim_card;

static int slot_count;
static int inserted_count;
static int remaining_attempts;
static int init_count;

int LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT;
int LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED;
int LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED;

static void _sim_lock_response_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimSecResult_t *result_data = data;

	remaining_attempts = result_data->retry_count;

	if (result)
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED, NULL, NULL, NULL);
	else {
		ERR("Wrong pin");
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT, NULL, NULL, NULL);
	}
}

static void _cp_name_list_free(char **list)
{

	int i = 0;
	while(list[i]) {
		free(list[i]);
		++i;
	}
	free(list);
}

void lockscreen_sim_lock_unlock(const char *pass)
{
	DBG("Unlock with pass: %s", pass);

	TelSimSecPw_t pin_data;
	pin_data.pw = (unsigned char *)pass;
	pin_data.pw_len = strlen(pass);

	DBG("Sim card status: %d", sim_card[0].status);

	/*for now only 1 card*/
	switch (sim_card[0].status) {
	case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:
		pin_data.type = TAPI_SIM_PTYPE_PIN1;
		tel_verify_sim_pins(sim_card[0].handle, &pin_data, _sim_lock_response_cb, NULL);
		break;
	case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:
		pin_data.type = TAPI_SIM_PTYPE_PUK1;
		tel_verify_sim_puks(sim_card[0].handle, &pin_data, NULL, _sim_lock_response_cb, NULL);
		break;
	case TAPI_SIM_STATUS_CARD_BLOCKED:
		ERR("Sim card is blocked");
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED, NULL, NULL, NULL);
		break;
	default:
		ERR("Not handled sim status: %d", sim_card[0].status);
	}

}

int lockscreen_sim_lock_locked_count_get(void)
{
	DBG("Sim lock get locked count");
	int count = 0;

	int i;
	for (i = 0; i < inserted_count; ++i)
		if (sim_card[i].status == TAPI_SIM_STATUS_SIM_PIN_REQUIRED
				|| sim_card[i].status == TAPI_SIM_STATUS_SIM_PUK_REQUIRED)
			count++;

	return count;
}

int lockscreen_sim_lock_pin_required(void)
{
	int result = 0;
	int i;

	for (i = 0; i < slot_count; ++i) {
		if (sim_card[i].status == TAPI_SIM_STATUS_SIM_PIN_REQUIRED
				|| sim_card[i].status == TAPI_SIM_STATUS_SIM_PUK_REQUIRED)
			result++;
	}

	return result;
}

int lockscreen_sim_lock_init(void)
{
	if(!init_count) {
		char **cp_name_list;
		int i = 0;

		cp_name_list = tel_get_cp_name_list();
		if (!cp_name_list) {
			ERR("tel_get_cp_name_list failed");
			return -1;
		}

		while (cp_name_list[i]) {

			sim_card = realloc(sim_card, (i + 1) * sizeof(sim_card_info_t));
			sim_card[i].handle = tel_init(cp_name_list[i]);
			int ret = tel_get_sim_init_info(sim_card[i].handle, &sim_card[i].status, &sim_card[i].card_changed);
			if (ret != TAPI_API_SUCCESS) {
				ERR("tel_get_sim_init_info failed: %s", get_error_message(ret));
				_cp_name_list_free(cp_name_list);
				return -1;
			}

			sim_card[i].cp_name = strdup(cp_name_list[i]);

			if (sim_card[i].status != TAPI_SIM_STATUS_CARD_NOT_PRESENT &&
					sim_card[i].status != TAPI_SIM_STATUS_CARD_REMOVED)
				inserted_count++;

			++i;
		}

		slot_count = i;

		DBG("Inserted cards count: %d", inserted_count);
		DBG("SIM slot count: %d", slot_count);

		_cp_name_list_free(cp_name_list);
	}

	LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT = ecore_event_type_new();
	LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED = ecore_event_type_new();
	LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED = ecore_event_type_new();

	init_count++;
	return 0;
}

void lockscreen_sim_lock_shutdown(void)
{
	int i;
	for (i = 0; i < inserted_count; ++i) {

		tel_deinit(sim_card[i].handle);
		free(sim_card[i].cp_name);
	}
	free(sim_card);
}
