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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#define APP_NAME_CAMERA "org.tizen.camera-app"

typedef enum {
	CAMERA_VIEW_DRAGGING_START = 1,
	CAMERA_VIEW_DRAGGING_STOP = 2,
	CAMERA_VIEW_ROTATE = 3,
	CAMERA_VIEW_MAX,
} camera_view_type_e;

Evas_Object *lock_camera_layout_get(void);

lock_error_e lock_camera_above_win_state_send(Eina_Bool state);
void lock_camera_app_launch(void);
lock_error_e lock_camera_view_action(camera_view_type_e action, int angle);
Evas_Object *lock_camera_layout_create(Evas_Object *parent);
void lock_camera_layout_destroy(void);

#endif
