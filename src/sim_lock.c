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
	DBG("Unlock response cb result: %d", result);

	TelSimSecResult_t *result_data = data;
	int card_num = (int)user_data;

	if (!result_data) {
		ERR("No cb data");
		return;
	}

	remaining_attempts = result_data->retry_count;

	DBG("Remaining attempts: %d", remaining_attempts);

	switch (result) {
	case TAPI_SIM_PIN_OPERATION_SUCCESS:
		sim_card[card_num].status = TAPI_SIM_STATUS_SIM_INIT_COMPLETED;
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED, (void *)card_num, NULL, NULL);
		break;
	case TAPI_SIM_PIN_INCORRECT_PASSWORD:
	case TAPI_SIM_PUK_INCORRECT_PASSWORD:
		ERR("Wrong pin");
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT, (void *)card_num, NULL, NULL);
	case TAPI_SIM_PERM_BLOCKED:
		ERR("Sim is blocked");
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED, (void *)card_num, NULL, NULL);
	default:
		ERR("Incorrect data");
		break;
	}
}

static void _card_status_changed_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	sim_card_info_t *sim_card = user_data;

	if (!strcmp(noti_id, TAPI_NOTI_SIM_STATUS)) {
		DBG("card status changed: %d -> %d", sim_card->status, *(int *)data);
		DBG("Sim pointer: %p", &sim_card);
		sim_card->status = *(TelSimCardStatus_t *)data;
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

void lockscreen_sim_lock_unlock(int card_num, const char *pass)
{
	int ret = 0;
	TelSimSecPw_t pin_data;

	pin_data.pw = (unsigned char *)pass;
	pin_data.pw_len = strlen(pass);

	DBG("Sim card status: %d", sim_card[card_num].status);

	/*for now only 1 card*/
	switch (sim_card[card_num].status) {
	case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:
		DBG("Unlock pin");
		pin_data.type = TAPI_SIM_PTYPE_PIN1;
		ret = tel_verify_sim_pins(sim_card[card_num].handle, &pin_data,
				_sim_lock_response_cb, (void *)card_num);
		break;
	case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:
		pin_data.type = TAPI_SIM_PTYPE_PUK1;
		ret = tel_verify_sim_puks(sim_card[card_num].handle, &pin_data, NULL,
				_sim_lock_response_cb, (void *)card_num);
		break;
	case TAPI_SIM_STATUS_CARD_BLOCKED:
		ERR("Sim card is blocked");
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED, NULL, NULL, NULL);
		break;
	default:
		ERR("Not handled sim status: %d", sim_card[0].status);
	}

	DBG("Pin data pass: %s", pin_data.pw);
	DBG("Pin data pass len: %d", pin_data.pw_len);
	DBG("Pin data type: %d", pin_data.type);

	DBG("tel verify result: %d", ret);
}

//lockscreen_sim_lock_get_first_locked()
int lockscreen_sim_lock_pin_required(int *locked_num)
{
	int result = 0;
	int i;

	*locked_num = -1;

	for (i = 0; i < slot_count; ++i) {
		if (sim_card[i].status == TAPI_SIM_STATUS_SIM_PIN_REQUIRED
				|| sim_card[i].status == TAPI_SIM_STATUS_SIM_PUK_REQUIRED) {
			result++;

			if (*locked_num < 0)
				*locked_num = i;
		}
	}

	return result;
}

int lockscreen_sim_lock_init(void)
{
	if (!init_count) {

		char **cp_name_list;
		slot_count = 0;

		cp_name_list = tel_get_cp_name_list();
		if (!cp_name_list) {
			ERR("tel_get_cp_name_list failed");
			return -1;
		}

		while (cp_name_list[slot_count]) {
			sim_card = realloc(sim_card, (slot_count + 1) * sizeof(sim_card_info_t));
			++slot_count;
		}

		int i = 0;
		for (i = 0; i < slot_count; ++i) {

			int ret = 0;

			sim_card[i].handle = tel_init(cp_name_list[i]);
			if (!sim_card[i].handle) {
				ERR("tel_init failed: %s", get_error_message(ret));
				return -1;
			}

			ret = tel_get_sim_init_info(sim_card[i].handle, &sim_card[i].status,
					&sim_card[i].card_changed);
			if (ret != TAPI_API_SUCCESS) {
				ERR("tel_get_sim_init_info failed: %s", get_error_message(ret));
				tel_deinit(sim_card[i].handle);
				_cp_name_list_free(cp_name_list);
				return -1;
			}

			ret = tel_register_noti_event(sim_card[i].handle, TAPI_NOTI_SIM_STATUS,
					_card_status_changed_cb, &sim_card[i]);
			if (ret != TAPI_API_SUCCESS) {
				ERR("tel_register_noti_event failed: %s", get_error_message(ret));
				tel_deinit(sim_card[i].handle);
				_cp_name_list_free(cp_name_list);
				return -1;
			}

			sim_card[i].cp_name = strdup(cp_name_list[i]);

			if (sim_card[i].status != TAPI_SIM_STATUS_CARD_NOT_PRESENT &&
					sim_card[i].status != TAPI_SIM_STATUS_CARD_REMOVED)
				inserted_count++;

			DBG("SIM%d status: %d", i, sim_card[i].status);
			DBG("SIM pointer: %p", &sim_card[i]);
			DBG("SIM handle: %p", sim_card[i].handle);
		}


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
	if (init_count) {
		--init_count;

		int i;
		for (i = 0; i < inserted_count; ++i) {

			tel_deregister_noti_event(sim_card[i].handle, TAPI_NOTI_SIM_STATUS);
			tel_deinit(sim_card[i].handle);

			free(sim_card[i].cp_name);
		}

		free(sim_card);
	}
}
