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

#include "main_view.h"
#include "util.h"
#include "log.h"
#include "lockscreen.h"
#include "util_time.h"

#include <Elementary.h>

#define EMG_BUTTON_WIDTH 322
#define PLMN_LABEL_STYLE_START "<style=far_shadow,bottom><shadow_color=#00000033><font_size=24><align=left><color=#FFFFFF><text_class=ATO007><color_class=ATO007><wrap=none>"
#define PLMN_LABEL_STYLE_START_CENTER "<style=far_shadow,bottom><shadow_color=#00000033><font_size=24><align=center><color=#FFFFFF><text_class=ATO007><color_class=ATO007><wrap=none>"
#define PLMN_LABEL_STYLE_END "</wrap></color_class></text_class></color></align></font_size></shadow_color></style>"

typedef enum {
	TEXT_TEMPLATE_TIME_24H = 0,
	TEXT_TEMPLATE_TIME_AMPM,
	TEXT_TEMPLATE_TIME_AMPM_KOREAN,
} time_template_e;

typedef enum {
	TEXT_TEMPLATE_DATE = 0,
} datee_template_e;

typedef enum {
	TEXT_TEMPLATE_DATETIME_24H = 0,
	TEXT_TEMPLATE_DATETIME_AMPM,
	TEXT_TEMPLATE_DATETIME_AMPM_KOREAN,
} datetime_template_e;

static const char *time_templates[] = {
	[TEXT_TEMPLATE_TIME_24H] = "%1$s",
	[TEXT_TEMPLATE_TIME_AMPM] = "%1$s <small_font>%2$s</>",
	[TEXT_TEMPLATE_TIME_AMPM_KOREAN] = "<small_font>%2$s</> %1$s",
};

static const char *date_templates[] = {
	[TEXT_TEMPLATE_DATE] = "<small_font>%1$s</>",
};

static const char *datetime_templates[] = {
	[TEXT_TEMPLATE_DATETIME_24H] = "%1$s %2$s",
	[TEXT_TEMPLATE_DATETIME_AMPM] = "%1$s %3$s %2$s",
	[TEXT_TEMPLATE_DATETIME_AMPM_KOREAN] = "%3$s %1$s %2$s",
};

