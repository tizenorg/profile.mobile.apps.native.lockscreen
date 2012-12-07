/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>
#include <sys/types.h>
#include <unistd.h>
#include <security-server.h>
#include <mdm.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "log.h"
#include "password-verification.h"

typedef enum {
	NORMAL_PASSWORD = 0,
	EMPTY_PASSWORD,
	OVERLENGTH_PASSWORD,
}lockscreen_password_type;

typedef struct {
	unsigned int current_attempt;
	unsigned int max_attempt;
	unsigned int expire_sec;
	int recoverable;
	int mdm_quality;
	password_operation_cb callback;
	void *data;
} lockscreen_password_policy;

static Eina_Bool __password_verification_check_pw_expired(void *data)
{
	lockscreen_password_policy *password_policy = (lockscreen_password_policy *)data;
	if (!password_policy) {
		return EINA_FALSE;
	}

	LOCK_SCREEN_TRACE_DBG("Checking pw expired => valid_secs:%d:%d", password_policy->expire_sec);

	if (password_policy->expire_sec == 0) {
		return EINA_TRUE;
	} else
		return EINA_FALSE;
}

static Eina_Bool __password_verification_check_recovery(void *data)
{
	lockscreen_password_policy *password_policy = (lockscreen_password_policy *)data;
	if (!password_policy) {
		return EINA_FALSE;
	}

	LOCK_SCREEN_TRACE_DBG("Checking recovery => current_attempt:%d max_attempt:%d",
		password_policy->current_attempt, password_policy->max_attempt);

	if ((password_policy->max_attempt != 0) && (password_policy->max_attempt == password_policy->current_attempt)) 	{
		password_policy->recoverable = 1;
		return EINA_TRUE;
	} else
		return EINA_FALSE;
}

static Eina_Bool __password_verification_check_phone_password(void *data, const char *str)
{
	LOCK_SCREEN_TRACE_DBG("%s : %s\n", __FUNCTION__, str);

	lockscreen_password_policy *lock_policy = (lockscreen_password_policy *)data;

	int ret = SECURITY_SERVER_API_ERROR_PASSWORD_MISMATCH;

	unsigned int current_attempt = 0;
	unsigned int max_attempt = 0;
	unsigned int valid_secs = 0;

	if(!lock_policy) {
		LOCK_SCREEN_TRACE_DBG("lock_policy is NULL");
		return EINA_FALSE;
	}

	ret = security_server_chk_pwd(str, &current_attempt, &max_attempt, &valid_secs);
	LOCK_SCREEN_TRACE_DBG("security_server_chk_pwd => ret:%d current_attempt:%d max_attempt:%d valid_secs:%d",
		ret, current_attempt, max_attempt, valid_secs);
	lock_policy->current_attempt = current_attempt;
	lock_policy->max_attempt = max_attempt;
	lock_policy->expire_sec = valid_secs;
	if ((ret == SECURITY_SERVER_API_SUCCESS) || (ret == SECURITY_SERVER_API_ERROR_PASSWORD_EXPIRED)) {
		LOCK_SCREEN_TRACE_DBG("correct password!");
		return EINA_TRUE;
	} else {
		LOCK_SCREEN_TRACE_DBG("incorrect password!");
		return EINA_FALSE;
	}
}

static lockscreen_password_type __password_verification_check_length(const char *str, int min, int max)
{
	int len = 0;

	if (!str) {
		return EMPTY_PASSWORD;
	}

	len = strlen(str);

	LOCK_SCREEN_TRACE_DBG("%s() len : %d", __FUNCTION__, len);

	if (len == 0) {
		return EMPTY_PASSWORD;
	}

	if (len < min || len > max) {
		return OVERLENGTH_PASSWORD;
	}

	return NORMAL_PASSWORD;
}

static void __password_verification_set_expired_password_by_mdm(void *data)
{
	lockscreen_password_policy *password_policy = (lockscreen_password_policy *)data;

	if(!password_policy) {
		LOCK_SCREEN_TRACE_DBG("lock_policy is NULL");
		return;
	}

	if(mdm_get_service() == MDM_RESULT_SUCCESS) {
		LOCK_SCREEN_TRACE_DBG("success to connect mdm server..!!");
		if (mdm_enforce_password_change() == MDM_RESULT_SUCCESS) {
			LOCK_SCREEN_TRACE_DBG("success to call mdm_enforce_password_change..!!");
		} else {
			LOCK_SCREEN_TRACE_DBG("[ERROR] failed to call mdm_enforce_password_change..!!");
		}

		if (mdm_set_password_status(MDM_PASSWORD_EXPIRED) == MDM_RESULT_SUCCESS) {
			LOCK_SCREEN_TRACE_DBG("success to set MDM_PASSWORD_EXPIRED..!!");
		} else {
			LOCK_SCREEN_TRACE_DBG("[ERROR] failed to set MDM_PASSWORD_EXPIRED..!!");
		}
		mdm_release_service();
	} else {
		LOCK_SCREEN_TRACE_DBG("[ERROR] failed to connect mdm server");
	}
}

