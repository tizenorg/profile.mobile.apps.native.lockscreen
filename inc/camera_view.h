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

#ifndef _LOCKSCREEN_CAMERA_VIEW_H_
#define _LOCKSCREEN_CAMERA_VIEW_H_

#include <Elementary.h>

/**
 * @brief Smart signal emitted when camera icon is being selected.
 */
#define SIGNAL_CAMERA_SELECTED "camera,icon,selected"

/**
 * @brief Creates camera view
 *
 * @note parent should be Elementary widget.
 */
Evas_Object *lockscreen_camera_view_create(Evas_Object *parent);

/**
 * @brief Resets camera view.
 *
 * Show unclicked camera button.
 */
void lockscreen_camera_view_reset(Evas_Object *parent);

#endif