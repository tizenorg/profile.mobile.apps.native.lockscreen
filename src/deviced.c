#include <gio/gio.h>
#include "log.h"


int lockscreen_deviced_lockscreen_background_state_set(bool val)
{
	GError *error = NULL;
	GVariant *value;
	int ret;

	GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if (!conn) {
		ERR("g_bus_get_sync failed");
		return 1;
	}

	value = g_dbus_connection_call_sync(
			conn,
			"org.tizen.system.deviced",
			"/Org/Tizen/System/DeviceD/Display",
			"org.tizen.system.deviced.display",
			"LockScreenBgOn",
			g_variant_new("(s)", val ? "true" : "false"),
			G_VARIANT_TYPE("(i)"),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);

	if (!value) {
		if (error) {
			ERR("No reply from deviced: %s", error->message);
			g_error_free(error);
		}
		ret = 1;
	} else {
		DBG("LockScreenBgOn  => (%s) answer: %d",
				val ? "true" : "false",
				g_variant_get_int32(value));
		g_variant_unref(value);
		ret = 0;
	}

	g_object_unref(conn);
	return ret;
}
