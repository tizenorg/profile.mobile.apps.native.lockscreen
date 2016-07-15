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

#include <notification.h>
#include <notification_list.h>
#include <notification_internal.h>
#include <app_control_internal.h>
#include <Ecore.h>
#include <package_manager.h>

#include "log.h"
#include "lockscreen.h"
#include "events.h"
#include "device_lock.h"

static Eina_Inlist *notifications;
static int init_count, events_count;
int LOCKSCREEN_EVENT_EVENT_ADDED;
int LOCKSCREEN_EVENT_EVENT_REMOVED;
int LOCKSCREEN_EVENT_EVENT_UPDATED;
static Ecore_Event_Handler *unlock_handler[2];

typedef enum {
	LOCKSCREEN_EVENTS_PRIVACY_MODE_SHOW_ALL, /* Shows all notification content */
	LOCKSCREEN_EVENTS_PRIVACY_MODE_HIDE_SENSITIVE_CONTENT, /* Hide content of notification */
	LOCKSCREEN_EVENTS_PRIVACY_MODE_DONT_SHOW, /* Disable events */
} lockscreen_privacy_mode_e;

static lockscreen_privacy_mode_e privacy_mode = LOCKSCREEN_EVENTS_PRIVACY_MODE_SHOW_ALL;

struct lockscreen_event {
	EINA_INLIST;
	Eina_Stringshare *icon_path;
	Eina_Stringshare *icon_sub_path;
	Eina_Stringshare *title;
	Eina_Stringshare *content;
	bundle *service_handle;
	time_t time;
	notification_h noti;
	int ref_count;
	void *user_data;
};

static bool _notification_accept(notification_h noti)
{
	int app_list = 0;
	notification_type_e type;

	if (!noti) {
		ERR("noti handle is null");
		return false;
	}

	int ret = notification_get_display_applist(noti, &app_list);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_display_applist failed: %s", get_error_message(ret));
		return false;
	}
	ret = notification_get_type(noti, &type);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_type failed: %s", get_error_message(ret));
		return false;
	}
	return (app_list & NOTIFICATION_DISPLAY_APP_LOCK) && (type == NOTIFICATION_TYPE_NOTI);
}

static bool
_lockscreen_event_notification_load_texts(lockscreen_event_t *event, notification_h noti)
{
	char *val;

	int ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_text failed: %s", get_error_message(ret));
		return false;
	}
	if (val) event->title = eina_stringshare_add(val);

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_text failed: %s", get_error_message(ret));
		return false;
	}
	if (val) event->content = eina_stringshare_add(val);
	return true;
}

static bool
_lockscreen_event_notification_load_texts_hidden(lockscreen_event_t *event, notification_h noti)
{
	char *val = NULL, *package;
	package_info_h handle;

	int ret = notification_get_pkgname(noti, &package);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_pkgname failed: %s", get_error_message(ret));
		return false;
	}

	ret = package_info_create(package, &handle);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		ERR("package_info_create failed: %s", get_error_message(ret));
		return false;
	}

	ret = package_info_get_label(handle, &val);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		ERR("package_info_get_label failed: %s", get_error_message(ret));
		package_info_destroy(handle);
		return false;
	}
	if (val) {
		event->title = eina_stringshare_add(val);
		free(val);
	}

	package_info_destroy(handle);

	event->content = eina_stringshare_add(_("IDS_ST_CONTENT_HIDDEN"));
	return true;
}

static void
_lockscreen_event_notification_unload_content(lockscreen_event_t *event)
{
	if (event->title) eina_stringshare_del(event->title);
	if (event->content) eina_stringshare_del(event->content);
	if (event->icon_path) eina_stringshare_del(event->icon_path);
	if (event->icon_sub_path) eina_stringshare_del(event->icon_sub_path);
	if (event->service_handle) bundle_free(event->service_handle);
	if (event->noti) notification_free(event->noti);

	event->title = NULL;
	event->content = NULL;
	event->icon_path = NULL;
	event->icon_sub_path = NULL;
	event->service_handle = NULL;
	event->noti = NULL;
}

