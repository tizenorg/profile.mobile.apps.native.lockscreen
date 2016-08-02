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

/**
 * @mainpage Lockscreen documentation
 *
 * @section desc Detailed description
 *
 * @image html lockscreen.png
 *
 * Lockscreen is the Tizen native application launched on device idle state
 * or power button lock request. It prevents unauthorized device access by
 * locking device with its own window and layout and setting vconf info flags
 * for related applications until the device is properly unlocked. Lockscreen
 * provides multiple lock and unlock device features. Depending on value set
 * in Settings application device can be unlocked in following ways:
 * @li unlock device with swipe gesture,
 * @li unlock device with simple 4 number password,
 * @li unlock device with password,
 * @li none - the device is always unlocked.
 * If password is not set, Lockscreen application shuts down immediately after
 * the swipe gesture, otherwise new password view shows up.
 *
 * Depending on type of the lock, password view has pin pad or built in tizen
 * keyboard as entry input. The number of attempts depends on value set by
 * related setting application and authentication password API.
 *
 * @subsection features Major features
 *
 * One of the major features in Lockscreen application is displaying current
 * time and date updated on system settings key value change such as:
 * @li SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR,
 * @li SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE,
 * @li SYSTEM_SETTINGS_KEY_TIME_CHANGED.
 *
 * Additionally it displays battery capacity percentage, charging and PLMN-SPN
 * status thats updates on following device key value change:
 * @li DEVICE_CALLBACK_BATTERY_CAPACITY,
 * @li DEVICE_CALLBACK_BATTERY_CHARGING.
 *
 * Registered callbacks are called every time when one of the above info data
 * changes. The current device system time and date might be changed in Settings
 * application.
 *
 * @subsection shorcuts Application shortcut
 *
 * Lockscreen application provides shortcut swipe icon for fast application
 * launch - for now it is camera application or none. To enable camera shortcut
 * it is needed to set VCONFKEY_LOCKSCREEN_CAMERA_QUICK_ACCESS. Depending on this
 * value, camera module is initialized and camera shortcut icon shown.
 *
 * Example of how to enable camera shorcut on lockscreen view:
 *
 * @code
 * 
 * if(vconf_set_bool(VCONFKEY_LOCKSCREEN_CAMERA_QUICK_ACCESS, 1))
 * 	fprintf(stderr, "vconf_set_int FAIL\n");
 * else
 * 	...
 *
 * @endcode
 *
 * @subsection minicontrol Minicotrollers
 *
 * Minicontrol is small application view that might be shown on the Lokscreen main view.
 * They are displaying layout of the application that is currently running in the background
 * such as music-player, etc. To create minicontol it is necessary to add required privilege:
 *
 * @code
 *
 * http://tizen.org/privilege/minicontrol.provider
 *
 * @endcode
 *
 * Here is a simple example of how to create minicontrol window on lockscreen view:
 *
 * @code
 *
 * #include <minicontrol_provider.h>
 *
 * Evas_Object *win;
 * win = minicontrol_create_window("mini-sample", MINICONTROL_TARGET_VIEWER_STOCK_LOCK_SCREEN, NULL);
 * evas_object_resize(win, LOCKSCREEN_MINICONTROLLER_WIDTH, LOCKSCREEN_MINICONTROLLER_HEIGHT);
 * evas_object_show(win);
 *
 * @endcode
 *
 * @subsection noti Notifications
 *
 * @image html lockscreen_noti.png
 *
 * Lockscreen supports notifications by displaying information about currently active
 * and providing chosen event handle application launch. Only these notifications that
 * have in app_list variable NOTIFICATION_DISPLAY_APP_LOCK bit set are shown and after
 * device is unlocked this bit is reseted and notification updated so it is not shown
 * in lockscreen view anymore. In case when password is needed application is not launched
 * until password is correct and device unlocked.
 *
 * The notification on lockscreen will be shown after required
 *
 * @code
 *
 * http://tizen.org/privilege/notification
 *
 * @endcode
 *
 * is added and following code executed:
 *
 * @code
 *
 * #include <notification.h>
 *
 * notification_h notification = NULL;
 * notification = notification_create(NOTIFICATION_TYPE_ONGOING);
 * if (notification != NULL)
 * 	// Notification was initialized successfully
 *
 * int ret = notification_set_display_applist(notification,
 * 			NOTIFICATION_DISPLAY_APP_LOCK | NOTIFICATION_DISPLAY_APP_TICKER);
 * if (ret != NOTIFICATION_ERROR_NONE)
 * 	// Error handling
 * 
 * ...
 *
 * ret = notification_post(notificaiton);
 * if (ret != NOTIFICATION_ERROR_NONE)
 * 	// Error handling
 *
 * @endcode
 *
 * @subsection lock_type Setting lock type
 *
 * Lock type is set by changing VCONFKEY_SETAPPL_SCREEN_LOCK_TYPE_INT value. There are few available:
 * - SETTING_SCREEN_LOCK_TYPE_NONE - lockscreen window does not show up,
 * - SETTING_SCREEN_LOCK_TYPE_SWIPE  - device unlocks after swipe gesture,
 * - SETTING_SCREEN_SIMPLE_PASSWORD - after swipe gesture 4 number password is required to unlock device,
 * - SETTING_SCREEN_PASSWORD - after swipe gesture full password is required to unlock device.
 *
 * Here is a short hint how to set lock type properly:
 *
 * @code
 *
 * if(vconf_set_int(VCONFKEY_SETAPPL_SCREEN_LOCK_TYPE_INT, SETTING_SCREEN_LOCK_TYPE_SWIPE))
 * 	fprintf(stderr, "vconf_set_int FAIL\n");
 * else
 * 	...
 *
 * @endcode
 *
 * @subsection unlock Unlocking device
 *
 * Lockscreen communicates with related applications by setting and reading following
 * vconf flag:
 * VCONFKEY_IDLE_LOCK_STATE - whenever this value changes, Lockscreen checks current
 * device lock type. If device does not require password, it is immediately unlocked,
 * otherwise unlock request is sent and proper password view shows up.
 *
 * Example of the device unlock request:
 *
 * @code
 *
 * if(vconf_set_bool(VCONFKEY_IDLE_LOCK_STATE, 0))
 * 	fprintf(stderr, "vconf_set_int FAIL\n");
 * else
 * 	...
 *
 * @endcode
 *
 */

#ifndef _LOCKSCREEN_H_
#define _LOCKSCREEN_H_

#define EDJE_DIR "edje/"
#define IMAGE_DIR "images/"

/**
 * @defgroup Models Models
 */

/**
 * @defgroup Views Views
 */

/**
 * @defgroup Controllers Controllers
 */

#define LOCK_EDJE_FILE EDJE_DIR"lockscreen.edj"

#endif
