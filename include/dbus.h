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

#ifndef __DBUS_H__
#define __DBUS_H__

typedef enum {
	DBUS_EVENT_LCD_ON = 0,
	DBUS_EVENT_LCD_OFF,
	DBUS_EVENT_ANGLE_CHANGED,
	DBUS_EVENT_MAX,
} dbus_event_type_e;

/* DBUS interfaces and signals */
#define DBUS_COORD_INTEFACE "org.tizen.system.coord"
#define DBUS_ROTATION_PATH "/Org/Tizen/System/Coord/Rotation"
#define DBUS_ROTATION_INTERFACE DBUS_COORD_INTEFACE".rotation"
#define DBUS_ROTATION_MEMBER_CHANGED "Changed"
#define DBUS_ROTATION_METHOD_DEGREE "Degree"

#define DBUS_LOW_BATTERY_PATH "/Org/Tizen/System/Popup/Lowbat"
#define DBUS_LOW_BATTERY_INTERFACE "org.tizen.system.popup.Lowbat"
#define DBUS_LOW_BATTERY_MEMBER_EXTREME_LEVEL "Extreme"

#define DBUS_DEVICED_BUS_NAME "org.tizen.system.deviced"
#define DBUS_DEVICED_PATH "/Org/Tizen/System/DeviceD"
#define DBUS_DEVICED_INTERFACE DBUS_DEVICED_BUS_NAME

/* deviced::display */
#define DBUS_DEVICED_DISPLAY_PATH DBUS_DEVICED_PATH"/Display"
#define DBUS_DEVICED_DISPLAY_INTERFACE DBUS_DEVICED_INTERFACE".display"
#define DBUS_DEVICED_DISPLAY_MEMBER_LCD_ON "LCDOn"
#define DBUS_DEVICED_DISPLAY_MEMBER_LCD_OFF "LCDOff"
#define DBUS_DEVICED_DISPLAY_MEMBER_LCD_ON_BY_POWERKEY "LCDOnByPowerkey"
#define DBUS_DEVICED_DISPLAY_METHOD_LCD_OFF "PowerKeyLCDOff"
#define DBUS_DEVICED_DISPLAY_METHOD_CHANGE_STATE "changestate"
#define DBUS_DEVICED_DISPLAY_METHOD_CUSTOM_LCD_ON "CustomLCDOn"
#define DBUS_DEVICED_DISPLAY_COMMAND_LCD_ON "lcdon"

int lock_dbus_register_cb(int type, void (*result_cb)(void *, void *), void *result_data);
void lock_dbus_unregister_cb(int type,	void (*result_cb)(void *, void *));

void lock_dbus_init(void *data);
void lock_dbus_fini(void *data);

#endif
