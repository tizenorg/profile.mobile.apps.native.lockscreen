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

#ifndef _LOCKSCREEN_SIM_H_
#define _LOCKSCREEN_SIM_H_

/**
 * @brief Event fired when sim information changes
 */
extern int LOCKSCREEN_EVENT_SIM_STATUS_CHANGED;

/**
 * @brief Initializes sim information changes
 *
 * @return: 0 on success, other value on failure.
 */
int lockscreen_sim_init();

typedef enum {
	LOCKSCREEN_PRIMARY_SIM = 0,
	LOCKSCREEN_SECONDARY_SIM,
	LOCKSCREEN_SIM_MAX,
} lockscreen_sim_num_e;

/**
 * @brief Shutdowns sim information changes.
 */
void lockscreen_sim_shutdown(void);

/**
 * @brief Get current PLMN information for given sim
 */
const char *lockscreen_sim_get_plmn(lockscreen_sim_num_e num);

#endif
