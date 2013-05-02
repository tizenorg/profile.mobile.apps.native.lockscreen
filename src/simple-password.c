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

#include "lockscreen.h"
#include "util.h"
#include "log.h"
#include "password-verification.h"

#define MAX_PASSWORD_NUM 4
#define PASSWORD_ATTEMPTS_MAX_NUM  5
#define VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT   VCONFKEY_SETAPPL_PREFIX"/phone_lock_attempts_left"
#define VCONFKEY_SETAPPL_PASSWORD_TIMESTAMP_STR       VCONFKEY_SETAPPL_PREFIX"/phone_lock_timestamp"
#define PASSWORD_TIMESTAMP_STR_LENGTH 512
#define PASSWORD_BLOCK_SECONDS        30

static char simple_password[MAX_PASSWORD_NUM + 1] = {0};
static int simple_password_length = 0;
static Evas_Coord simple_pw_down_y = 0;

static void __simple_password_password_show(struct appdata *ad)
{
	char buf1[50] = {0};
	char buf2[50] = {0};
	int i = 1;

	for(i = 1; i <= simple_password_length; i++){
		snprintf(buf1, sizeof(buf1), "show,password%d", i);
		snprintf(buf2, sizeof(buf2), "password%d", i);
		edje_object_signal_emit(_EDJ(ad->ly_simple_password), buf1, buf2);
	}
	for(i = MAX_PASSWORD_NUM; i > simple_password_length; i--){
		snprintf(buf1, sizeof(buf1), "hide,password%d", i);
		snprintf(buf2, sizeof(buf2), "password%d", i);
		edje_object_signal_emit(_EDJ(ad->ly_simple_password), buf1, buf2);
	}
}

static void __simple_password_keypad_process(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct appdata *ad = data;

	LOCK_SCREEN_TRACE_DBG("source = %s", source);
	if (strcmp("SOS", source) == 0) {
		launch_emgcall(ad);
		return;
	} else if (strcmp("Backspace", source) == 0) {
		if(simple_password_length <= 0){
			simple_password[0] = '\0';
			return;
		}
		simple_password_length--;
		simple_password[simple_password_length] = '\0';
	} else {
		if(ad->is_disabled){
			return;
		}
		if(simple_password_length >= MAX_PASSWORD_NUM){
			return;
		}else {
			simple_password[simple_password_length] = *source;
			simple_password_length++;
		}

		if(simple_password_length == MAX_PASSWORD_NUM){
			password_verification_verify(ad->h_password_policy, simple_password);
		}
	}

	LOCK_SCREEN_TRACE_DBG("simple_password = %s, simple_password_length = %d", simple_password, simple_password_length);
	__simple_password_password_show(ad);
}

static void __simple_password_layout_destroy(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct appdata *ad = data;
	int i = 0;
	LOCK_SCREEN_TRACE_DBG("__simple_password_layout_destroy");

	for(i = 0; i <= MAX_PASSWORD_NUM; i++){
		simple_password[i] = 0;
	}
	simple_password_length = 0;
	if (ad->password_timer) {
		ecore_timer_del(ad->password_timer);
		ad->password_timer = NULL;
	}
	if(ad->ly_simple_password){
		evas_object_del(ad->ly_simple_password);
		ad->ly_simple_password = NULL;
	}
	password_verification_policy_destroy(ad->h_password_policy);
	lockscreen_info_show(ad);
}

static void __simple_password_mouse_down_cb(void *data, Evas * evas, Evas_Object * obj,
			    void *event_info)
{
	Evas_Event_Mouse_Down *ei = event_info;

	simple_pw_down_y = ei->output.y;

	LOCK_SCREEN_TRACE_DBG("simple_pw_down_y = %d", simple_pw_down_y);
}

static void __simple_password_mouse_up_cb(void *data, Evas * evas, Evas_Object * obj,
			  void *event_info)
{
	struct appdata *ad = data;
	Evas_Event_Mouse_Down *ei = event_info;

	Evas_Coord pos_up_y = ei->output.y;

	if((pos_up_y - simple_pw_down_y) > _X(300)){
		edje_object_signal_emit(_EDJ(ad->ly_simple_password), "hide,numberkeyboard", "sw.keypad.number");
		edje_object_signal_callback_add(_EDJ(ad->ly_simple_password), "hide,simplepw", "event", __simple_password_layout_destroy, ad);
	}
}

static void __simple_password_mouse_move_cb(void *data, Evas * evas, Evas_Object * obj,
			    void *event_info)
{
	LOCK_SCREEN_TRACE_DBG("__simple_password_mouse_move_cb");
}

void __simple_password_check_result(void *data)
{
	struct appdata *ad = (struct appdata *) data;

	int value = -1;

	vconf_get_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, &value);
	value--;

	if (value > 0 && value <= PASSWORD_ATTEMPTS_MAX_NUM) {
		char temp_str[200] = {0};
		char temp_left[50] = {0};

		vconf_set_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT,
			      value);
		if(value == 1)
		{
			snprintf(temp_left, sizeof(temp_left), _("IDS_IDLE_BODY_1_ATTEMPT_LEFT"));
		}
		else
		{
			snprintf(temp_left, sizeof(temp_left), _("IDS_IDLE_BODY_PD_ATTEMPTS_LEFT"), value);
		}
		snprintf(temp_str, sizeof(temp_str), "%s, %s", _("IDS_IDLE_BODY_INCORRECT_PASSWORD"), temp_left);
		edje_object_part_text_set(_EDJ(ad->ly_simple_password), "txt.result", temp_str);
	} else if (value == 0) {
		time_t cur_time = time(NULL);
		char cur_timestamp[64] = {0};
		char temp_str[512] = {0};

		snprintf(cur_timestamp, sizeof(cur_timestamp), "%ld", cur_time);
		vconf_set_str(VCONFKEY_SETAPPL_PASSWORD_TIMESTAMP_STR, cur_timestamp);
		vconf_set_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, value);
		ad->block_seconds = PASSWORD_BLOCK_SECONDS;

		snprintf(temp_str, sizeof(temp_str), _L("Input password again after %d seconds."), ad->block_seconds);
		edje_object_part_text_set(_EDJ(ad->ly_simple_password), "txt.result", temp_str);
	}
}

