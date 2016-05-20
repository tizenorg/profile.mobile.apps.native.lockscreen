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

#define MAX_DEVICE_SIM_SLOT_NUM 2

static int init_count;

typedef struct {
	char *cp_name;
	TapiHandle *handle;
	TelSimCardStatus_t status;
	int card_changed;
	int attempts_left;
} sim_card_info_t;

static sim_card_info_t sim_card[MAX_DEVICE_SIM_SLOT_NUM];

static struct {
	int slot_count;
	int card_count;

	int card_no;
} context = {
	.card_no = -1,
};

int LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT;
int LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED;
int LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED;

static void _sim_lock_response_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSimSecResult_t *result_data = data;
	int card_no = context.card_no;

	if (!result_data) {
		ERR("No cb data");
		return;
	}

	sim_card[card_no].attempts_left = result_data->retry_count;

	switch (result) {
	case TAPI_SIM_PIN_OPERATION_SUCCESS:
		sim_card[card_no].status = TAPI_SIM_STATUS_SIM_INIT_COMPLETED;
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED, NULL, NULL, NULL);
		break;
	case TAPI_SIM_PIN_INCORRECT_PASSWORD:
	case TAPI_SIM_PUK_INCORRECT_PASSWORD:
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT, NULL, NULL, NULL);
		break;
	case TAPI_SIM_PERM_BLOCKED:
	case TAPI_SIM_PUK_REQUIRED:
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED, NULL, NULL, NULL);
		break;
	default:
		ERR("Incorrect data");
		break;
	}
}

static void _card_status_changed_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	sim_card_info_t *sim_card = user_data;

	if (!strcmp(noti_id, TAPI_NOTI_SIM_STATUS))
		sim_card->status = *(TelSimCardStatus_t *)data;
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

static void _sim_lock_deinit(void)
{
	int i;
	for (i = 0; i < MAX_DEVICE_SIM_SLOT_NUM; ++i) {

		if (sim_card[i].handle) {
			tel_deregister_noti_event(sim_card[i].handle, TAPI_NOTI_SIM_STATUS);
			tel_deinit(sim_card[i].handle);
		}

		free(sim_card[i].cp_name);
	}
}

static pin_type_e _lockscreen_sim_lock_tapi_to_pin_type(TelSimCardStatus_t status)
{
	switch (status) {
	case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:
		return SIM_LOCK_PIN_TYPE_PIN;
	case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:
		return SIM_LOCK_PIN_TYPE_PUK;
	case TAPI_SIM_STATUS_CARD_BLOCKED:
		return SIM_LOCK_PIN_TYPE_CARD_BLOCKED;
	default:
		return SIM_LOCK_PIN_TYPE_NONE;
	}
}

int lockscreen_sim_lock_get_attempts_left(void)
{
	if (context.card_no >= 0)
		return sim_card[context.card_no].attempts_left;
	else
		return -1;
}

void lockscreen_sim_lock_unlock(sim_lock_e card_no, const char *pass)
{
	int ret = 0;
	TelSimSecPw_t pin_data;

	context.card_no = card_no;

	pin_data.pw = (unsigned char *)pass;
	pin_data.pw_len = strlen(pass);

	switch (sim_card[card_no].status) {
	case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:
		pin_data.type = TAPI_SIM_PTYPE_PIN1;
		ret = tel_verify_sim_pins(sim_card[card_no].handle, &pin_data,
				_sim_lock_response_cb, NULL);
		break;
	case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:
	case TAPI_SIM_STATUS_CARD_BLOCKED:
		ecore_event_add(LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED, NULL, NULL, NULL);
		break;
	default:
		ERR("Not handled sim status: %d", sim_card[card_no].status);
	}

	if (ret != TAPI_API_SUCCESS)
		ERR("tel verification failed: %d msg: %s", ret, get_error_message(ret));
}

int lockscreen_sim_lock_pin_required(sim_lock_e *sim, pin_type_e *type)
{
	int result = 0;
	int i;
	*sim = SIM_LOCK_NONE;
	*type = SIM_LOCK_PIN_TYPE_NONE;

	for (i = 0; i < context.slot_count && i < MAX_DEVICE_SIM_SLOT_NUM; ++i) {

		if (sim_card[i].status ==  TAPI_SIM_STATUS_SIM_PIN_REQUIRED ||
				sim_card[i].status == TAPI_SIM_STATUS_SIM_PUK_REQUIRED) {

			++result;

			if (*sim == SIM_LOCK_NONE) {
				*sim = i;
				*type = _lockscreen_sim_lock_tapi_to_pin_type(sim_card[i].status);
			}
		}
	}

	return result;
}

int lockscreen_sim_lock_available_sim_card_count(void)
{
	return context.card_count;
}

int lockscreen_sim_lock_init(void)
{
	int slot_count = 0;
	int card_count = 0;

	if (!init_count) {
		char **cp_name_list;

		cp_name_list = tel_get_cp_name_list();
		if (!cp_name_list) {
			ERR("tel_get_cp_name_list failed");
			return -1;
		}

		int i = 0;
		for (i = 0; i < MAX_DEVICE_SIM_SLOT_NUM; ++i) {

			int ret = 0;

			if (!cp_name_list[i])
				break;

			slot_count++;

			sim_card[i].handle = tel_init(cp_name_list[i]);
			if (!sim_card[i].handle) {
				ERR("tel_init failed: %s", get_error_message(ret));

				_sim_lock_deinit();
				_cp_name_list_free(cp_name_list);

				return -1;
			}

			ret = tel_get_sim_init_info(sim_card[i].handle, &sim_card[i].status,
					&sim_card[i].card_changed);
			if (ret != TAPI_API_SUCCESS) {
				ERR("tel_get_sim_init_info failed: %s", get_error_message(ret));

				_sim_lock_deinit();
				_cp_name_list_free(cp_name_list);

				return -1;
			}

			ret = tel_register_noti_event(sim_card[i].handle, TAPI_NOTI_SIM_STATUS,
					_card_status_changed_cb, &sim_card[i]);
			if (ret != TAPI_API_SUCCESS) {
				ERR("tel_register_noti_event failed: %s", get_error_message(ret));

				_sim_lock_deinit();
				_cp_name_list_free(cp_name_list);

				return -1;
			}

			sim_card[i].cp_name = strdup(cp_name_list[i]);

			if (sim_card[i].status != TAPI_SIM_STATUS_CARD_NOT_PRESENT &&
					sim_card[i].status != TAPI_SIM_STATUS_CARD_REMOVED)
				card_count++;

			DBG("SIM%d status: %d", i, sim_card[i].status);
		}

		_cp_name_list_free(cp_name_list);

		LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT = ecore_event_type_new();
		LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED = ecore_event_type_new();
		LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED = ecore_event_type_new();
	}

	context.slot_count = slot_count;
	context.card_count = card_count;

	init_count++;
	return 0;
}

void lockscreen_sim_lock_shutdown(void)
{
	if (init_count) {
		--init_count;

		_sim_lock_deinit();
	}
}