static bool
_lockscreen_event_notification_load_content(lockscreen_event_t *event, notification_h noti)
{
	char *val;

	if (privacy_mode == LOCKSCREEN_EVENTS_PRIVACY_MODE_SHOW_ALL) {
		if (!_lockscreen_event_notification_load_texts(event, noti)) {
			ERR("_lockscreen_event_notification_load_texts failed");
			_lockscreen_event_notification_unload_content(event);
			return false;
		}
	} else if (privacy_mode == LOCKSCREEN_EVENTS_PRIVACY_MODE_HIDE_SENSITIVE_CONTENT) {
		if (!_lockscreen_event_notification_load_texts_hidden(event, noti)) {
			ERR("_lockscreen_event_notification_load_texts_hidden failed");
			_lockscreen_event_notification_unload_content(event);
			return false;
		}
	}

	int ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_event_notification_unload_content(event);
		return false;
	}
	if (val) event->icon_path = eina_stringshare_add(val);

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_SUB, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_event_notification_unload_content(event);
		return false;
	}
	if (val) event->icon_sub_path = eina_stringshare_add(val);

	ret = notification_get_time(noti, &event->time);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_time failed: %s", get_error_message(ret));
		_lockscreen_event_notification_unload_content(event);
		return false;
	}

	ret = notification_get_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &event->service_handle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_execute_option failed: %s", get_error_message(ret));
		_lockscreen_event_notification_unload_content(event);
		return false;
	}

	if (event->service_handle) {
		event->service_handle = bundle_dup(event->service_handle);
	}

	ret = notification_clone(noti, &event->noti);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_clone failed: %s", get_error_message(ret));
		_lockscreen_event_notification_unload_content(event);
		return false;
	}

	return true;
}

static lockscreen_event_t *_lockscreen_event_notification_create(notification_h noti)
{
	lockscreen_event_t *event = calloc(1, sizeof(lockscreen_event_t));
	if (!event) return NULL;

	if (!_lockscreen_event_notification_load_content(event, noti)) {
		ERR("_lockscreen_event_notification_load_content failed");
		free(event);
		return NULL;
	}

	DBG("Title: %s", event->title);
	DBG("Content: %s", event->content);
	DBG("Icon: %s", event->icon_path);
	DBG("SubIcon: %s", event->icon_sub_path);

	return event;
}

static void
_dummy(void *data, void *fn)
{
}

static int _lockscreen_events_ctrl_sort(const void *data1, const void *data2)
{
	time_t time1 = lockscreen_event_time_get(data1);
	time_t time2 = lockscreen_event_time_get(data2);

	return time1 > time2 ? -1 : 1;
}

static void
_notification_add(int priv_id, notification_h noti)
{
	if (!_notification_accept(noti))
		return;

	lockscreen_event_t *event = _lockscreen_event_notification_create(noti);
	if (!event) return;

	lockscreen_event_ref(event);
	notifications = eina_inlist_sorted_insert(notifications, EINA_INLIST_GET(event),
			_lockscreen_events_ctrl_sort);
	events_count++;
	ecore_event_add(LOCKSCREEN_EVENT_EVENT_ADDED, event, _dummy, NULL);
}

static int _load_notifications()
{
	notification_list_h noti_list;
	notification_list_h noti_list_head = NULL;
	notification_h noti = NULL;
	int id;

	int ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &noti_list_head);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_list failed: %s", get_error_message(ret));
		return 1;
	}

	noti_list = noti_list_head;
	while (noti_list) {
		noti = notification_list_get_data(noti_list);
		if (_notification_accept(noti)) {
			int err = notification_get_id(noti, NULL, &id);
			if (err == NOTIFICATION_ERROR_NONE) {
				_notification_add(id, noti);
			}
		}
		noti_list = notification_list_get_next(noti_list);
	}

	notification_free_list(noti_list_head);
	return 0;
}

static lockscreen_event_t*
_notification_search_by_id(int priv_id)
{
	lockscreen_event_t *event;

	EINA_INLIST_FOREACH(notifications, event) {
		int id, err;
		err = notification_get_id(event->noti, NULL, &id);
		if (err != NOTIFICATION_ERROR_NONE) {
			ERR("notification_get_id failed: %s", get_error_message(err));
			continue;
		}
		if (id == priv_id) return event;
	}
	return NULL;
}

static void
_notification_update(int priv_id, notification_h noti)
{
	lockscreen_event_t *event = _notification_search_by_id(priv_id);
	if (!event) {
		_notification_add(priv_id, noti);
		return;
	}

	_lockscreen_event_notification_unload_content(event);
	if (!_lockscreen_event_notification_load_content(event, noti))
		ERR("_lockscreen_event_notification_load_content failed");

	notifications = eina_inlist_sort(notifications, _lockscreen_events_ctrl_sort);
	ecore_event_add(LOCKSCREEN_EVENT_EVENT_UPDATED, event, _dummy, NULL);
}

