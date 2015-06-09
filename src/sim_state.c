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

#include <tapi_common.h>
#include <ITapiSim.h>
#include <TelCall.h>
#include <ITapiSim.h>
#include <TelNetwork.h>

#if TIZEN_BUILD_TARGET
//#include <telephony_network.h>
#endif

#include "lockscreen.h"
#include "log.h"
#include "sim_state.h"
#include "property.h"
#include "default_lock.h"

#define NO_SIM_LEN 8
#define PLMN_LENGTH 6
#define PLMN_SPN_LENGTH 32
#define EMG_BUTTON_WIDTH 322

#define TAPI_HANDLE_MAX  2

#define PLMN_LABEL_STYLE_START "<style=far_shadow,bottom><shadow_color=#00000033><font_size=24><align=left><color=#FFFFFF><text_class=ATO007><color_class=ATO007><wrap=none>"
#define PLMN_LABEL_STYLE_END "</wrap></color_class></text_class></color></align></font_size></shadow_color></style>"

static struct _s_info {
	TapiHandle *handle[TAPI_HANDLE_MAX+1];
	Eina_Bool sim_card_ready[2];

	Evas_Object *layout;
	Evas_Object *operator_name;

	int call_state;	// 0:none, 1:call
} s_info = {
	.handle[0] = NULL,
	.handle[1] = NULL,
	.handle[2] = NULL,
	.sim_card_ready[0] = EINA_FALSE,
	.sim_card_ready[1] = EINA_FALSE,
	.layout = NULL,
	.operator_name = NULL,
	.call_state = 0,
};

static void _sim_callback_register(void);
static void _sim_callback_unregister(void);

static void _operator_name_slide_mode_set(Evas_Object *label)
{
	Evas_Object *label_edje = NULL;
	Evas_Object *tb = NULL;
	Evas_Coord tb_w = 0;

	ret_if(!label);

	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);

	label_edje = elm_layout_edje_get(label);
	ret_if(!label_edje);

	tb = (Evas_Object*)edje_object_part_object_get(label_edje, "elm.text");
	ret_if(!tb);

	evas_object_textblock_size_native_get(tb, &tb_w, NULL);

	if ((tb_w > 0) && (tb_w > _X(EMG_BUTTON_WIDTH))) {
		elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	}

	elm_label_slide_go(label);
}

static void _operator_sliding_label_create(Evas_Object *layout, char *text)
{
	Evas_Object *label = NULL;
	char buf[512] = { 0, };
	char *markup_text = NULL;

	label = elm_label_add(layout);
	ret_if(!label);

	markup_text = elm_entry_utf8_to_markup(text);
	snprintf(buf, sizeof(buf), "%s%s%s", PLMN_LABEL_STYLE_START, markup_text, PLMN_LABEL_STYLE_END);
	free(markup_text);

	elm_object_style_set(label, "slide_short");
	elm_label_wrap_width_set(label, 100);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_label_slide_duration_set(label, 2);
	_operator_name_slide_mode_set(label);

	elm_object_text_set(label, buf);

	elm_object_part_content_set(layout, "txt.plmn", label);
	evas_object_show(label);

	s_info.operator_name = label;
}

static int _sim_controller_get_call_state(void)
{
	int value = 0;
	int ret = 0;

	ret = lock_property_get_int(PROPERTY_TYPE_VCONFKEY, VCONFKEY_CALL_STATE, &value);
	retv_if(ret != LOCK_ERROR_OK, 0);

	if (value == VCONFKEY_CALL_OFF) {
		_E("Call is OFF");
		return 0;
	}

	_D("Call status[%d]", value);

	return 1;
}

static void _sim_controller_call_state_changed_cb(keynode_t *key, void *data)
{
	int call_state = _sim_controller_get_call_state();

	if (s_info.call_state != call_state) {
		_D("Call state changed[%d]", call_state);
		s_info.call_state = call_state;
	}
}