static void __simple_password_check_result_cb (const char *event, void *data)
{
	__simple_password_check_result(data);
}

static void __simple_password_results_update(void *data)
{
	int value = -1;
	int ret = 0;

	struct appdata *ad = (struct appdata *) data;

	if (ad == NULL) {
		return;
	}
	LOCK_SCREEN_TRACE_DBG("%s", __FUNCTION__);

	ret =
	    vconf_get_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT,
			  &value);

	if (ret != 0) {
		return;
	}

	if (value == 0) {
		char timestamp_str[512] = {0};
		char *temp = NULL;
		int length = 0;
		int temp_length = 0;
		temp = vconf_get_str(VCONFKEY_SETAPPL_PASSWORD_TIMESTAMP_STR);
		temp_length = strlen(temp);
		length = (temp_length <= PASSWORD_TIMESTAMP_STR_LENGTH) ? temp_length : PASSWORD_TIMESTAMP_STR_LENGTH;
		strncpy(timestamp_str, temp, length);
		if(length == PASSWORD_TIMESTAMP_STR_LENGTH) {
			timestamp_str[length-1] = '\0';
		}else {
			timestamp_str[length] = '\0';
		}
		if ((strcmp(timestamp_str, "") != 0)
		    || (strlen(timestamp_str) != 0)) {
			time_t cur_time = time(NULL);
			time_t last_lock_time;
			sscanf(timestamp_str, "%ld", &last_lock_time);

			if ((cur_time - last_lock_time) < PASSWORD_BLOCK_SECONDS) {
				ad->block_seconds = PASSWORD_BLOCK_SECONDS - (cur_time - last_lock_time);
				LOCK_SCREEN_TRACE_DBG("ad->block_seconds = %d", ad->block_seconds);
				ad->is_disabled = EINA_TRUE;
				char temp_str[512] = {0};
				snprintf(temp_str, sizeof(temp_str), _L("Input password again after %d seconds."), ad->block_seconds);
				edje_object_part_text_set(_EDJ(ad->ly_simple_password), "txt.result", temp_str);
			} else {
				ad->is_disabled = EINA_FALSE;
				vconf_set_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, PASSWORD_ATTEMPTS_MAX_NUM);
			}
		}
	}
}

static Eina_Bool __simple_password_results_update_cb(void *data)
{
	__simple_password_results_update(data);
	return EINA_TRUE;
}

static void __simple_password_check_vconf_value(void *data)
{
	int phone_lock_value = -1;
	struct appdata *ad = (struct appdata *) data;

	if (ad == NULL) {
		return;
	}
	LOCK_SCREEN_TRACE_DBG("%s", __FUNCTION__);

	vconf_get_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, &phone_lock_value);

	if (phone_lock_value == 0) {
		if (ad->password_timer) {
			return;
		}
		ad->password_timer = ecore_timer_add(1, (Ecore_Task_Cb)__simple_password_results_update_cb, ad);
	} else {
		if (ad->password_timer) {
			ecore_timer_del(ad->password_timer);
			ad->password_timer = NULL;
		}
		ad->is_disabled = EINA_FALSE;
	}
}

static void __simple_password_check_vconf_value_cb(keynode_t * key, void *data)
{
	__simple_password_check_vconf_value(data);
}

void simple_password_layout_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if(ad == NULL){
		return;
	}

	Evas_Object *keypad_layout = NULL;

	ad->h_password_policy = password_verification_policy_create();
	password_verification_callback_set(ad->h_password_policy, __simple_password_check_result_cb, ad);
	lockscreen_info_hide(ad);
	ad->ly_simple_password = elm_layout_add(ad->ly_main);
	elm_layout_file_set(ad->ly_simple_password, EDJEFILE, "lock-simple-password");
	elm_object_part_content_set(ad->ly_main, "sw.phone-lock", ad->ly_simple_password);
	edje_object_part_text_set(_EDJ(ad->ly_simple_password), "txt.title", _S("IDS_COM_BODY_ENTER_PASSWORD"));

	keypad_layout = elm_layout_add(ad->ly_simple_password);
	elm_layout_file_set(keypad_layout, EDJEFILE, "lock-keypad-number");
	elm_object_part_content_set(ad->ly_simple_password, "sw.keypad.number", keypad_layout);
	edje_object_signal_emit(_EDJ(ad->ly_simple_password), "show,numberkeyboard", "sw.keypad.number");
	edje_object_signal_callback_add(_EDJ(keypad_layout), "keypad_down_clicked", "*", __simple_password_keypad_process, ad);
	evas_object_event_callback_add(ad->ly_simple_password, EVAS_CALLBACK_MOUSE_DOWN, __simple_password_mouse_down_cb, ad);
	evas_object_event_callback_add(ad->ly_simple_password, EVAS_CALLBACK_MOUSE_MOVE, __simple_password_mouse_move_cb, ad);
	evas_object_event_callback_add(ad->ly_simple_password, EVAS_CALLBACK_MOUSE_UP, __simple_password_mouse_up_cb, ad);
	evas_object_show(keypad_layout);
	vconf_notify_key_changed(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, __simple_password_check_vconf_value_cb, ad);
	__simple_password_check_vconf_value(ad);
}
