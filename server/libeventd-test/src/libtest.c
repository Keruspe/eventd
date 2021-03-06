/*
 * eventd - Small daemon to act on remote or local events
 *
 * Copyright © 2011-2012 Quentin "Sardem FF7" Glidic
 *
 * This file is part of eventd.
 *
 * eventd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eventd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eventd. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include <glib.h>

#include <libeventd-test.h>

#define EVENTS_DIR     SRC_DIR   G_DIR_SEPARATOR_S "plugins" G_DIR_SEPARATOR_S "test-plugin" G_DIR_SEPARATOR_S "events"
#define RUN_DIR        BUILD_DIR G_DIR_SEPARATOR_S ".test-run"
#define PLUGINS_DIR    BUILD_DIR G_DIR_SEPARATOR_S LT_OBJDIR

#define EVENTD_PATH    BUILD_DIR G_DIR_SEPARATOR_S "eventd" EXEEXT
#define EVENTDCTL_PATH BUILD_DIR G_DIR_SEPARATOR_S "eventdctl" EXEEXT

struct _EventdTestsEnv {
    const gchar *dir;
    gchar **env;
    gchar **start_args;
    gchar **stop_args;
};

void
eventd_tests_env_setup(gchar **argv)
{
#ifdef DEBUG
    g_setenv("G_MESSAGES_DEBUG", "all", FALSE);
#endif /* DEBUG */

#if ! GLIB_CHECK_VERSION(2,35,1)
    g_type_init();
#endif /* ! GLIB_CHECK_VERSION(2,35,1) */

    gchar *tmp = g_path_get_basename(argv[0]);
    g_set_prgname(tmp);
    g_free(tmp);

    const gchar *tmp_dir;
    tmp_dir = g_getenv("EVENTD_TESTS_TMP_DIR");

    if ( tmp_dir == NULL )
        g_setenv("EVENTD_TESTS_TMP_DIR", tmp_dir = RUN_DIR, TRUE);
    g_setenv("XDG_RUNTIME_DIR", tmp_dir, TRUE);

    if ( ( ! g_file_test(tmp_dir, G_FILE_TEST_IS_DIR) ) && ( g_mkdir_with_parents(tmp_dir, 0755) < 0 ) )
        g_warning("Couldn't create the test temp dir '%s': %s", tmp_dir, g_strerror(errno));

    g_setenv("EVENTD_CONFIG_DIR", EVENTS_DIR, TRUE);
    g_setenv("EVENTD_PLUGINS_DIR", PLUGINS_DIR, TRUE);
    g_unsetenv("EVENTD_PLUGINS_WHITELIST");
    g_unsetenv("EVENTD_PLUGINS_BLACKLIST");
}

EventdTestsEnv *
eventd_tests_env_new(const gchar *plugins, const gchar *port, gchar **argv, gint argc)
{
    EventdTestsEnv *self;
    gint i;

    self = g_new0(EventdTestsEnv, 1);

    self->dir = g_getenv("EVENTD_TESTS_TMP_DIR");

    guint length;

    self->env = g_get_environ();
    length = g_strv_length(self->env);
    self->env = g_renew(gchar *, self->env, length+2);
    self->env[length] = g_strdup_printf("EVENTD_PLUGINS_WHITELIST=%s", plugins);
    self->env[length+1] = NULL;


    self->start_args = g_new(char *, 10+argc);
    self->start_args[0] = g_strdup(EVENTDCTL_PATH);
    self->start_args[1] = g_strdup("--socket");
    self->start_args[2] = g_strdup(port);
    self->start_args[3] = g_strdup("start");
    self->start_args[4] = g_strdup("--argv0");
    self->start_args[5] = g_strdup(EVENTD_PATH);
    self->start_args[6] = g_strdup("--take-over");
    self->start_args[7] = g_strdup("--private-socket");
    self->start_args[8] = g_strdup(port);
    for ( i = 0 ; i < argc ; ++i )
        self->start_args[9+i] = argv[i];
    self->start_args[9+i] = NULL;
    g_free(argv);


    self->stop_args = g_new(char *, 5);
    self->stop_args[0] = g_strdup(EVENTDCTL_PATH);
    self->stop_args[1] = g_strdup("--socket");
    self->stop_args[2] = g_strdup(port);
    self->stop_args[3] = g_strdup("stop");
    self->stop_args[4] = NULL;


    return self;
}

void
eventd_tests_env_free(EventdTestsEnv *self)
{
    g_strfreev(self->stop_args);
    g_strfreev(self->start_args);
    g_strfreev(self->env);
    g_free(self);
}

gboolean
eventd_tests_env_start_eventd(EventdTestsEnv *self)
{
    GError *error = NULL;
    if ( g_spawn_sync(self->dir, self->start_args, self->env, 0, NULL, NULL, NULL, NULL, NULL, &error) )
        return TRUE;
    g_warning("Failed to start eventd: %s", error->message);
    g_error_free(error);
    return FALSE;
}

gboolean
eventd_tests_env_stop_eventd(EventdTestsEnv *self)
{
    GError *error = NULL;
    if ( g_spawn_sync(self->dir, self->stop_args, self->env, 0, NULL, NULL, NULL, NULL, NULL, &error) )
        return TRUE;
    g_warning("Failed to stop eventd: %s", error->message);
    g_error_free(error);
    return FALSE;
}