static char *_sim_plmn_get(TapiHandle *handle)
{
	int ret = 0;
	char *network_name = NULL;

	/* Reading Network (PLMN) name - ‘string’ type Property */
	ret = tel_get_property_string(handle, TAPI_PROP_NETWORK_NETWORK_NAME, &network_name);
	if (ret == TAPI_API_SUCCESS) {
		/* ‘network_name’ contains valid Network name based on Display condition */
		return network_name;
	} else {
		_E("Sim = %p PLMN = ERROR[%d]", handle, ret);
		/* get property failed */
	}

	return NULL;
}

static char *_sim_spn_get(TapiHandle *handle)
{
	int ret = 0;
	char *spn_name = NULL;

	/* Reading SPN name - ‘string’ type Property */
	ret = tel_get_property_string(handle, TAPI_PROP_NETWORK_SPN_NAME, &spn_name);
	if (ret == TAPI_API_SUCCESS) {
		/* ‘spn_name’ contains valid Service provider name */
		return spn_name;
	} else {
		_E("Sim = %p SPN = ERROR[%d]", handle, ret);
		/* get property failed */
		return NULL;
	}
}

static char *_plmn_spn_network_get(int handle_num, TapiHandle *handle)
{
	int ret = TAPI_API_SUCCESS;
	int service_type = TAPI_NETWORK_SERVICE_TYPE_UNKNOWN;
	int name_option = TAPI_NETWORK_DISP_INVALID;
	char *plmn = NULL;
	char *spn = NULL;
	char buf[1024] = { 0, };

	// get service type
	ret = tel_get_property_int(handle, TAPI_PROP_NETWORK_SERVICE_TYPE, &service_type);
	if (ret != TAPI_API_SUCCESS) {
		_E("Failed to get service type[%d]", ret);
	}

	if (service_type >= TAPI_NETWORK_SERVICE_TYPE_2G) {
		// get network name option
		ret = tel_get_property_int(handle, TAPI_PROP_NETWORK_NAME_OPTION, &name_option);
		if (ret != TAPI_API_SUCCESS) {
			_E("Failed to get name option[%d]", ret);
		}

		switch (name_option) {
		case TAPI_NETWORK_DISP_SPN:
			spn = _sim_spn_get(handle);
			if (spn != NULL && spn[0] != 0) {
				_I("PLMN/SPN - Sim %p using SPN: %s", handle, spn);
				snprintf(buf, sizeof(buf), "%s", spn);
			}
			break;
		case TAPI_NETWORK_DISP_PLMN:
			plmn = _sim_plmn_get(handle);
			if (plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		case TAPI_NETWORK_DISP_SPN_PLMN:
			spn = _sim_spn_get(handle);
			plmn = _sim_plmn_get(handle);
			if (spn != NULL && spn[0] != 0 && plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using SPN: %s, PLMN: %s", handle, spn, plmn);
				snprintf(buf, sizeof(buf), "%s - %s", plmn, spn);
			} else if (spn != NULL && spn[0] != 0) {
				_I("PLMN/SPN - Sim %p using SPN: %s", handle, spn);
				snprintf(buf, sizeof(buf), "%s", spn);
			} else if (plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		default:
			_E("Invalid name option[%d]", name_option);
			plmn = _sim_plmn_get(handle);
			if (plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		}
	} else {
		switch (service_type) {
		case TAPI_NETWORK_SERVICE_TYPE_NO_SERVICE:
			snprintf(buf, sizeof(buf), "%s", _("IDS_COM_BODY_NO_SERVICE"));
			break;
		case TAPI_NETWORK_SERVICE_TYPE_EMERGENCY:
			snprintf(buf, sizeof(buf), "%s", _("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"));
			break;
		case TAPI_NETWORK_SERVICE_TYPE_SEARCH:
			snprintf(buf, sizeof(buf), "%s", _("IDS_COM_BODY_SEARCHING"));
			break;
		default:
			_E("invalid service type[%d]", service_type);
			plmn = _sim_plmn_get(handle);
			if (plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		}
	}

	_D("handle[%d][%p] service_type[%d], name_option[%d] >> [%s]", handle_num, handle, service_type, name_option, buf);

	if (strlen(buf) == 0) {
		_E("Empty string");
		snprintf(buf, sizeof(buf), "%s", _("IDS_COM_BODY_NO_SERVICE"));
	} else if (strncasecmp(buf, "No Service", strlen("No Service")) == 0) {
		_E("USING SPECIAL NETWORK NAME:  %s in handle: %d", _("IDS_COM_BODY_NO_SERVICE"), handle_num);
		return strdup(_("IDS_COM_BODY_NO_SERVICE"));
	} else if (strncasecmp(buf, "EMERGENCY", strlen("EMERGENCY")) == 0) {
		_E("USING SPECIAL NETWORK NAME:  %s in handle: %d", _("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"), handle_num);
		return strdup(_("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"));
	} else if (strncasecmp(buf, "Searching", strlen("Searching")) == 0) {
		_E("USING SPECIAL NETWORK NAME:  %s in handle: %d", _("IDS_COM_BODY_SEARCHING"), handle_num);
		return strdup(_("IDS_COM_BODY_SEARCHING"));
	} else if (strncasecmp(buf, "SIM _Eor", strlen("SIM Error")) == 0) {
		_E("USING SPECIAL NETWORK NAME:  %s in handle: %d", _("IDS_IDLE_BODY_INVALID_SIM_CARD"), handle_num);
		return strdup(_("IDS_IDLE_BODY_INVALID_SIM_CARD"));
	} else if (strncasecmp(buf, "NO SIM", strlen("NO SIM")) == 0) {
		_E("USING SPECIAL NETWORK NAME:  %s in handle: %d", _("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"), handle_num);
		return strdup(_("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"));
	}

	return strdup(buf);
}

static void _sim_status_print(TelSimCardStatus_t sim_status, int card_changed)
{
	switch(sim_status) {
	case TAPI_SIM_STATUS_CARD_ERROR:
		_I("Sim card status: TAPI_SIM_STATUS_CARD__EOR");
		break;
	case TAPI_SIM_STATUS_CARD_NOT_PRESENT:
		_I("Sim card status: TAPI_SIM_STATUS_CARD_NOT_PRESENT");
		break;
	case TAPI_SIM_STATUS_SIM_INITIALIZING:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_INITIALIZING");
		break;
	case TAPI_SIM_STATUS_SIM_INIT_COMPLETED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_INIT_COMPLETED");
		break;
	case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_PIN_REQUIRED");
		break;
	case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_PUK_REQUIRED");
		break;
	case TAPI_SIM_STATUS_CARD_BLOCKED:
		_I("Sim card status: TAPI_SIM_STATUS_CARD_BLOCKED");
		break;
	case TAPI_SIM_STATUS_SIM_NCK_REQUIRED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_NCK_REQUIRED");
		break;
	case TAPI_SIM_STATUS_SIM_NSCK_REQUIRED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_NSCK_REQUIRED");
		break;
	case TAPI_SIM_STATUS_SIM_SPCK_REQUIRED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_SPCK_REQUIRED");
		break;
	case TAPI_SIM_STATUS_SIM_CCK_REQUIRED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_CCK_REQUIRED");
		break;
	case TAPI_SIM_STATUS_CARD_REMOVED:
		_I("Sim card status: TAPI_SIM_STATUS_CARD_REMOVED");
		break;
	case TAPI_SIM_STATUS_SIM_LOCK_REQUIRED:
		_I("Sim card status: TAPI_SIM_STATUS_SIM_LOCK_REQUIRED");
		break;
	case TAPI_SIM_STATUS_CARD_CRASHED:
		_I("Sim card status: TAPI_SIM_STATUS_CARD_CRASHED");
		break;
	case TAPI_SIM_STATUS_CARD_POWEROFF:
		_I("Sim card status: TAPI_SIM_STATUS_CARD_POWEROFF");
		break;
	case TAPI_SIM_STATUS_UNKNOWN:
		_I("Sim card status: TAPI_SIM_STATUS_UNKNOWN");
		break;
	}

	_I("Sim_card_changed: %d", card_changed);
}

static void _sim_status_get(void)
{
	int i = 0;
	int ret = 0;
	TelSimCardStatus_t sim_status;
	int card_changed = 0;

	for (i = 0; i < TAPI_HANDLE_MAX + 1; ++i) {
		if (s_info.handle[i]) {
			ret = tel_get_sim_init_info (s_info.handle[i], &sim_status, &card_changed);
			if (ret == 0) {
				_sim_status_print(sim_status, card_changed);

				if (sim_status == TAPI_SIM_STATUS_SIM_INIT_COMPLETED || sim_status == TAPI_SIM_STATUS_SIM_PIN_REQUIRED) {
					if (i < TAPI_HANDLE_MAX) {
						s_info.sim_card_ready[i] = EINA_TRUE;
					}
				} else {
					_E("SIM[%d] is not completed initialization [%d]", i, sim_status);
				}
			} else {
				_E("Could not get sim[%d] status[%d]", i, ret);
			}
		}
	}
}

static void _sim_state_text_set(Eina_Bool flight_mode)
{
	Evas_Object *swipe_layout = lock_default_swipe_layout_get();
	ret_if(!swipe_layout);

	if (flight_mode) {
		/* if flight mode, No service */
		_operator_sliding_label_create(swipe_layout, _("IDS_COM_BODY_NO_SERVICE"));
	} else if (s_info.sim_card_ready[0] && s_info.sim_card_ready[1]) {
		_operator_sliding_label_create(swipe_layout, "");
	} else if (s_info.sim_card_ready[0]) {
		char *plmn_spn1 = _plmn_spn_network_get(0, s_info.handle[0]);
		_operator_sliding_label_create(swipe_layout, plmn_spn1);
		free(plmn_spn1);
	} else if (s_info.sim_card_ready[1]) {
		char *plmn_spn1 = _plmn_spn_network_get(1, s_info.handle[1]);
		_operator_sliding_label_create(swipe_layout, plmn_spn1);
		free(plmn_spn1);
	} else {
		_operator_sliding_label_create(swipe_layout, _("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"));
	}
}

static void _view_init(void)
{
	int flight_mode_state = EINA_FALSE;
	int ret = 0;

	ret = lock_property_get_bool(PROPERTY_TYPE_VCONFKEY, VCONFKEY_TELEPHONY_FLIGHT_MODE, &flight_mode_state);
	if (ret != LOCK_ERROR_OK) {
		_E("Could not get 'VCONFKEY_TELEPHONY_FLIGHT_MODE' value");
	}

	_sim_state_text_set(flight_mode_state);
}

static void _tel_init(void)
{
	char **cp_list = NULL;
	unsigned int modem_num = 0;

	/* Get CP name list – cp_list */
	cp_list = tel_get_cp_name_list();
	ret_if(!cp_list);

	while (cp_list[modem_num]) {
		/* Initialize TAPI handle */
		s_info.handle[modem_num] = tel_init(cp_list[modem_num]);

		if (cp_list[modem_num]) {
			_E("s_info.handle[%d] = %s; ptr = %p", modem_num, cp_list[modem_num], s_info.handle[modem_num]);
		}

		/* Move to next CP Name in cp_list */
		modem_num++;
	}

	s_info.handle[modem_num] = NULL;

	/* free cp_list */
	free(cp_list);
}

static void _tel_deinit(void)
{
	int i = 0;
	while (s_info.handle[i]) {
		/* De-initialize TAPI handle */
		tel_deinit(s_info.handle[i]);
		s_info.handle[i] = NULL;

		/* Move to next handle */
		i++;
	}
}

static void _tel_ready_cb(keynode_t *key, void *data)
{
	Eina_Bool status = EINA_FALSE;

	status = vconf_keynode_get_bool(key);
	_D("tel status[%d]", status);

	if (status) {	/* Telephony State - READY */
		_tel_init();
		_sim_callback_register();
		_sim_status_get();

		_view_init();
	} else {   /* Telephony State – NOT READY */
		/* De-initialization is optional here (ONLY if required) */
		_tel_deinit();
		s_info.sim_card_ready[0] = EINA_FALSE;
		s_info.sim_card_ready[1] = EINA_FALSE;

		_sim_callback_unregister();
	}
}

static void _tel_flight_mode_cb(keynode_t *key, void *data)
{
	Eina_Bool flight_mode_state = EINA_FALSE;

	flight_mode_state = vconf_keynode_get_bool(key);
	_sim_state_text_set(flight_mode_state);
}

static void _on_sim_card_status_changed_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	int handle_num = (int)user_data;
	int *sim_status = data;

	_E("SIM[%p][%d] status[%d], [%d][%d]", handle, handle_num, *sim_status, s_info.sim_card_ready[0], s_info.sim_card_ready[1]);

	if (*sim_status == TAPI_SIM_STATUS_SIM_INIT_COMPLETED || *sim_status == TAPI_SIM_STATUS_SIM_PIN_REQUIRED) {
		s_info.sim_card_ready[handle_num] = EINA_TRUE;
	} else {
		s_info.sim_card_ready[handle_num] = EINA_FALSE;
	}

	_view_init();
}

static void _on_plmn_spn_changed_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	int  flight_mode_state = EINA_FALSE;
	int ret = 0;

	ret_if(!handle);

	ret = lock_property_get_bool(PROPERTY_TYPE_VCONFKEY, VCONFKEY_TELEPHONY_FLIGHT_MODE, &flight_mode_state);
	if (ret != LOCK_ERROR_OK) {
		_E("Could not get the 'VCONFKEY_TELEPHONY_FLIGHT_MODE' value");
	}

	_sim_state_text_set(flight_mode_state);
}

static void _sim_callback_register(void)
{
	int i = 0;
	int ret = 0;

	for (i = 0; i < TAPI_HANDLE_MAX; ++i) {
		if (s_info.handle[i]) {
			ret = tel_register_noti_event(s_info.handle[i], TAPI_NOTI_SIM_STATUS, _on_sim_card_status_changed_cb, (void*)i);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to register '_on_sim_card_status_changed_cb' callback to handle[%d][%d]", i, ret);
			} else {
				_E("SIM card status changed event registered");
			}

			ret = tel_register_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_SPN_NAME, _on_plmn_spn_changed_cb, (void*)i);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to register '_on_plmn_spn_changed_cb' callback to handle[%d][%d]", i, ret);
			}

			ret = tel_register_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_NETWORK_NAME, _on_plmn_spn_changed_cb, (void*)i);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to register '_on_plmn_spn_changed_cb' callback to handle: %i", i);
			}

			ret = tel_register_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_SERVICE_TYPE, _on_plmn_spn_changed_cb, (void*) i);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to register network service type[%d][%d]", ret, i);
			}

			ret = tel_register_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_NAME_OPTION, _on_plmn_spn_changed_cb, (void*) i);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to register network name option[%d][%d]", ret, i);
			}
		} else {
			_E("No handle [%d]", i);
		}
	}
}

