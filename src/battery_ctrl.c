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

#include <device/battery.h>
#include <device/callback.h>

#include "lockscreen.h"
#include "log.h"
#include "battery_ctrl.h"
#include "battery.h"
#include "main_view.h"

#include <Ecore.h>

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;


// FIXME why this is needed?
static char *_replaceString(char *strInput, const char *strTarget, const char *strChange)
{
	char* strResult; char* strTemp;
	int i = 0;
	int nCount = 0;
	int nTargetLength = strlen(strTarget);

	if (nTargetLength < 1) {
		ERR("there is no target to chnage");
		return NULL;
	}

	int nChangeLength = strlen(strChange);

	if (nChangeLength != nTargetLength) {
		for (i = 0; strInput[i] != '\0';) {
			if (memcmp(&strInput[i], strTarget, nTargetLength) == 0) {
				nCount++;		//consider same string exist
				i += nTargetLength;
			} else {
				i++;
			}
		}
	} else {
		i = strlen(strInput);
	}

	strResult = (char *) malloc(i + 1 + nCount * (nChangeLength - nTargetLength));

	if (!strResult) {
		ERR("fail malloc!!");
		return NULL;
	}

	strTemp = strResult;
	while (*strInput) {
		if (memcmp(strInput, strTarget, nTargetLength) == 0) {
			memcpy(strTemp, strChange, nChangeLength);
			strTemp += nChangeLength;	//move changed length
			strInput  += nTargetLength;	// move target length
		} else {
			*strTemp++ = *strInput++;		// original str cpy
		}
	}

	*strTemp = '\0';

	return strResult;
}

static char *_text_from_percentage(int capacity)
{
	char buff[64];
	char *newString = NULL;
	newString = _replaceString(_("IDS_LCKSCN_BODY_CHARGING_C_PDP"), "%d%", "%d%%");

	if (newString != NULL) {
		snprintf(buff, sizeof(buff), newString , capacity);
		free(newString) ;
	} else {
		snprintf(buff, sizeof(buff), _("IDS_LCKSCN_BODY_CHARGING_C_PDP") , capacity);
	}
	return strdup(buff);
}

static int _battery_update(void)
{
	if (lockscreen_battery_is_charging()) {
		if (lockscreen_battery_level_get() == 100) {
			lockscreen_main_view_battery_status_text_set(main_view, _("IDS_SM_POP_FULLY_CHARGED"));
		} else {
			char *buff = _text_from_percentage(lockscreen_battery_level_get());
			lockscreen_main_view_battery_status_text_set(main_view, buff);
			free(buff);
		}
	} else {
		if (lockscreen_battery_level_get() == 100 && lockscreen_battery_is_connected()) {
			lockscreen_main_view_battery_status_text_set(main_view, _("IDS_SM_POP_FULLY_CHARGED"));
		} else {
			lockscreen_main_view_battery_status_text_set(main_view, NULL);
		}
	}
	return 0;
}

static Eina_Bool _data_battery_update(void *data, int event, void *event_info)
{
	_battery_update();
	return EINA_TRUE;
}

int lock_battery_ctrl_init(Evas_Object *view)
{
	if (lockscreen_battery_init()) {
		FAT("lockscreen_battery_init failed. Battery related information will not be available");
		return 1;
	}
	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_BATTERY_CHANGED, _data_battery_update, NULL);
	if (!handler)
		FAT("ecore_event_handler_add failed on LOCKSCREEN_DATA_MODEL_EVENT_BATTERY_CHANGED event");
	main_view = view;
	_battery_update();
	return 0;
}

void lock_battery_ctrl_fini(void)
{
	ecore_event_handler_del(handler);
}
