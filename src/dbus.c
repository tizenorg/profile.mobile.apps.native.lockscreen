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

#include <Elementary.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "lockscreen.h"
#include "log.h"
#include "dbus.h"
#include "property.h"
#include "lock_time.h"
#include "default_lock.h"

static struct _s_info {
	DBusConnection *connection;
	Eina_List *cbs_list[DBUS_EVENT_MAX];
	int is_rotate_signal_added;
} s_info = {
	.connection = NULL,
	.cbs_list = { NULL, },
	.is_rotate_signal_added = 0,
};

typedef struct {
	void (*result_cb)(void *, void *);
	void *result_data;
} dbus_cb_s;

static void _execute_cbs(int type, void *event_info)
{
	dbus_cb_s *cb = NULL;

	Eina_List *list = eina_list_clone(s_info.cbs_list[type]);
	EINA_LIST_FREE(list, cb) {
		continue_if(!cb);
		continue_if(!cb->result_cb);

		cb->result_cb(cb->result_data, event_info);
	}
}

static void _cbs_fini(void)
{
	int i = 0;

	const Eina_List *l = NULL;
	const Eina_List *n = NULL;
	dbus_cb_s *cb = NULL;

	for (i = 0; i < DBUS_EVENT_MAX; i++) {
		EINA_LIST_FOREACH_SAFE(s_info.cbs_list[i], l, n, cb) {
			free(cb);
		}
	}
}

static DBusConnection *_dbus_connection_get(void)
{
	if (!s_info.connection) {
		DBusError derror;
		DBusConnection *connection = NULL;

		dbus_error_init(&derror);
		connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &derror);
		if (!connection) {
			_E("Failed to get dbus connection : %s", derror.message);
			dbus_error_free(&derror);
			return NULL;
		}
		dbus_connection_setup_with_g_main(connection, NULL);
		dbus_error_free(&derror);

		s_info.connection = connection;
	}

	return s_info.connection;
}