static void _sim_callback_unregister(void)
{
	int i = 0;
	int ret = 0;
	for (i = 0; i < TAPI_HANDLE_MAX; ++i) {
		if (s_info.handle[i]) {
			ret = tel_deregister_noti_event(s_info.handle[i], TAPI_NOTI_SIM_STATUS);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to dereregister TAPI_NOTI_SIM_STATUS callback from handle: %i", i);
			} else {
				_D("SIM status changed event deregistered");
			}

			ret = tel_deregister_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_NETWORK_NAME);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to dereregister TAPI_PROP_NETWORK_PLMN callback from handle: %i", i);
			}

			ret = tel_deregister_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_SPN_NAME);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to dereregister TAPI_PROP_NETWORK_SPN_NAME callback from handle: %i", i);
			}

			ret = tel_deregister_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_SERVICE_TYPE);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to deregister network service type[%d][%d]", ret, i);
			}

			ret = tel_deregister_noti_event(s_info.handle[i], TAPI_PROP_NETWORK_NAME_OPTION);
			if (ret != TAPI_API_SUCCESS) {
				_E("Failed to deregister network name option[%d][%d]", ret, i);
			}

			if (i == 0) {
				ret = tel_deregister_noti_event(s_info.handle[i], TAPI_NOTI_CALL_PREFERRED_VOICE_SUBSCRIPTION);
				if (ret != TAPI_API_SUCCESS) {
					_E("Failed to dereregister  callback to handle: %d", i);
				}
			}
		}
	}
}


