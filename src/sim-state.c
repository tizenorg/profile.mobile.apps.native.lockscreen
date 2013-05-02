/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vconf.h>
#include <vconf-keys.h>

#include "lockscreen.h"
#include "sim-state.h"

#define TELEPHONY_MAX_KEY_NUM 4
#define BUFFER_LENGTH 256

static int _sim_state_value_get(const char *key, char *txt, int size)
{
	int i = 0;
	int len = 0;
	char *str = NULL;

	str = vconf_get_str(key);
	if (str == NULL || str[0] == '\0'){
		return 0;
	}

	/* check ASCII code */
	for (i = strlen(str) - 1; i >= 0; i--) {
		if (str[i] <= 31 || str[i] >= 127) {
			if (str){
				free(str);
				str = NULL;
			}
			return 0;
		}
	}

	len = snprintf(txt, size, "%s", str);
	if (str){
		free(str);
		str = NULL;
	}

	return len;
}

static void _sim_state_string_get(char *keylist[], char *sim_state_string)
{
	char txt[BUFFER_LENGTH] = {0};
	char buf[BUFFER_LENGTH] = {0};
	int len = 0;
	int i = 0;

	for (i = 0; keylist[i]; i++) {
		if (_sim_state_value_get(keylist[i], buf, sizeof(buf))) {
			LOCK_SCREEN_TRACE_DBG("VCONFKEY(%s) = %s", keylist[i], buf);
			len = strlen(txt);
			snprintf(&txt[len], sizeof(txt) - len, "%s%s", len ? " - " : "", buf);
		}
		if (keylist[i]) {
			free(keylist[i]);
			keylist[i] = NULL;
		}
	}
	snprintf(sim_state_string, BUFFER_LENGTH, "%s", txt);
}

static void _set_sim_state(void *data)
{
	struct appdata *ad = data;
	if (ad == NULL)
		return;

	int state = 0;
	int ret = 0;
	int i = 0;
	char buf[BUFFER_LENGTH] = {0};
	char *keylist[TELEPHONY_MAX_KEY_NUM] = {0};

	int service_type = VCONFKEY_TELEPHONY_SVCTYPE_SEARCH;

	if(vconf_get_int(VCONFKEY_TELEPHONY_SVCTYPE, &service_type) != 0) {
		LOCK_SCREEN_TRACE_ERR("fail to get VCONFKEY_TELEPHONY_SVCTYPE");
	}

	ret = (vconf_get_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, &state));
	if (ret == 0) {
		LOCK_SCREEN_TRACE_DBG("[%s:%d] VCONFKEY(%s) = %d", __func__, __LINE__, VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, state);
		if (state != VCONFKEY_TELEPHONY_DISP_INVALID && service_type > VCONFKEY_TELEPHONY_SVCTYPE_SEARCH) {
			if(i < TELEPHONY_MAX_KEY_NUM) {
				if(state & VCONFKEY_TELEPHONY_DISP_SPN) {
					keylist[i++] = strdup(VCONFKEY_TELEPHONY_SPN_NAME);
				}
				if(state & VCONFKEY_TELEPHONY_DISP_PLMN) {
					keylist[i++] = strdup(VCONFKEY_TELEPHONY_NWNAME);
				}
			}
			if (i > 0) {
				_sim_state_string_get(keylist, buf);
				edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", buf);
			}

		}else if (service_type == VCONFKEY_TELEPHONY_SVCTYPE_NOSVC) {
			edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", _S("IDS_COM_BODY_NO_SERVICE"));
		} else if (service_type == VCONFKEY_TELEPHONY_SVCTYPE_EMERGENCY) {
			edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", _("IDS_LCKSCN_HEADER_EMERGENCY_CALLS_ONLY"));
		} else if (service_type == VCONFKEY_TELEPHONY_SVCTYPE_SEARCH) {
			edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", _S("IDS_COM_BODY_SEARCHING"));
		} else {
			edje_object_part_text_set(_EDJ(ad->ly_main), "sim.state", "");
		}
	}
}

static void _sim_state_changed_cb(keynode_t *node, void *data)
{
	_set_sim_state(data);
}

static void _register_sim_state_event_handler(void *data)
{
	int ret = 0;

	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_SVCTYPE,
				_sim_state_changed_cb, data);
	if (ret != 0)
		LOCK_SCREEN_TRACE_ERR("Failed to register [%s]",
			VCONFKEY_TELEPHONY_SVCTYPE);

	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION,
				_sim_state_changed_cb, data);
	if (ret != 0)
		LOCK_SCREEN_TRACE_ERR("Failed to register [%s]",
			VCONFKEY_TELEPHONY_SPN_DISP_CONDITION);

	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_SPN_NAME,
				_sim_state_changed_cb, data);
	if (ret != 0)
		LOCK_SCREEN_TRACE_ERR("Failed to register [%s]",
			VCONFKEY_TELEPHONY_SPN_NAME);


	ret = vconf_notify_key_changed(VCONFKEY_TELEPHONY_NWNAME,
				_sim_state_changed_cb, data);
	if (ret != 0)
		LOCK_SCREEN_TRACE_ERR("Failed to register [%s]",
			VCONFKEY_TELEPHONY_NWNAME);
}

void set_sim_state(void *data)
{
	_set_sim_state(data);
	_register_sim_state_event_handler(data);
}

void fini_sim_state(void *data)
{
	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SVCTYPE, _sim_state_changed_cb);

	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, _sim_state_changed_cb);

	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SPN_NAME, _sim_state_changed_cb);

	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_NWNAME, _sim_state_changed_cb);
}
