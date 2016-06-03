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

#define CALL_APP_CONTROL_CALL_APP_ID "org.tizen.contacts"

//Until there is no operation set on phone-contacts app, these operation won't be used.
//#define CALL_APP_CONTROL_OPERATION_RETURN "http://tizen.org/appcontrol/operation/return_to_call"
//#define CALL_APP_CONTROL_OPERATION_EMERGENCY "http://tizen.org/appcontrol/operation/emergency_call"

#define ICON_PATH_CALL "quick_call_icon.png"

/**
 * @brief Event fired when call status changes
 */
extern int LOCKSCREEN_EVENT_CALL_STATUS_CHANGED;

/**
 * @brief Sends launch request to phone application
 *
 * @return: 0 on success, other value on failure
 */
int lockscreen_call_app_launch_request(void);

/**
 * @brief Checks status of current calls
 *
 * @return: 1 on active calls, otherwise 0.
 */
int lockscreen_call_active_is(void);

/**
 * @brief Initializes call module
 *
 * @return: 0 on success, other value on failure
 */
int lockscreen_call_init(void);

/**
 * @brief Deinitializes call module
 */
void lockscreen_call_shutdown(void);
