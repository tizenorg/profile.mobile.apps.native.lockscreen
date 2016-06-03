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

#ifndef _LOCKSCREEN_SIM_LOCK_H_
#define _LOCKSCREEN_SIM_LOCK_H_

/**
 * @brief Event fired when sim verification succeeds
 */
extern int LOCKSCREEN_EVENT_SIM_LOCK_UNLOCKED;

/**
 * @brief Event fired when sim verification fails
 */
extern int LOCKSCREEN_EVENT_SIM_LOCK_INCORRECT;

/**
 * @brief Event fired when sim card is blocked
 */
extern int LOCKSCREEN_EVENT_SIM_LOCK_BLOCKED;

typedef enum {
	SIM_LOCK_PIN_TYPE_NONE,
	SIM_LOCK_PIN_TYPE_PIN,
	SIM_LOCK_PIN_TYPE_PUK,
	SIM_LOCK_PIN_TYPE_CARD_BLOCKED,
} pin_type_e;

typedef enum {
	SIM_LOCK_NONE = -1,
	SIM_LOCK_SIM1,
	SIM_LOCK_SIM2,
} sim_lock_e;

/**
 * @brief Initalizes sim lock information
 *
 * @return: 0 on success, other value on failure
 */
int lockscreen_sim_lock_init(void);

/**
 * @brief Shutdowns sim lock information
 *
 * @return: 0 on success, other value on failure
 */
void lockscreen_sim_lock_shutdown(void);

/**
 * @brief Request for sim lock unlock
 *
 * @param[in] card_no Card to be unlocked
 * @param[in] pass Pin password
 *
 */
void lockscreen_sim_lock_unlock(sim_lock_e card_no, const char *pass);

/**
 * @brief Gets last requested sim card pin verification attempts left
 *
 * #return: Number of attempts or negative value on failure.
 */
int lockscreen_sim_lock_get_attempts_left(void);

/**
 * @brief Get locked cards info
 *
 * @param[out] sim Sim number
 * @param[out] type The type of first locked card
 *
 * @return: Number of locked cards.
 */
int lockscreen_sim_lock_pin_required(sim_lock_e *sim, pin_type_e *type);

/**
 * @brief Get number of available sim cards.
 *
 * @return: Number of available sim cards
 */
int lockscreen_sim_lock_available_sim_card_count(void);

#endif
