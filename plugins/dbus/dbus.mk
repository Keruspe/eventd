# dbus plugin

plugins_LTLIBRARIES += \
	dbus.la

dist_pkgdata_DATA += \
	plugins/dbus/events/libnotify.event


dbus_la_SOURCES = \
	plugins/dbus/src/dbus.c

dbus_la_CFLAGS = \
	$(AM_CFLAGS) \
	-D G_LOG_DOMAIN=\"eventd-dbus\" \
	$(GIO_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS)

dbus_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version -module

dbus_la_LIBADD = \
	libeventd-event.la \
	$(GIO_LIBS) \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS)