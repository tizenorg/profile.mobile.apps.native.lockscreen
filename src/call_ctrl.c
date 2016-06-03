#include <Elementary.h>

#include "log.h"
#include "util.h"
#include "call.h"
#include "password_view.h"
#include "main_view.h"
#include "swipe_icon.h"

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;

static Eina_Bool _call_status_changed_cb(void *data, int type, void *event_info)
{
	DBG("Call status changed");

	Evas_Object *pass_view = NULL;

	if (!main_view) {
		ERR("main_view == NULL");
		return ECORE_CALLBACK_PASS_ON;
	}

	pass_view = lockscreen_main_view_part_content_get(main_view, PART_PASSWORD);
	if (!pass_view) {
		ERR("pass_view == NULL");
		return ECORE_CALLBACK_PASS_ON;
	}

	if (lockscreen_call_status_active())
		lockscreen_password_view_btn_return_to_call_show(pass_view);
	else
		lockscreen_password_view_btn_return_to_call_hide(pass_view);

	return ECORE_CALLBACK_PASS_ON;
}

int lockscreen_call_ctrl_init(Evas_Object *view)
{
	if (lockscreen_call_init()) {
		ERR("lockscreen_call_init failed");
		return 1;
	}

	main_view = view;
	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_CALL_STATUS_CHANGED, _call_status_changed_cb, NULL);

	if (lockscreen_call_status_active()) {

		Evas_Object *call_view = lockscreen_swipe_icon_view_create(view, ICON_CALL);
		if (!call_view) {
			DBG("call_view == NULL");
			return 1;
		}

		lockscreen_main_view_part_content_set(view, PART_CALL, call_view);
	}

	return 0;
}

void lockscreen_call_ctrl_shutdown(void)
{
	ecore_event_handler_del(handler);

	Evas_Object *call_view = lockscreen_main_view_part_content_unset(main_view, PART_CALL);
	if (call_view)
		evas_object_del(call_view);

	lockscreen_call_shutdown();
}
