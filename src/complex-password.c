/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.tizenopensource.org/license
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
#include "complex-password.h"
#include "password-verification.h"

#define MIN_PASSWORD_NUM 4
#define MAX_PASSWORD_NUM 16
#define PASSWORD_ATTEMPTS_MAX_NUM  5
#define VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT   VCONFKEY_SETAPPL_PREFIX"/phone_lock_attempts_left"
#define VCONFKEY_SETAPPL_PASSWORD_TIMESTAMP_STR       VCONFKEY_SETAPPL_PREFIX"/phone_lock_timestamp"
#define PASSWORD_TIMESTAMP_STR_LENGTH 512
#define PASSWORD_BLOCK_SECONDS        30
#define EDJ_LOCKSCREEN_ENTRY EDJDIR"/lockscreen-entry.edj"

static Evas_Coord complex_pw_down_y = 0;

static void __complex_password_keypad_process(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct appdata *ad = data;
	if(ad == NULL)
		return;

	LOCK_SCREEN_TRACE_DBG("source = %s", source);

	if (strcasecmp("function1", source) == 0 || strcasecmp("function2", source) == 0) {
		return ;
	}

	if (strcasecmp("space", source) == 0) {
		if(ad->is_disabled) {
			return;
		}
		elm_entry_entry_insert(ad->c_password_entry, " ");
	} else if (strcasecmp("bs", source) == 0) {
		elm_entry_entry_insert(ad->c_password_entry, "");
		const char *input_entry_str = NULL;
		input_entry_str = elm_entry_entry_get(ad->c_password_entry);
		if (input_entry_str != NULL && strlen(input_entry_str) > 0 ) {
			int cursor_pos = elm_entry_cursor_pos_get(ad->c_password_entry);

			if (cursor_pos > 0) {
				Eina_Strbuf *temp_str_buf = eina_strbuf_new();
				eina_strbuf_append(temp_str_buf, input_entry_str);
				eina_strbuf_remove(temp_str_buf, cursor_pos - 1, cursor_pos);
				elm_entry_entry_set(ad->c_password_entry, eina_strbuf_string_get(temp_str_buf));
				elm_entry_cursor_pos_set(ad->c_password_entry,
						(cursor_pos - 1 > 0) ? cursor_pos -1 : 0);
				eina_strbuf_free(temp_str_buf);
			}
		}
	} else if (strcasecmp("enter", source) == 0) {
		if(ad->is_disabled) {
			return;
		}
		password_verification_verify(ad->h_password_policy, elm_entry_entry_get(ad->c_password_entry));
	} else {
		if(ad->is_disabled) {
			return;
		}
		elm_entry_entry_insert(ad->c_password_entry, source);
	}
}

static void __complex_password_layout_destroy(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct appdata *ad = data;

	if(ad->ly_complex_password){
		evas_object_del(ad->ly_complex_password);
		ad->ly_complex_password = NULL;
	}
	password_verification_policy_destroy(ad->h_password_policy);
	lockscreen_info_show(ad);
}

static void __complex_password_mouse_down_cb(void *data, Evas * evas, Evas_Object * obj,
			    void *event_info)
{
	Evas_Event_Mouse_Down *ei = event_info;

	complex_pw_down_y = ei->output.y;

	LOCK_SCREEN_TRACE_DBG("complex_pw_down_y = %d", complex_pw_down_y);
}

static void __complex_password_mouse_up_cb(void *data, Evas * evas, Evas_Object * obj,
			  void *event_info)
{
	struct appdata *ad = data;
	Evas_Event_Mouse_Down *ei = event_info;

	Evas_Coord pos_up_y = ei->output.y;

	if((pos_up_y - complex_pw_down_y) > _X(300)){
		edje_object_signal_emit(_EDJ(ad->ly_complex_password), "hide,qwertykeyboard", "sw.keypad.qwerty");
		edje_object_signal_callback_add(_EDJ(ad->ly_complex_password), "hide,complexpw", "event", __complex_password_layout_destroy, ad);
	}
}

static void __complex_password_mouse_move_cb(void *data, Evas * evas, Evas_Object * obj,
			    void *event_info)
{
	LOCK_SCREEN_TRACE_DBG("__complex_password_mouse_move_cb");
}

void __complex_password_check_result(void *data)
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
			snprintf(temp_left, sizeof(temp_left), _L("1 attempt left."));
		}
		else
		{
			snprintf(temp_left, sizeof(temp_left), _L("%d attempts left."), value);
		}
		snprintf(temp_str, sizeof(temp_str), "%s, %s", _L("Wrong password"), temp_left);
		edje_object_part_text_set(_EDJ(ad->ly_complex_password), "txt.result", temp_str);
	} else if (value == 0) {
		time_t cur_time = time(NULL);
		char cur_timestamp[64] = {0};
		char temp_str[512] = {0};

		snprintf(cur_timestamp, sizeof(cur_timestamp), "%ld", cur_time);
		vconf_set_str(VCONFKEY_SETAPPL_PASSWORD_TIMESTAMP_STR, cur_timestamp);
		vconf_set_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, value);
		ad->block_seconds = PASSWORD_BLOCK_SECONDS;

		snprintf(temp_str, sizeof(temp_str), _L("Input password again after %d seconds."), ad->block_seconds);
		edje_object_part_text_set(_EDJ(ad->ly_complex_password), "txt.result", temp_str);
	}
}