static void
_lockscreen_event_unref(void *data, void *fn)
{
	lockscreen_event_t *event = fn;
	notifications = eina_inlist_remove(notifications, EINA_INLIST_GET(event));
	events_count--;
	lockscreen_event_unref(event);
}

static void
_notification_remove(int priv_id)
{
	lockscreen_event_t *event = _notification_search_by_id(priv_id);
	if (!event) return;

	ecore_event_add(LOCKSCREEN_EVENT_EVENT_REMOVED, event, _lockscreen_event_unref, NULL);
}

static void
_notification_remove_all(void)
{
	Eina_Inlist *l;
	lockscreen_event_t *event;
	int id;

	EINA_INLIST_FOREACH_SAFE(notifications, l, event) {
		int err = notification_get_id(event->noti, NULL, &id);
		if (err == NOTIFICATION_ERROR_NONE) {
			_notification_remove(id);
		}
	}
	notifications = NULL;
}

static void _noti_changed_cb(void *data, notification_type_e type, notification_op *op_list, int num_op)
{
	if (!op_list)
		return;

	switch (op_list->type) {
		case NOTIFICATION_OP_INSERT:
			_notification_add(op_list->priv_id, op_list->noti);
			break;
		case NOTIFICATION_OP_UPDATE:
			_notification_update(op_list->priv_id, op_list->noti);
			break;
		case NOTIFICATION_OP_DELETE:
			_notification_remove(op_list->priv_id);
			break;
		case NOTIFICATION_OP_DELETE_ALL:
			_notification_remove_all();
			break;
		default:
			break;
	}
}

int lockscreen_events_init(void)
{
	if (!init_count) {
		// Fixme load privacy mode from settings, when it will be implemented
		if (privacy_mode == LOCKSCREEN_EVENTS_PRIVACY_MODE_DONT_SHOW)
			return 0;

		if (lockscreen_device_lock_init()) {
			ERR("lockscreen_device_lock_init failed");
			return 1;
		}
		LOCKSCREEN_EVENT_EVENT_ADDED = ecore_event_type_new();
		LOCKSCREEN_EVENT_EVENT_REMOVED = ecore_event_type_new();
		LOCKSCREEN_EVENT_EVENT_UPDATED = ecore_event_type_new();

		int ret = notification_register_detailed_changed_cb(_noti_changed_cb, NULL);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_register_detailed_changed_cb failed: %s", get_error_message(ret));
			lockscreen_device_lock_shutdown();
			return 1;
		}
		if (_load_notifications()) {
			ERR("load_notifications failed");
			notification_unregister_detailed_changed_cb(_noti_changed_cb, NULL);
			lockscreen_device_lock_shutdown();
			return 1;
		}
	}
	init_count++;
	return 0;
}

void lockscreen_events_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count) {
			int ret = notification_unregister_detailed_changed_cb(_noti_changed_cb, NULL);
			if (ret != NOTIFICATION_ERROR_NONE) {
				ERR("notification_unregister_detailed_changed_cb failed: %s", get_error_message(ret));
			}
			Eina_Inlist *l;
			EINA_INLIST_FREE(notifications, l) {
				lockscreen_event_unref(EINA_INLIST_CONTAINER_GET(l, lockscreen_event_t));
			}
			notifications = NULL;
		}
	}
}

static Eina_Bool
_lockscreen_events_device_unlock_cancelled(void *data, int event, void *event_info)
{
	ecore_event_handler_del(unlock_handler[0]);
	ecore_event_handler_del(unlock_handler[1]);
	return EINA_TRUE;
}

static bool _lockscreen_event_launch_internal(lockscreen_event_t *event)
{
	app_control_h service = NULL;

	int ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_create failed: %s", get_error_message(ret));
		return false;
	}

	ret = app_control_import_from_bundle(service, event->service_handle);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_import_from_bundle: %s", get_error_message(ret));
		app_control_destroy(service);
		return false;
	}

	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_send_launch_request failed: %s", get_error_message(ret));
		app_control_destroy(service);
		return false;
	}

	app_control_destroy(service);

	return true;
}

