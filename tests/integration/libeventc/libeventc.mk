TESTS += \
	tests/integration/libeventc-connection.test

check_PROGRAMS += \
	tests/integration/libeventc-connection.test

tests_integration_libeventc_connection_test_SOURCES = \
	tests/integration/libeventc/connection.vala

tests_integration_libeventc_connection_test_VALAFLAGS = \
	$(AM_TESTS_INTEGRATION_VALAFLAGS) \
	--pkg libeventd-test \
	--pkg libeventd-event \
	--pkg libeventc \
	$(GIO_VALAFLAGS) \
	$(GOBJECT_VALAFLAGS) \
	$(GLIB_VALAFLAGS)

tests_integration_libeventc_connection_test_CFLAGS = \
	$(AM_TESTS_INTEGRATION_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS)

tests_integration_libeventc_connection_test_LDADD = \
	tests/integration/libeventd-test.la \
	libeventd-event.la \
	libeventc.la \
	$(GIO_LIBS) \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS)

$(srcdir)/tests_integration_libeventc_connection_test_vala.stamp: $(srcdir)/tests_integration_libeventd_test_la_vala.stamp

TESTS += \
	tests/integration/libeventc-reconnection.test

check_PROGRAMS += \
	tests/integration/libeventc-reconnection.test

tests_integration_libeventc_reconnection_test_SOURCES = \
	tests/integration/libeventc/reconnection.vala

tests_integration_libeventc_reconnection_test_VALAFLAGS = \
	$(AM_TESTS_INTEGRATION_VALAFLAGS) \
	--pkg libeventd-test \
	--pkg libeventd-event \
	--pkg libeventc \
	$(GIO_VALAFLAGS) \
	$(GOBJECT_VALAFLAGS) \
	$(GLIB_VALAFLAGS)

tests_integration_libeventc_reconnection_test_CFLAGS = \
	$(AM_TESTS_INTEGRATION_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS)

tests_integration_libeventc_reconnection_test_LDADD = \
	tests/integration/libeventd-test.la \
	libeventd-event.la \
	libeventc.la \
	$(GIO_LIBS) \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS)

$(srcdir)/tests_integration_libeventc_reconnection_test_vala.stamp: $(srcdir)/tests_integration_libeventd_test_la_vala.stamp
