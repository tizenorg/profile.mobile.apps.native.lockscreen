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
 * Lockscreen is the Tizen native application launched on device idle state
 * or power button lock request. It prevents unauthorized device access by
 * locking device with its own window and layout and setting vconf info flags
 * for related applications until the device is properly unlocked. Lockscreen
 * provides multiple lock and unlock device features. Device can be unlocked
 * in following ways:
 * @li unlock device with swipe gesture,
 * @li unlock device with simple 4 number password,
 * @li unlock device with password,
 * @li none - the device is always unlocked.
 *
 * If password is not set, Lockscreen application shuts down immediately after
 * the swipe gesture, otherwise new password view shows up. Depending on type
 * of the lock, password view has pin pad or built in tizen keyboard as entry
 * input. The number of attempts depends on value set by related setting
 * application and authentication password API.
 *
 * One of the major features in Lockscreen application is displaying current
 * time and date, battery capacity percentage, charging and PLMN-SPN status.
 * Additionally Lockscreen application provides shortcut swipe icon for fast
 * application launch - by default it is camera application or none. The available
 * applications might be set in Settings application. Lockscreen supports
 * notifications by displaying information about currently active and providing
 * chosen event handle application launch. Only these notifications that have
 * in app_list variable NOTIFICATION_DISPLAY_APP_LOCK bit set are shown and after
 * device is unlocked this bit is reseted and notification updated. In case when
 * password is needed application is not launched until password is correct and
 * device unlocked.
 *
 * Lockscreen communicates with related applications by setting and reading
 * following vconf flags:
 * @li VCONFKEY_IDLE_LOCK_STATE - whenever this value changes, Lockscreen checks
 * current device lock type. If device does not require password, it is immediately
 * unlocked, otherwise unlock request is sent and proper password view shows up.
 * @li VCONFKEY_LOCKSCREEN_CAMERA_QUICK_ACCESS – describes whether camera for
 * Lockscreen is enabled or not. Depending on its value, camera module is
 * initialized and camera shortcut icon shown.
 * @li VCONFKEY_SETAPPL_SCREEN_LOCK_TYPE_INT – flag needed to get currently set device
 * lock type. There are few available values might be changed in Settings application:
 * - SETTING_SCREEN_LOCK_TYPE_NONE - lockscreen window does not show up,
 * - SETTING_SCREEN_LOCK_TYPE_SWIPE  - device unlocks after swipe gesture,
 * - SETTING_SCREEN_SIMPLE_PASSWORD - after swipe gesture 4 number password is
 *   required to unlock device,
 * - SETTING_SCREEN_PASSWORD - after swipe gesture full password is required to
 *   unlock device.
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
