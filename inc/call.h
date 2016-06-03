
#define CALL_APP_CONTROL_CALL_APP_ID "org.tizen.contacts"

#define CALL_APP_CONTROL_CALL_APP_OPERATION "http://tizen.org/appcontrol/operation/return_to_call"

extern int LOCKSCREEN_EVENT_CALL_STATUS_CHANGED;

int lockscreen_call_app_launch_request(void);
int lockscreen_call_status_active(void);
int lockscreen_call_init(void);
void lockscreen_call_shutdown(void);