lock_error_e lock_sim_state_init(void)
{
	int state = EINA_FALSE;
	int ret = 0;
	/* Check if Telephony state - READY */
	ret = lock_property_get_bool(PROPERTY_TYPE_VCONFKEY, VCONFKEY_TELEPHONY_READY, &state);

	_D("Telephony Ready : %d", state);

	if (ret == LOCK_ERROR_OK && state == EINA_TRUE) {
		/* Telephony State - READY */
		/* Initialize TAPI handles */

		_tel_init();
		_sim_callback_register();
		_sim_status_get();

		_view_init();
	} else {
		/* Telephony State – NOT READY, register for change in state */
		_D("Telephony state: [NOT Ready]");
	}

	/* Register for Telephony state change */
	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_READY, _tel_ready_cb, NULL);
	if (ret != 0) {
		_E("Failed to register VCONFKEY_TELEPHONY_READY key changed callback");
	}

	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_FLIGHT_MODE, _tel_flight_mode_cb, NULL);
	if (ret != 0) {
		_E("Failed to register VCONFKEY_TELEPHONY_FLIGHT_MODE key changed callback");
	}

	ret = vconf_notify_key_changed(VCONFKEY_CALL_STATE, _sim_controller_call_state_changed_cb, NULL);
	if (ret != 0) {
		_E("Failed to notify call state[%d]", ret);
	}

	return LOCK_ERROR_OK;
}

