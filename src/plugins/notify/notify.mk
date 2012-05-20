# notify plugin
plugins_LTLIBRARIES += \
	notify.la

notify_la_SOURCES = \
	src/plugins/notify/icon.h \
	src/plugins/notify/icon.c \
	src/plugins/notify/libnotify-compat.h \
	src/plugins/notify/notify.c

notify_la_CFLAGS = \
	$(AM_CFLAGS) \
	-D G_LOG_DOMAIN=\"eventd-notify\" \
	$(NOTIFY_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS)

notify_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version -module -export-symbols-regex 'eventd_plugin_(id|get_info)'

notify_la_LIBADD = \
	libeventd-event.la \
	libeventd.la \
	$(NOTIFY_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS)