static Evas_Object *_swipe_layout_create(Evas_Object *parent)
{
	Evas_Object *swipe_layout = NULL;
	if (!parent) return NULL;

	swipe_layout = elm_layout_add(parent);

	if (!elm_layout_file_set(swipe_layout, util_get_res_file_path(LOCK_EDJE_FILE), "swipe-lock")) {
		ERR("elm_layout_file_set failed.");
		evas_object_del(swipe_layout);
		return NULL;
	}

	evas_object_size_hint_weight_set(swipe_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(swipe_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(swipe_layout);

	return swipe_layout;
}

static void _lockscreen_main_view_swipe_plmn_center_set(Evas_Object *swipe_layout, bool set)
{
	char buf[512] = {0, };

	Evas_Object *label = elm_object_part_content_get(swipe_layout, "txt.plmn");
	if (!label)
		return;

	char *utf8_text = elm_entry_markup_to_utf8(elm_object_text_get(label));

	if (set) {
		snprintf(buf, sizeof(buf), "%s%s%s", PLMN_LABEL_STYLE_START_CENTER, utf8_text, PLMN_LABEL_STYLE_END);
		elm_object_signal_emit(swipe_layout, "show,txt,plmn,center", "lockscreen");
	} else {
		snprintf(buf, sizeof(buf), "%s%s%s", PLMN_LABEL_STYLE_START, utf8_text, PLMN_LABEL_STYLE_END);
		elm_object_signal_emit(swipe_layout, "show,txt,plmn", "lockscreen");
	}

	elm_object_text_set(label, buf);

	free(utf8_text);

	return ;
}

static void _lockscreen_main_view_swipe_part_content_set(Evas_Object *swipe_layout, const char *part, Evas_Object *content)
{
	if (!swipe_layout) {
		return;
	}

	if (!strcmp(PART_EVENTS, part)) {
		evas_object_propagate_events_set(content, EINA_FALSE);
		elm_object_signal_emit(swipe_layout, "contextual,events,show", "lockscreen");
	} else if (!strcmp(PART_SHORTCUT, part)) {
		evas_object_propagate_events_set(content, EINA_FALSE);
	} else if (!strcmp(PART_CALL, part)) {
		evas_object_propagate_events_set(content, EINA_FALSE);
		_lockscreen_main_view_swipe_plmn_center_set(swipe_layout, true);
	}

	elm_object_part_content_set(swipe_layout, part, content);
}

void lockscreen_main_view_part_content_set(Evas_Object *view, const char *part, Evas_Object *content)
{
	if (!part) return;
	Evas_Object *sl = elm_object_part_content_get(view, "sw.swipe_layout");

	if (!strcmp(part, PART_SHORTCUT) || !strcmp(part, PART_EVENTS) ||
			!strcmp(part, PART_CALL)) {
		_lockscreen_main_view_swipe_part_content_set(sl, part, content);
	} else if (!strcmp(part, PART_PASSWORD)) {
		elm_object_signal_emit(sl, "unlock,anim,start", "lockscreen");
		elm_object_part_content_set(view, part, content);
	} else if (!strcmp(part, PART_SIMLOCK)) {
		elm_object_signal_emit(view, "simlock,show", "lockscreen");
		elm_object_part_content_set(view, part, content);
	}
}

static Evas_Object*
_lockscreen_main_view_swipe_part_content_unset(Evas_Object *swipe_layout, const char *part)
{
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return false;
	}
	if (!strcmp(PART_EVENTS, part))
		elm_object_signal_emit(swipe_layout, "contextual,events,hide", "lockscreen");
	else if (!strcmp(part, PART_CALL))
		_lockscreen_main_view_swipe_plmn_center_set(swipe_layout, false);

	return elm_object_part_content_unset(swipe_layout, part);
}

Evas_Object *lockscreen_main_view_part_content_unset(Evas_Object *view, const char *part)
{
	if (!part) return NULL;
	Evas_Object *sl = elm_object_part_content_get(view, "sw.swipe_layout");
	if (!strcmp(part, PART_SHORTCUT) || !strcmp(part, PART_EVENTS)
			|| !strcmp(part, PART_CALL))
		return _lockscreen_main_view_swipe_part_content_unset(sl, part);
	if (!strcmp(part, PART_PASSWORD)) {
		elm_object_signal_emit(sl, "lock,anim,start", "lockscreen");
		return elm_object_part_content_unset(view, PART_PASSWORD);
	}
	if (!strcmp(part, PART_SIMLOCK)) {
		elm_object_signal_emit(view, "simlock,hide", "lockscreen");
		return elm_object_part_content_unset(view, part);
	}

	return NULL;
}

Evas_Object *lockscreen_main_view_part_content_get(Evas_Object *view, const char *part)
{
	if (!part) return NULL;
	if (!strcmp(part, PART_SHORTCUT) || !strcmp(part, PART_EVENTS) || !strcmp(part, PART_CALL))
		return elm_object_part_content_get(elm_object_part_content_get(view, "sw.swipe_layout"), part);
	if (!strcmp(part, PART_PASSWORD)) {
		return elm_object_part_content_get(view, part);
	}

	return NULL;
}

static Evas_Event_Flags _swipe_state_end(void *data, void *event_info)
{
	evas_object_smart_callback_call(data, SIGNAL_SWIPE_GESTURE_FINISHED, NULL);
	return EVAS_EVENT_FLAG_NONE;
}

Evas_Object *lockscreen_main_view_create(Evas_Object *win)
{
	Evas_Object *layout = elm_layout_add(win);
	if (!elm_layout_file_set(layout, util_get_res_file_path(LOCK_EDJE_FILE), "lockscreen")) {
		FAT("Failed to set edje file for main view.");
		return NULL;
	}

	evas_object_show(layout);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *swipe_layout = _swipe_layout_create(layout);
	if (!swipe_layout) {
		evas_object_del(layout);
		return NULL;
	}
	elm_object_part_content_set(layout, "sw.swipe_layout", swipe_layout);

	Evas_Object *gesture_layer = elm_gesture_layer_add(layout);
	elm_gesture_layer_hold_events_set(gesture_layer, EINA_TRUE);
	elm_gesture_layer_attach(gesture_layer, swipe_layout);
	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _swipe_state_end, layout);
	elm_gesture_layer_flick_time_limit_ms_set(gesture_layer, 500);
	// set minimum swipe length scaled by edje scale factor
	elm_gesture_layer_line_min_length_set(gesture_layer, 140 * edje_scale_get() / edje_object_scale_get(elm_layout_edje_get(layout)));
	evas_object_show(gesture_layer);

	return layout;
}