void lock_sim_state_resume(void)
{
	int state = FALSE;
	int ret = 0;
	int i = 0;
	TelSimCardStatus_t sim_status;
	int card_changed = 0;

	ret = lock_property_get_bool(PROPERTY_TYPE_VCONFKEY, VCONFKEY_TELEPHONY_READY, &state);
	if (ret != LOCK_ERROR_OK || state == FALSE) {
		_E("Failed to get telephony state[%d][%d]", state, ret);
		return;
	}

	for (i = 0; i < TAPI_HANDLE_MAX; ++i) {
		if (s_info.handle[i]) {
			ret = tel_get_sim_init_info(s_info.handle[i], &sim_status, &card_changed);
			_D("SIM[%d] info[%d][%d][%d]", i, ret, sim_status, card_changed);
			if (sim_status == TAPI_SIM_STATUS_SIM_INIT_COMPLETED || sim_status == TAPI_SIM_STATUS_SIM_PIN_REQUIRED) {
				if (s_info.sim_card_ready[i] != EINA_TRUE) {
					_E("SIM[%d] is init completed but local value is not ture", i);
				}
			}
		} else {
			_E("No handle[%d]", i);
		}
	}
}

void lock_sim_state_deinit(void)
{
	_D("De-initialization");
	_tel_deinit();
	s_info.sim_card_ready[0] = EINA_FALSE;
	s_info.sim_card_ready[1] = EINA_FALSE;

	_sim_callback_unregister();
}

void lock_sim_state_language_change(void)
{
	_on_plmn_spn_changed_cb(s_info.handle[0], "SELF", NULL, (void*) 0);
	_on_plmn_spn_changed_cb(s_info.handle[1], "SELF", NULL, (void*) 1);

	if (s_info.handle[0] == NULL && s_info.handle[1] == NULL) {
		int flight_mode = EINA_FALSE;
		int ret = lock_property_get_bool(PROPERTY_TYPE_VCONFKEY, VCONFKEY_TELEPHONY_FLIGHT_MODE, &flight_mode);
		if (ret != 0) {
			_E("Failed to get flight mode[%d]", ret);
		}

		_sim_state_text_set(flight_mode);
	}
}
