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

#include "log.h"
#include "lockscreen.h"
#include "events.h"
#include "device_lock.h"

static Eina_List *notifications;
static int init_count;
int LOCKSCREEN_EVENT_EVENTS_CHANGED;
static bool freeze_event;
static Ecore_Event_Handler *unlock_handler[2];

struct lockscreen_event {
	char *icon_path;
	char *icon_sub_path;
	char *title;
	char *content;
	bundle *service_handle;
	time_t time;
	char *package;
	notification_h noti;
};

static bool _notification_accept(notification_h noti)
{
	int app_list = 0;
	int ret = notification_get_display_applist(noti, &app_list);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_display_applist failed: %s", get_error_message(ret));
		return false;
	}
	return app_list & NOTIFICATION_DISPLAY_APP_LOCK;
}

static void _lockscreen_event_destroy(lockscreen_event_t *event)
{
	if (event->title) free(event->title);
	if (event->content) free(event->content);
	if (event->icon_path) free(event->icon_path);
	if (event->icon_sub_path) free(event->icon_sub_path);
	if (event->package) free(event->package);
	if (event->service_handle) bundle_free(event->service_handle);
	if (event->noti) notification_free(event->noti);

	free(event);
}

static lockscreen_event_t *_lockscreen_event_notification_create(notification_h noti)
{
	int ret;
	char *val;
	lockscreen_event_t *event = calloc(1, sizeof(lockscreen_event_t));
	if (!event) return NULL;

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_text failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->title = strdup(val);

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_text failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->content = strdup(val);

	ret = notification_get_pkgname(noti, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_pkgname failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->package = strdup(val);

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->icon_path = strdup(val);

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_SUB, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->icon_sub_path = strdup(val);

	ret = notification_get_time(noti, &event->time);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_time failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}

	ret = notification_get_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &event->service_handle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_execute_option failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}

	if (event->service_handle) {
		event->service_handle = bundle_dup(event->service_handle);
	}

	ret = notification_clone(noti, &event->noti);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_clone failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}

	DBG("Title: %s", event->title);
	DBG("Content: %s", event->content);
	DBG("Package: %s", event->package);
	DBG("Icon: %s", event->icon_path);
	DBG("SubIcon: %s", event->icon_sub_path);

	return event;
}

static void _unload_notifications(void *eevent, void *data)
{
	lockscreen_event_t *event;
	Eina_List *free_list = data;

	if (!free_list)
		return;

	EINA_LIST_FREE(free_list, event)
		_lockscreen_event_destroy(event);
}

static int _reload_notifications()
{
	notification_list_h noti_list;
	notification_list_h noti_list_head = NULL;
	notification_h noti = NULL;
	Eina_List *new_notifications = NULL;

	int ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &noti_list_head);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_get_list failed: %s", get_error_message(ret));
		return 1;
	}

	noti_list = noti_list_head;
	while (noti_list) {
		noti = notification_list_get_data(noti_list);
		if (_notification_accept(noti)) {
			lockscreen_event_t *me = _lockscreen_event_notification_create(noti);
			new_notifications = eina_list_append(new_notifications, me);
		}
		noti_list = notification_list_get_next(noti_list);
	}

	ecore_event_add(LOCKSCREEN_EVENT_EVENTS_CHANGED, notifications, _unload_notifications, NULL);
	notifications = new_notifications;

	notification_free_list(noti_list_head);
	return 0;
}

static void _noti_changed_cb(void *data, notification_type_e type, notification_op *op_list, int num_op)
{
	//TODO optimize loading
	switch (op_list->type) {
		case NOTIFICATION_OP_INSERT:
		case NOTIFICATION_OP_UPDATE:
			if (_notification_accept(op_list->noti))
				_reload_notifications();
			break;
		case NOTIFICATION_OP_DELETE:
		case NOTIFICATION_OP_DELETE_ALL:
			_reload_notifications();
			break;
		default:
			break;
	}
}

int lockscreen_events_init(void)
{
	if (!init_count) {
		if (lockscreen_device_lock_init()) {
			ERR("lockscreen_device_lock_init failed");
			return 1;
		}
		LOCKSCREEN_EVENT_EVENTS_CHANGED = ecore_event_type_new();
		int ret = notification_register_detailed_changed_cb(_noti_changed_cb, NULL);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_register_detailed_changed_cb failed: %s", get_error_message(ret));
			return 1;
		}
		if (_reload_notifications()) {
			ERR("load_notifications failed");
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
			_unload_notifications(NULL, notifications);
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

	ret = app_control_enable_app_started_result_event(service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_enable_app_started_result_event: %s", get_error_message(ret));
		app_control_destroy(service);
		return false;
	}

	INF("Launching event for package: %s", event->package);

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

	context.type = LOCKSCREEN_DEVICE_UNLOCK_CONTEXT_EVENT;
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
	return eina_list_clone(notifications);
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

static void _lockscreen_event_notification_delete(notification_h noti)
{
	int app_list;
	int ret = notification_get_display_applist(noti, &app_list);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_set_display_applist failed: %s", get_error_message(ret));
		return;
	}
	// if only for lockscreen remove totally
	if (app_list == NOTIFICATION_DISPLAY_APP_LOCK) {
		ret = notification_delete(noti);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_delete failed: %s", get_error_message(ret));
		}
	} else {
		// remove lockscreen from display list
		ret = notification_set_display_applist(noti, app_list & ~NOTIFICATION_DISPLAY_APP_LOCK);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_set_display_applist failed: %s", get_error_message(ret));
			return;
		}
		ret = notification_update(noti);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("notification_update failed: %s", get_error_message(ret));
			return;
		}
	}
}

void lockscreen_event_remove(lockscreen_event_t *event)
{
	_lockscreen_event_notification_delete(event->noti);
	notifications = eina_list_remove(notifications, event);
	_lockscreen_event_destroy(event);
	if (!freeze_event)
		ecore_event_add(LOCKSCREEN_EVENT_EVENTS_CHANGED, NULL, NULL, NULL);
}

void lockscreen_events_remove_all(void)
{
	lockscreen_event_t *event;
	freeze_event = true;
	Eina_List *l, *l2;

	EINA_LIST_FOREACH_SAFE(notifications, l, l2, event) {
		lockscreen_event_remove(event);
	}
	notifications = NULL;
	freeze_event = false;
	ecore_event_add(LOCKSCREEN_EVENT_EVENTS_CHANGED, NULL, NULL, NULL);
}

lockscreen_event_t *lockscreen_event_copy(const lockscreen_event_t *event)
{
	return _lockscreen_event_notification_create(event->noti);
}

void lockscreen_event_free(lockscreen_event_t *event)
{
	_lockscreen_event_destroy(event);
}