void lockscreen_main_view_battery_status_text_set(Evas_Object *view, const char *battery)
{
	Evas_Object *swipe_layout = elm_object_part_content_get(view, "sw.swipe_layout");
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return;
	}
	if (battery) {
		elm_object_part_text_set(swipe_layout, "txt.battery", battery);
		elm_object_signal_emit(swipe_layout, "show,txt,battery", "lockscreen");
	} else {
		elm_object_signal_emit(swipe_layout, "hide,txt,battery", "lockscreen");
		elm_object_part_text_set(swipe_layout, "txt.battery", "");
	}
}

void lockscreen_main_view_sim_status_text_set(Evas_Object *view, const char *text)
{
	Evas_Object *swipe_layout = elm_object_part_content_get(view, "sw.swipe_layout");
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return;
	}
	Evas_Object *label = NULL;
	char buf[512] = { 0, };
	char *markup_text = NULL;
	Evas_Object *tb = NULL;
	Evas_Coord tb_w = 0;

	if (!text) {
		elm_object_signal_emit(swipe_layout, "hide,txt,plmn", "lockscreen");
		return;
	}

	label = lockscreen_main_view_part_content_get(swipe_layout, "txt.plmn");
	if (!label) {
		label = elm_label_add(swipe_layout);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_scale_set(label, edje_scale_get() * 0.6);

		elm_object_style_set(label, "slide_short");
		elm_label_wrap_width_set(label, 100);
		elm_label_ellipsis_set(label, EINA_TRUE);
		elm_label_slide_duration_set(label, 2);
		elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);

		Evas_Object *label_edje = elm_layout_edje_get(label);
		tb = (Evas_Object *)edje_object_part_object_get(label_edje, "elm.text");
		if (!tb)
			FAT("elm.text part not found in edje");

		evas_object_textblock_size_native_get(tb, &tb_w, NULL);

		if ((tb_w > 0) && (tb_w > ELM_SCALE_SIZE(EMG_BUTTON_WIDTH)))
			elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);

		elm_label_slide_go(label);

		elm_object_part_content_set(swipe_layout, "txt.plmn", label);
		elm_object_signal_emit(swipe_layout, "show,txt,plmn", "lockscreen");
		evas_object_show(label);
	}

	markup_text = elm_entry_utf8_to_markup(text);
	snprintf(buf, sizeof(buf), "%s%s%s", PLMN_LABEL_STYLE_START, markup_text, PLMN_LABEL_STYLE_END);

	elm_object_text_set(label, buf);
}

static void _layout_unlocked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *swipe_layout = elm_object_part_content_get(data, "sw.swipe_layout");
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return;
	}
	evas_object_smart_callback_call(data, SIGNAL_UNLOCK_ANIMATION_FINISHED, NULL);
	elm_object_signal_callback_del(swipe_layout, "unlock,anim,end", "swipe-layout", _layout_unlocked);
}