static void __complex_password_check_result_cb (const char *event, void *data)
{
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) {
		return;
	}

	if(strcasecmp("empty", event) == 0 || strcasecmp("overlength", event) == 0) {
		char temp_str[100] = {0};
		snprintf(temp_str, sizeof(temp_str), _L("%d to %d digits or letters required"), MIN_PASSWORD_NUM, MAX_PASSWORD_NUM);
		edje_object_part_text_set(_EDJ(ad->ly_complex_password), "txt.result", temp_str);
	}else {
		__complex_password_check_result(data);
	}
}

static void __complex_password_results_update(void *data)
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
		timestamp_str[length] = '\0';
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
				edje_object_part_text_set(_EDJ(ad->ly_complex_password), "txt.result", temp_str);
			} else {
				ad->is_disabled = EINA_FALSE;
				vconf_set_int(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, PASSWORD_ATTEMPTS_MAX_NUM);
			}
		}
	}
}

static Eina_Bool __complex_password_results_update_cb(void *data)
{
	__complex_password_results_update(data);
	return EINA_TRUE;
}

static void __complex_password_check_vconf_value(void *data)
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
		ad->password_timer = ecore_timer_add(1, (Ecore_Task_Cb)__complex_password_results_update_cb, ad);
	} else {
		if (ad->password_timer) {
			ecore_timer_del(ad->password_timer);
			ad->password_timer = NULL;
		}
		ad->is_disabled = EINA_FALSE;
	}
}

static void __complex_password_check_vconf_value_cb(keynode_t * key, void *data)
{
	__complex_password_check_vconf_value(data);
}

static void __complex_password_customize_entry(Evas_Object * entry)
{
	static Elm_Entry_Filter_Limit_Size limit_filter_data_alpha;
	Elm_Theme *th = elm_theme_new();
	elm_theme_ref_set(th, NULL);
	elm_theme_extension_add(th, EDJ_LOCKSCREEN_ENTRY);
	elm_object_theme_set(entry, th);
	elm_object_style_set(entry, "lockscreen_complex_password_style");
	limit_filter_data_alpha.max_char_count = MAX_PASSWORD_NUM;
	limit_filter_data_alpha.max_byte_count = 0;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size,
				       &limit_filter_data_alpha);
}

void complex_password_layout_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if(ad == NULL){
		return;
	}

	Evas_Object *keypad_layout = NULL;

	ad->h_password_policy = password_verification_policy_create();
	password_verification_callback_set(ad->h_password_policy, __complex_password_check_result_cb, ad);
	lockscreen_info_hide(ad);
	ad->ly_complex_password = elm_layout_add(ad->ly_main);
	elm_layout_file_set(ad->ly_complex_password, EDJEFILE, "lock-complex-password");
	elm_object_part_content_set(ad->ly_main, "sw.phone-lock", ad->ly_complex_password);
	edje_object_part_text_set(_EDJ(ad->ly_complex_password), "txt.title", _S("IDS_COM_BODY_ENTER_PASSWORD"));

	Evas_Object *entry = NULL;
	entry = elm_entry_add(ad->ly_complex_password);
	__complex_password_customize_entry(entry);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_password_set(entry, EINA_TRUE);
	elm_entry_entry_set(entry, "");
	elm_entry_cursor_end_set(entry);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_object_part_content_set(ad->ly_complex_password, "sw.password", entry);
	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(entry);
	ecore_imf_context_input_panel_enabled_set(imf_context, EINA_FALSE);
	ad->c_password_entry = entry;

	keypad_layout = elm_layout_add(ad->ly_complex_password);
	elm_layout_file_set(keypad_layout, EDJEFILE, "lock-keypad-qwerty");
	elm_object_part_content_set(ad->ly_complex_password, "sw.keypad.qwerty", keypad_layout);
	edje_object_signal_emit(_EDJ(ad->ly_complex_password), "show,qwertykeyboard", "sw.keypad.qwerty");
	edje_object_signal_callback_add(_EDJ(keypad_layout), "pad_qwerty_clicked", "*", __complex_password_keypad_process, ad);
	evas_object_event_callback_add(ad->ly_complex_password, EVAS_CALLBACK_MOUSE_DOWN, __complex_password_mouse_down_cb, ad);
	evas_object_event_callback_add(ad->ly_complex_password, EVAS_CALLBACK_MOUSE_MOVE, __complex_password_mouse_move_cb, ad);
	evas_object_event_callback_add(ad->ly_complex_password, EVAS_CALLBACK_MOUSE_UP, __complex_password_mouse_up_cb, ad);
	evas_object_show(keypad_layout);

	vconf_notify_key_changed(VCONFKEY_SETAPPL_PASSWORD_ATTEMPTS_LEFT_INT, __complex_password_check_vconf_value_cb, ad);
	__complex_password_check_vconf_value(ad);
}
