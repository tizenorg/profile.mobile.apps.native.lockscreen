/* * Copyright 2016  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOCKSCREEN_UTIL_H_
#define _LOCKSCREEN_UTIL_H_

#include <Elementary.h>

/**
 * @defgroup Utils Utilities
 *
 * Contains set of time related utility functions that are used by various lockscreen
 * modules.
 */

/**
 * @addtogroup Utils
 * @{
 */

/*
 * @brief Application sub-directories type.
 */
enum app_subdir {
	APP_DIR_DATA,
	APP_DIR_CACHE,
	APP_DIR_RESOURCE,
	APP_DIR_SHARED_DATA,
	APP_DIR_SHARED_RESOURCE,
	APP_DIR_SHARED_TRUSTED,
	APP_DIR_EXTERNAL_DATA,
	APP_DIR_EXTERNAL_CACHE,
	APP_DIR_EXTERNAL_SHARED_DATA,
};

/**
 * @brief Returns absolute path to resource file located in applications directory.
 *
 * @param dir type of subdirectory
 * @param relative path of resource from starting from given sub dir.
 *        eg. for APP_DATA_DIR subdir and relative "database.db" => "/home/owner/apps/org.tizen.lockscreen/data/database.db"
 * @return absolute path string.
 *
 * @note returns statically allocated string
 */
const char *util_get_file_path(enum app_subdir dir, const char *relative);

/**
 * @brief Convinience macros
 */
#define util_get_data_file_path(x) util_get_file_path(APP_DIR_DATA, (x))
#define util_get_cache_file_path(x) util_get_file_path(APP_DIR_CACHE, (x))
#define util_get_res_file_path(x) util_get_file_path(APP_DIR_RESOURCE, (x))
#define util_get_shared_data_file_path(x) util_get_file_path(APP_DIR_SHARED_DATA, (x))
#define util_get_shared_res_file_path(x) util_get_file_path(APP_DIR_SHARED_RESOURCE, (x))
#define util_get_trusted_file_path(x) util_get_file_path(APP_DIR_SHARED_TRUSTED, (x))
#define util_get_external_data_file_path(x) util_get_file_path(APP_DIR_EXTERNAL_DATA, (x))
#define util_get_external_cache_file_path(x) util_get_file_path(APP_DIR_EXTERNAL_CACHE, (x))
#define util_get_external_shared_data_file_path(x) util_get_file_path(APP_DIR_EXTERNAL_SHARED_DATA, (x))

/**
 * @brief utility macro to enumerate
 */
#define SIZE(x) sizeof(x)/sizeof(x[0])

/**
 * @brief Get default lockscreen elementary theme extension.
 * 
 * @return Lockscreen elementary theme extension.
 */
const Elm_Theme *util_lockscreen_theme_get(void);

/**
 * @brief Plays default tap signal
 */
void util_feedback_tap_play(void);

/**
 * @brief Creates notification popup with title, description and "OK" button.
 *
 * Function creates popup with title string in popup header
 * and desc information in content area.
 * Popup is automatically closed after clicking "OK" button.
 *
 * @param obj parent elementary object
 * @param title Popup title
 * @param desc Popup description
 */
void util_popup_create(Evas_Object *obj, const char *title, const char *desc);

/**
 * @}
 */

#endif /* __HOME_SCREEN_UTIL_H__ */
