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

#ifndef __LOG_H__
#define __LOG_H__

#include <dlog.h>
#include <app_i18n.h>

#ifdef LOG_TAG
	#undef LOG_TAG
#endif

#define LOG_TAG "LOCKSCREEN"
#define ENABLE_LOG_SYSTEM

#ifdef ENABLE_LOG_SYSTEM
	#define _E(fmt, arg...)  LOGE("[%s:%d:E] "fmt,  __func__, __LINE__, ##arg)
	#define _D(fmt, arg...)  LOGD("[%s:%d:D] "fmt,  __func__, __LINE__, ##arg)
	#define _W(fmt, arg...) LOGW("[%s:%d:W] "fmt,  __func__, __LINE__, ##arg)
	#define _I(fmt, arg...) LOGI("[%s:%d:I] "fmt,  __func__, __LINE__, ##arg)
	#define _SECURE_E(fmt, arg...)  SECURE_LOGE("["LOG_TAG"%s:%d:E] : %s "fmt, __FILE__, __LINE__, __func__, ##arg)
	#define _SECURE_D(fmt, arg...)  SECURE_LOGD("["LOG_TAG"%s:%d:D] : %s "fmt, __FILE__, __LINE__, __func__, ##arg)
	#define _SECURE_W(fmt, arg...) SECURE_LOGW("["LOG_TAG"%s:%d:W] : %s "fmt, __FILE__, __LINE__, __func__, ##arg)
	#define _SECURE_I(fmt, arg...) SECURE_LOGI("["LOG_TAG"%s:%d:I] : %s "fmt, __FILE__, __LINE__, __func__, ##arg)
#else
	#define _E(fmt, arg...)
	#define _D(fmt, arg...)
	#define _W(fmt, arg...)
	#define _I(fmt, arg...)
	#define _SECURE_E(fmt, arg...)
	#define _SECURE_D(fmt, arg...)
	#define _SECURE_W(fmt, arg...)
	#define _SECURE_I(fmt, arg...)
#endif

#undef _
#define _(str) i18n_get_text(str)

#define retv_if(expr, val) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retm_if(expr, fmt, arg...) do { \
	if(expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define ret_if(expr) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define goto_if(expr, val) do { \
	if(expr) { \
		_E("(%s) -> goto", #expr); \
		goto val; \
	} \
} while (0)

#define break_if(expr) { \
	if(expr) { \
		_E("(%s) -> break", #expr); \
		break; \
	} \
}

#define continue_if(expr) { \
	if(expr) { \
		_E("(%s) -> continue", #expr); \
		continue; \
	} \
}

#endif