static DBusHandlerResult _dbus_message_recv_cb(DBusConnection *connection, DBusMessage *message, void *data)
{
	if (dbus_message_is_signal(message, DBUS_DEVICED_DISPLAY_INTERFACE, DBUS_DEVICED_DISPLAY_MEMBER_LCD_ON)) {
		_I("LCD on");
		int ret = 0;
		DBusError derror;
		const char *state = NULL;
		dbus_error_init(&derror);
		ret = dbus_message_get_args(message, &derror, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);
		if (!ret) {
			_E("Failed to get reply (%s:%s)", derror.name, derror.message);
		}
		_execute_cbs(DBUS_EVENT_LCD_ON, (void*)state);
		dbus_error_free(&derror);
	} else if (dbus_message_is_signal(message, DBUS_DEVICED_DISPLAY_INTERFACE, DBUS_DEVICED_DISPLAY_MEMBER_LCD_OFF)) {
		_I("LCD off");
		int ret = 0;
		DBusError derror;
		const char *state = NULL;
		dbus_error_init(&derror);
		ret = dbus_message_get_args(message, &derror, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);
		if (!ret) {
			_E("Failed to get reply (%s:%s)", derror.name, derror.message);
		}
		_execute_cbs(DBUS_EVENT_LCD_OFF, (void*)state);
		dbus_error_free(&derror);
	} else if (dbus_message_is_signal(message, DBUS_ROTATION_INTERFACE, DBUS_ROTATION_MEMBER_CHANGED)) {
		int ret = 0;
		DBusError derror;
		int state = 0;
		dbus_error_init(&derror);
		ret = dbus_message_get_args(message, &derror, DBUS_TYPE_INT32, &state, DBUS_TYPE_INVALID);
		if (!ret) {
			_E("Failed to get reply (%s:%s)", derror.name, derror.message);
		}

		int angle = (state - 1) * 90;
		angle = (angle < 0) ? 0 : angle;

		_I("rotation changed : %d", angle);
		_execute_cbs(DBUS_EVENT_ANGLE_CHANGED, (void*)angle);
		dbus_error_free(&derror);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

static lock_error_e _dbus_sig_attach(char *path, char *interface, char *member)
{
	DBusError derror;
	DBusConnection *connection = NULL;

	retv_if(!path, LOCK_ERROR_INVALID_PARAMETER);
	retv_if(!interface, LOCK_ERROR_INVALID_PARAMETER);
	retv_if(!member, LOCK_ERROR_INVALID_PARAMETER);

	/* DBUS */
	connection = _dbus_connection_get();
	if (!connection) {
		_E("Failed to get DBUS connection");
		return LOCK_ERROR_FAIL;
	}

	dbus_error_init(&derror);

	/* Set the DBus rule for the wakeup gesture signal */
	char rules[512] = { 0, };
	snprintf(rules, sizeof(rules) - 1, "path='%s',type='signal',interface='%s', member='%s'", path, interface, member);
	dbus_bus_add_match(connection, rules, &derror);
	if (dbus_error_is_set(&derror)) {
		_E("D-BUS rule adding error: %s", derror.message);
		dbus_error_free(&derror);
		return LOCK_ERROR_FAIL;
	}

	/* Set the callback function */
	if (dbus_connection_add_filter(connection, _dbus_message_recv_cb, NULL, NULL) == FALSE) {
		_E("Failed to add dbus filter : %s", derror.message);
		dbus_error_free(&derror);
		return LOCK_ERROR_FAIL;
	}

	dbus_error_free(&derror);

	return LOCK_ERROR_OK;
}

static lock_error_e _dbus_sig_dettach(const char *path, const char *interface, const char *member)
{
	DBusError err;
	DBusConnection *connection = NULL;

	int ret = LOCK_ERROR_OK;

	retv_if(!path, LOCK_ERROR_INVALID_PARAMETER);
	retv_if(!interface, LOCK_ERROR_INVALID_PARAMETER);
	retv_if(!member, LOCK_ERROR_INVALID_PARAMETER);

	connection = _dbus_connection_get();
	if (!connection) {
		_E("failed to get DBUS connection");
		return LOCK_ERROR_FAIL;
	}

	dbus_error_init(&err);
	dbus_connection_remove_filter(connection, _dbus_message_recv_cb, NULL);

	char rules[512] = { 0, };

	snprintf(rules, sizeof(rules), "path='%s',type='signal',interface='%s',member='%s'", path, interface, member);
	dbus_bus_remove_match(connection, rules, &err);
	if (dbus_error_is_set(&err)) {
		_E("Failed to dbus_bus_remove_match : %s", err.message);
		ret = LOCK_ERROR_FAIL;
	}

	dbus_error_free(&err);

	return ret;
}

int lock_dbus_register_cb(int type, void (*result_cb)(void *, void *), void *result_data)
{
	retv_if(!result_cb, LOCK_ERROR_FAIL);

	dbus_cb_s *cb = calloc(1, sizeof(dbus_cb_s));
	retv_if(!cb, LOCK_ERROR_FAIL);

	cb->result_cb = result_cb;
	cb->result_data = result_data;

	s_info.cbs_list[type] = eina_list_prepend(s_info.cbs_list[type], cb);
	retv_if(!s_info.cbs_list[type], LOCK_ERROR_FAIL);

	return LOCK_ERROR_OK;
}

void lock_dbus_unregister_cb(int type,	void (*result_cb)(void *, void *))
{
	const Eina_List *l;
	const Eina_List *n;
	dbus_cb_s *cb;
	EINA_LIST_FOREACH_SAFE(s_info.cbs_list[type], l, n, cb) {
		continue_if(!cb);
		if (result_cb != cb->result_cb) continue;
		s_info.cbs_list[type] = eina_list_remove(s_info.cbs_list[type], cb);
		free(cb);
		return;
	}
}

static void _lcd_on_cb(void *user_data, void *event_info)
{
	_I("Dbus LCD on");

	lock_time_resume();

	lockscreen_lcd_off_timer_set();
}

static void _lcd_off_cb(void *user_data, void *event_info)
{
	_I("Dbus LCD off");

	lock_time_pause();

	lockscreen_lcd_off_timer_unset();
	lockscreen_lcd_off_count_reset();
}

void lock_dbus_init(void *data)
{
	if (_dbus_sig_attach(DBUS_DEVICED_DISPLAY_PATH,
				DBUS_DEVICED_DISPLAY_INTERFACE,
				DBUS_DEVICED_DISPLAY_MEMBER_LCD_ON) != LOCK_ERROR_OK) {
		_E("Failed to attach LCD on signal filter");
	}

	if (_dbus_sig_attach(DBUS_DEVICED_DISPLAY_PATH,
				DBUS_DEVICED_DISPLAY_INTERFACE,
				DBUS_DEVICED_DISPLAY_MEMBER_LCD_OFF) != LOCK_ERROR_OK) {
		_E("Failed to attach LCD off signal filter");
	}

	if (lock_property_rotation_enabled_get()) {
		if (_dbus_sig_attach(DBUS_ROTATION_PATH,
					DBUS_ROTATION_INTERFACE,
					DBUS_ROTATION_MEMBER_CHANGED) != LOCK_ERROR_OK) {
			_E("Failed to attach rotation signal filter");
		}
	}

	if (lock_dbus_register_cb(DBUS_EVENT_LCD_ON, _lcd_on_cb, NULL) != LOCK_ERROR_OK) {
		_E("Failed to register lcd status changed cb");
	}

	if (lock_dbus_register_cb(DBUS_EVENT_LCD_OFF, _lcd_off_cb, NULL) != LOCK_ERROR_OK) {
		_E("Failed to register lcd status changed cb");
	}
}

void lock_dbus_fini(void *data)
{
	_dbus_sig_dettach(DBUS_DEVICED_DISPLAY_PATH,
			DBUS_DEVICED_DISPLAY_INTERFACE,
			DBUS_DEVICED_DISPLAY_MEMBER_LCD_ON);

	_dbus_sig_dettach(DBUS_DEVICED_DISPLAY_PATH,
			DBUS_DEVICED_DISPLAY_INTERFACE,
			DBUS_DEVICED_DISPLAY_MEMBER_LCD_OFF);

	if (lock_property_rotation_enabled_get()) {
		_dbus_sig_dettach(DBUS_ROTATION_PATH,
				DBUS_ROTATION_INTERFACE,
				DBUS_ROTATION_MEMBER_CHANGED);
	}

	if (s_info.connection != NULL) {
		dbus_connection_close(s_info.connection);
		dbus_connection_unref(s_info.connection);
		s_info.connection = NULL;

		_D("DBUS connection is closed");
	}

	_cbs_fini();

	lock_dbus_unregister_cb(DBUS_EVENT_LCD_ON, _lcd_on_cb);
	lock_dbus_unregister_cb(DBUS_EVENT_LCD_OFF, _lcd_off_cb);
}
