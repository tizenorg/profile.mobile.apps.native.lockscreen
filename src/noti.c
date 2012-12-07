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

#include <notification.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "lockscreen.h"
#include "log.h"

#define PHONE_LAUNCH_PKG               "org.tizen.phone"
#define EMAIL_LAUNCH_PKG               "org.tizen.email"
#define MESSAGE_LAUNCH_PKG             "org.tizen.message"
#define MAX_NOTI_NUM                   3

static void __noti_info_set(void *data, const char *pkgname, const char *title, int count, int line_num)
{
	struct appdata *ad = (struct appdata *)data;
	if(ad == NULL || pkgname == NULL){
		return;
	}

	LOCK_SCREEN_TRACE_DBG("pkgname : %s, title : %s, count = %d, line_num = %d", pkgname, title, count, line_num);

	char buf1[512] = {0};
	char buf2[512] = {0};

	snprintf(buf2, sizeof(buf2), "noti_title%d", line_num);

	if (!strcmp(pkgname, PHONE_LAUNCH_PKG)) {
		if(count == 1){
			snprintf(buf1, sizeof(buf1), "%d %s", count, _NOT_LOCALIZED("Missed call"));
		}else if(count > 1){
			snprintf(buf1, sizeof(buf1), "%d %s", count, _NOT_LOCALIZED("Missed calls"));
		}
		edje_object_part_text_set(_EDJ(ad->noti), buf2, buf1);
	} else if (!strcmp(pkgname, MESSAGE_LAUNCH_PKG)) {
		if(count == 1){
			snprintf(buf1, sizeof(buf1), "%d %s", count, _NOT_LOCALIZED("New message"));
		}else if(count > 1){
			snprintf(buf1, sizeof(buf1), "%d %s", count, _NOT_LOCALIZED("New messages"));
		}
		edje_object_part_text_set(_EDJ(ad->noti), buf2, buf1);
	} else if (!strcmp(pkgname, EMAIL_LAUNCH_PKG)) {
		if(count == 1){
			snprintf(buf1, sizeof(buf1), "%d %s", count, _NOT_LOCALIZED("New email"));
		}else if(count > 1){
			snprintf(buf1, sizeof(buf1), "%d %s", count, _NOT_LOCALIZED("New emails"));
		}
		edje_object_part_text_set(_EDJ(ad->noti), buf2, buf1);
	} else {
		edje_object_part_text_set(_EDJ(ad->noti), buf2, title);
	}
}

static int __ticker_check_setting_event_value(notification_h noti)
{
	char *pkgname = NULL;
	char key[512] = {0, };
	int ret = 0;
	int boolval = 0;

	notification_get_application(noti, &pkgname);

	if (pkgname == NULL) {
		notification_get_pkgname(noti, &pkgname);
	}

	if (pkgname == NULL) {
		return 0;
	}

	if (!strcmp(pkgname, MESSAGE_LAUNCH_PKG)) {
		ret = vconf_get_bool(VCONFKEY_SETAPPL_STATE_TICKER_NOTI_MESSAGES_BOOL, &boolval);
		if (ret == 0 && boolval == 0) {
			return 0;
		}
	} else if (!strcmp(pkgname, EMAIL_LAUNCH_PKG)) {
		ret = vconf_get_bool(VCONFKEY_SETAPPL_STATE_TICKER_NOTI_EMAIL_BOOL, &boolval);
		if (ret == 0 && boolval == 0) {
			return 0;
		}
	}

	snprintf(key, sizeof(key), "db/app-settings/noti-enabled/%s", pkgname);
	ret = vconf_get_int(key, &boolval);
	if (ret == 0 && boolval == 0) {
		return 0;
	}

	return 1;
}

static void __noti_changed_cb(void *data, notification_type_e type)
{
	notification_h noti = NULL;
	notification_list_h notification_list = NULL;
	notification_list_h get_list = NULL;
	int count = 0, group_id = 0, priv_id = 0, show_noti = 0, num = 1;
	char *pkgname = NULL;
	char *title = NULL;
	char *str_count = NULL;
	int i = 1;
	char buf[512] = {0};
	struct appdata *ad = (struct appdata *)data;

	for(i = 1; i <= MAX_NOTI_NUM; i++){
		snprintf(buf, sizeof(buf), "noti_title%d", i);
		edje_object_part_text_set(_EDJ(ad->noti), buf, "");
	}

	notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &notification_list);
	if (notification_list) {
		get_list = notification_list_get_head(notification_list);
		noti = notification_list_get_data(get_list);
		LOCK_SCREEN_TRACE_DBG("get_list : %p, noti : %p", get_list, noti);
		while(get_list != NULL) {
			notification_get_id(noti, &group_id, &priv_id);
			notification_get_pkgname(noti, &pkgname);
			if(pkgname == NULL){
				notification_get_application(noti, &pkgname);
			}
			show_noti = __ticker_check_setting_event_value(noti);
			LOCK_SCREEN_TRACE_DBG("show noti : %d", show_noti);
			if (show_noti == 0) {
				get_list = notification_list_get_next(get_list);
				noti = notification_list_get_data(get_list);
				continue;
			}

			if(num > MAX_NOTI_NUM){
				if (notification_list != NULL) {
					notification_free_list(notification_list);
					notification_list = NULL;
				}
				return;
			}

			notification_get_text(noti, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, &str_count);
			if (!str_count) {
				count = 0;
			} else {
				count = atoi(str_count);
			}
			notification_get_title(noti, &title, NULL);
			__noti_info_set(data, pkgname, title, count, num);
			get_list = notification_list_get_next(get_list);
			noti = notification_list_get_data(get_list);
			num++;
		}
	}
	if (notification_list != NULL) {
		notification_free_list(notification_list);
		notification_list = NULL;
	}
}

void noti_process(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	if(ad == NULL){
		return;
	}

	ad->noti = elm_layout_add(ad->ly_main);
	elm_layout_file_set(ad->noti, EDJEFILE, "lock-noti");
	evas_object_show(ad->noti);
	elm_object_part_content_set(ad->ly_main, "sw.noti", ad->noti);

	notification_resister_changed_cb(__noti_changed_cb, ad);
	__noti_changed_cb(ad, NOTIFICATION_TYPE_NOTI);
}