static Eina_Bool
_lockscreen_events_device_unlocked(void *data, int event, void *event_info)
{
	lockscreen_device_unlock_context_t *context = event_info;

	if (!_lockscreen_event_launch_internal(context->data.event))
		ERR("lockscreen_event_launch failed");

	ecore_event_handler_del(unlock_handler[0]);
	ecore_event_handler_del(unlock_handler[1]);

	return EINA_TRUE;
}

bool lockscreen_event_launch(lockscreen_event_t *event)
{
	lockscreen_device_unlock_context_t context;

	if (!event->service_handle) {
		return false;
	}

	context.type = LOCKSCREEN_DEVICE_UNLOCK_CONTEXT_LAUNCH_EVENT;
	context.data.event = event;

	if (!lockscreen_device_lock_unlock_request(&context)) {
		unlock_handler[0] = ecore_event_handler_add(
				LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCK_CANCELLED,
				_lockscreen_events_device_unlock_cancelled, NULL);
		unlock_handler[1] = ecore_event_handler_add(
				LOCKSCREEN_EVENT_DEVICE_LOCK_UNLOCKED,
				_lockscreen_events_device_unlocked, NULL);
	} else {
		INF("lockscreen_device_lock_unlock_request failed");
		return false;
	}
	return true;
}

Eina_List *lockscreen_events_get(void)
{
	lockscreen_event_t *event;
	Eina_List *ret = NULL;

	EINA_INLIST_FOREACH(notifications, event)
		ret = eina_list_append(ret, event);

	return ret;
}

bool lockscreen_events_exists(void)
{
	return notifications ? EINA_TRUE : EINA_FALSE;
}

const char *lockscreen_event_title_get(const lockscreen_event_t *event)
{
	return event->title;
}

const char *lockscreen_event_content_get(const lockscreen_event_t *event)
{
	return event->content;
}

time_t lockscreen_event_time_get(const lockscreen_event_t *event)
{
	return event->time;
}

const char *lockscreen_event_icon_get(const lockscreen_event_t *event)
{
	return event->icon_path;
}

const char *lockscreen_event_sub_icon_get(const lockscreen_event_t *event)
{
	return event->icon_sub_path;
}

bool lockscreen_event_remove(lockscreen_event_t *event)
{
	int app_list;
	int ret = notification_get_display_applist(event->noti, &app_list);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_set_display_applist failed: %s", get_error_message(ret));
		return false;
	}
	// if only for lockscreen remove totally
	if (app_list == NOTIFICATION_DISPLAY_APP_LOCK) {
		ret = notification_delete(event->noti);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_delete failed: %s", get_error_message(ret));
			return false;
		}
	} else {
		// remove lockscreen from display list
		ret = notification_set_display_applist(event->noti,
				app_list & ~NOTIFICATION_DISPLAY_APP_LOCK);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_set_display_applist failed: %s", get_error_message(ret));
			return false;
		}
		ret = notification_update(event->noti);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_update failed: %s", get_error_message(ret));
			return false;
		}
	}
	return true;
}

void lockscreen_events_remove_all(void)
{
	lockscreen_event_t *event;
	EINA_INLIST_FOREACH(notifications, event)
		lockscreen_event_remove(event);

	_notification_remove_all();
}

void lockscreen_event_ref(lockscreen_event_t *event)
{
	event->ref_count++;
}

void lockscreen_event_unref(lockscreen_event_t *event)
{
	event->ref_count--;
	if (event->ref_count <= 0) {
		_lockscreen_event_notification_unload_content(event);
		free(event);
	}
}

void lockscreen_event_user_data_set(lockscreen_event_t *event, void *user_data)
{
	event->user_data = user_data;
}

void *lockscreen_event_user_data_get(lockscreen_event_t *event)
{
	return event->user_data;
}

lockscreen_event_t *lockscreen_event_prev(lockscreen_event_t *event)
{
	Eina_Inlist *prev = EINA_INLIST_GET(event)->prev;
	return prev ? EINA_INLIST_CONTAINER_GET(prev, lockscreen_event_t) : NULL;
}

lockscreen_event_t *lockscreen_event_next(lockscreen_event_t *event)
{
	Eina_Inlist *next = EINA_INLIST_GET(event)->next;
	return next ? EINA_INLIST_CONTAINER_GET(next, lockscreen_event_t) : NULL;
}

int lockscreen_events_count(void)
{
	return events_count;
}