static void __password_verification_recorver_password_by_mdm(void *data)
{
	lockscreen_password_policy *password_policy = (lockscreen_password_policy *)data;

	if(!password_policy) {
		LOCK_SCREEN_TRACE_DBG("lock_policy is NULL");
		return;
	}

	if(mdm_get_service() == MDM_RESULT_SUCCESS) {
		LOCK_SCREEN_TRACE_DBG("success to connect mdm server..!!");
		mdm_data_t * lp_data = NULL;
		mdm_password_policy_t *pwd_info = NULL;
		lp_data = mdm_get_password_policy();
		if (lp_data != NULL) {
			pwd_info = (mdm_password_policy_t *)lp_data->data;
			password_policy->mdm_quality = pwd_info->quality;
			LOCK_SCREEN_TRACE_DBG("mdm quality : %d", password_policy->mdm_quality);
			lp_data->mem_header.free_func(lp_data);
		} else {
			LOCK_SCREEN_TRACE_DBG("set password policy fail");
		}
		if (password_policy->mdm_quality != MDM_PASSWORD_QUALITY_UNSPECIFIED) {
			if (mdm_set_password_status(MDM_PASSWORD_FAILED) == MDM_RESULT_SUCCESS) {
				LOCK_SCREEN_TRACE_DBG("success to set MDM_PASSWORD_FAILED..!!");
			} else {
				LOCK_SCREEN_TRACE_DBG("[ERROR] failed to set MDM_PASSWORD_FAILED..!!");
			}
		}
		mdm_release_service();
	}else {
		LOCK_SCREEN_TRACE_DBG("[ERROR] failed to connect mdm server");
	}
}

void *password_verification_policy_create()
{
	lockscreen_password_policy *password_policy = NULL;
	int ret = 0;
	unsigned int cur_attempt = 0;
	unsigned int max_attempt = 0;
	unsigned int expire_sec = 0;

	password_policy = (lockscreen_password_policy *) calloc(1, sizeof(lockscreen_password_policy));

	if (!password_policy){
		return NULL;
	}

	password_policy->mdm_quality = MDM_PASSWORD_QUALITY_UNSPECIFIED;

	ret = security_server_is_pwd_valid(&cur_attempt, &max_attempt, &expire_sec);
	LOCK_SCREEN_TRACE_DBG("policy status:%d, cur_attempt:%d, max_attempt:%d ", ret, cur_attempt, max_attempt);

	if(ret == SECURITY_SERVER_API_ERROR_NO_PASSWORD) {
		password_policy->current_attempt = cur_attempt;
		password_policy->max_attempt = max_attempt;
		password_policy->recoverable = EINA_FALSE;
		password_policy->expire_sec = expire_sec;
	}

	return password_policy;
}

void password_verification_policy_destroy(void *data)
{
	lockscreen_password_policy *password_policy = (lockscreen_password_policy *)data;

	if (!password_policy){
		return;
	}

	free(password_policy);
	password_policy = NULL;
}

void password_verification_verify(void *data, const char *password)
{
	lockscreen_password_type password_type = NORMAL_PASSWORD;
	lockscreen_password_policy *password_policy = (lockscreen_password_policy *)data;

	if(!password_policy) {
		LOCK_SCREEN_TRACE_DBG("lock_policy is NULL");
		return;
	}
	password_type = __password_verification_check_length(password, 4, 16);
	switch(password_type){
		case NORMAL_PASSWORD:
			if (__password_verification_check_phone_password(password_policy, password)) {
				if ( __password_verification_check_pw_expired(password_policy)) {
					__password_verification_set_expired_password_by_mdm(password_policy);
				}
				LOCK_SCREEN_TRACE_DBG("password is right!!!!");
				elm_exit();
			} else {
				if (__password_verification_check_recovery (password_policy)) {
					LOCK_SCREEN_TRACE_DBG("Recovery lock mode..!!");
					__password_verification_recorver_password_by_mdm(password_policy);
				}
				if(password_policy->callback){
					password_policy->callback("", password_policy->data);
				}
			}
			break;
		case EMPTY_PASSWORD:
			password_policy->callback("empty", password_policy->data);
			break;
		case OVERLENGTH_PASSWORD:
			password_policy->callback("overlength", password_policy->data);
			break;
	}
}

void password_verification_callback_set(void *data, password_operation_cb callback, void *priv)
{
	lockscreen_password_policy *password_policy = (lockscreen_password_policy *)data;

	if (!password_policy){
		return;
	}

	password_policy->callback = callback;
	password_policy->data = priv;
}