void lockscreen_main_view_unlock(Evas_Object *view)
{
	Evas_Object *swipe_layout = elm_object_part_content_get(view, "sw.swipe_layout");
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return;
	}
	elm_object_signal_callback_add(swipe_layout, "unlock,anim,end", "swipe-layout", _layout_unlocked, view);
	elm_object_signal_emit(swipe_layout, "unlock,anim,start", "lockscreen");
}

static int _is_korea_locale(const char *locale)
{
	int ret = 0;
	if (locale) {
		if (strstr(locale,"ko_KR")) {
			ret = 1;
		}
	}
	return ret;
}

void lockscreen_main_view_time_set(Evas_Object *view, const char *locale, const char *timezone, bool use24hformat, time_t time)
{
	Evas_Object *swipe_layout = elm_object_part_content_get(view, "sw.swipe_layout");
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return;
	}
	char buf[PATH_MAX] = {0,};
	char *str_date, *str_time, *str_meridiem;
	time_template_e time_tmpl;
	datetime_template_e datetime_tmpl;

	if (!util_time_formatted_time_get(time, locale, timezone, use24hformat, &str_time, &str_meridiem)) {
		ERR("util_time_formatted_time_get failed");
		return;
	}
	if (!util_time_formatted_date_get(time, locale, timezone, "MMMMEEEEd", &str_date)) {
		ERR("util_time_formatted_date_get failed");
		free(str_time);
		free(str_meridiem);
		return;
	}

	if (use24hformat) {
		time_tmpl = TEXT_TEMPLATE_TIME_24H;
		datetime_tmpl = TEXT_TEMPLATE_DATETIME_24H;
	} else {
		if (_is_korea_locale(locale)) {
			time_tmpl = TEXT_TEMPLATE_TIME_AMPM_KOREAN;
			datetime_tmpl = TEXT_TEMPLATE_DATETIME_AMPM_KOREAN;
		} else {
			time_tmpl = TEXT_TEMPLATE_TIME_AMPM;
			datetime_tmpl = TEXT_TEMPLATE_DATETIME_AMPM;
		}
	}

	snprintf(buf, sizeof(buf), time_templates[time_tmpl], str_time, str_meridiem);
	elm_object_part_text_set(swipe_layout, "txt.time", buf);

	snprintf(buf, sizeof(buf), date_templates[TEXT_TEMPLATE_DATE], str_date);
	elm_object_part_text_set(swipe_layout, "txt.date", buf);

	snprintf(buf, sizeof(buf), datetime_templates[datetime_tmpl], str_time, str_date, str_meridiem);
	elm_object_part_text_set(swipe_layout, "txt.timedate", buf);

	free(str_date);
	free(str_time);
	free(str_meridiem);
}

void lockscreen_main_view_contextual_view_fullscreen_set(Evas_Object *view, bool fullscreen)
{
	Evas_Object *swipe_layout = elm_object_part_content_get(view, "sw.swipe_layout");
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return;
	}
	if (fullscreen) {
		elm_object_signal_emit(swipe_layout, "contextual,events,fullscreen,show", "lockscreen");
	} else {
		elm_object_signal_emit(swipe_layout, "contextual,events,fullscreen,hide", "lockscreen");
	}
}

void lockscreen_main_view_unlock_state_set(Evas_Object *view, bool top, bool bottom)
{
	Evas_Object *swipe_layout = elm_object_part_content_get(view, "sw.swipe_layout");
	if (!swipe_layout) {
		FAT("No sw.swipe_layout part");
		return;
	}
	if (top)
		elm_object_signal_emit(swipe_layout, "unlock,top,anim,start", "lockscreen");
	else
		elm_object_signal_emit(swipe_layout, "lock,top,anim,start", "lockscreen");

	if (bottom)
		elm_object_signal_emit(swipe_layout, "unlock,bottom,anim,start", "lockscreen");
	else
		elm_object_signal_emit(swipe_layout, "lock,bottom,anim,start", "lockscreen");
}
