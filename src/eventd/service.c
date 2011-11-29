/*
 * eventd - Small daemon to act on remote or local events
 *
 * Copyright © 2011 Quentin "Sardem FF7" Glidic
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

#include <glib.h>
#include <glib-object.h>
#ifdef G_OS_UNIX
#include <glib-unix.h>
#endif /* G_OS_UNIX */
#include <gio/gio.h>
#if ENABLE_GIO_UNIX
#include <gio/gunixsocketaddress.h>
#include <glib/gstdio.h>
#endif /* ENABLE_GIO_UNIX */

#include <libeventd-client.h>
#include <libeventd-event.h>

#include "types.h"

#include "config.h"
#include "plugins.h"
#include "control.h"
#include "queue.h"

#include "service.h"

struct _EventdService {
    EventdControl *control;
    EventdConfig *config;
    EventdQueue *queue;
    GMainLoop *loop;
    GSocketService *service;
    guint32 count;
};

void
eventd_service_config_reload(gpointer user_data)
{
    EventdService *service = user_data;
    service->config = eventd_config_parser(service->config);
}

gboolean
eventd_service_quit(gpointer user_data)
{
    EventdService *service = user_data;
    if ( service->loop != NULL )
        g_main_loop_quit(service->loop);

    return FALSE;
}

static void
_eventd_service_send_data(gpointer key, gpointer value, gpointer user_data)
{
    const gchar *name = key;
    const gchar *content = value;
    GDataOutputStream *output = user_data;

    #if DEBUG
    g_debug("Send back data: %s", name);
    #endif /* DEBUG */

    if ( g_utf8_strchr(content, -1, '\n') == NULL )
    {
        gchar *msg;
        msg = g_strconcat("DATAL ", name, " ", content, "\n", NULL);
        g_data_output_stream_put_string(output, msg, NULL, NULL);
        g_free(msg);
    }
    else
    {
        gchar *msg;
        gchar **line;
        gchar **lines;

        msg = g_strconcat("DATA ", name, "\n", NULL);
        g_data_output_stream_put_string(output, msg, NULL, NULL);

        lines = g_strsplit(content, "\n", 0);

        for ( line = lines ; line != NULL ; ++line )
        {
            if ( (*line)[0] == '.' )
                g_data_output_stream_put_byte(output, '.', NULL, NULL);
            g_data_output_stream_put_string(output, *line, NULL, NULL);
            g_data_output_stream_put_byte(output, '\n', NULL, NULL);
        }

        g_strfreev(lines);

        g_data_output_stream_put_string(output, ".\n", NULL, NULL);
    }
}

static gboolean
_eventd_service_connection_handler(GThreadedSocketService *socket_service, GSocketConnection *connection, GObject *source_object, gpointer user_data)
{
    EventdService *service = user_data;

    GDataInputStream *input = NULL;
    GDataOutputStream *output = NULL;
    GError *error = NULL;

    EventdClient *client = NULL;
    EventdEvent *event = NULL;

    gchar *event_data_name = NULL;
    gchar *event_data_content = NULL;

    gsize size = 0;
    gchar *line = NULL;

    gint32 last_eventd_id = 0;

    input = g_data_input_stream_new(g_io_stream_get_input_stream((GIOStream *)connection));
    output = g_data_output_stream_new(g_io_stream_get_output_stream((GIOStream *)connection));

    while ( ( line = g_data_input_stream_read_upto(input, "\n", -1, &size, NULL, &error) ) != NULL )
    {
        g_data_input_stream_read_byte(input, NULL, &error);
        if ( error )
            break;
        #if DEBUG
        g_debug("Line received: %s", line);
        #endif /* DEBUG */

        if ( event != NULL )
        {
            if ( g_ascii_strcasecmp(line, ".") == 0 )
            {
                if ( event_data_name )
                {
                    eventd_event_add_data(event, event_data_name, event_data_content);
                    event_data_name = NULL;
                    event_data_content = NULL;
                }
                else
                {
                    gboolean disable;
                    gint64 timeout;

                    eventd_config_event_get_disable_and_timeout(service->config, client, event, &disable, &timeout);
                    eventd_event_set_timeout(event, timeout);

                    if ( ! disable )
                    {
                        GHashTable *pong = NULL;

                        switch ( libeventd_client_get_mode(client) )
                        {
                        case EVENTD_CLIENT_MODE_PING_PONG:
                            eventd_plugins_event_action_all(client, event);
                            if ( ! g_data_output_stream_put_string(output, "EVENT\n", NULL, &error) )
                                break;
                            pong = eventd_event_get_pong_data(event);
                            if ( pong != NULL )
                                g_hash_table_foreach(pong, _eventd_service_send_data, output);
                            if ( ! g_data_output_stream_put_string(output, ".\n", NULL, &error) )
                                break;
                            eventd_event_end(event, EVENTD_EVENT_END_REASON_RESERVED);
                        break;
                        case EVENTD_CLIENT_MODE_NORMAL:
                        default:
                            eventd_queue_push(service->queue, client, event);
                        break;
                        }
                        if ( error != NULL )
                            break;
                    }

                    g_object_unref(event);
                    event = NULL;
                }
            }
            else if ( event_data_content )
            {
                gchar *old = NULL;

                old = event_data_content;
                event_data_content = g_strjoin("\n", old, ( line[0] == '.' ) ? line+1 : line, NULL);

                g_free(old);
            }
            else if ( g_ascii_strncasecmp(line, "DATA ", 5) == 0 )
            {
                event_data_name = g_strdup(line+5);
            }
            else if ( g_ascii_strncasecmp(line, "DATAL ", 6) == 0 )
            {
                gchar **data = NULL;

                data = g_strsplit(line+6, " ", 2);
                eventd_event_add_data(event, data[0], data[1]);
                g_free(data);
            }
            else if ( event_data_name )
                event_data_content = g_strdup(( line[0] == '.' ) ? line+1 : line);
            else
                g_warning("Unknown message");
        }
        else if ( g_ascii_strncasecmp(line, "EVENT ", 6) == 0 )
        {
            if ( client == NULL )
                break;

            last_eventd_id = ++service->count;

            event = eventd_event_new_with_id(last_eventd_id, line+6);
        }
        else if ( g_ascii_strcasecmp(line, "BYE") == 0 )
        {
            if ( client == NULL )
                break;

            g_data_output_stream_put_string(output, "BYE\n", NULL, &error);
            break;
        }
        else if ( g_ascii_strncasecmp(line, "MODE ", 5) == 0 )
        {
            EventdClientMode mode;

            if ( client == NULL )
                break;

            if ( last_eventd_id > 0 )
            {
                g_warning("Can’t change the mode after the first event");
                g_free(line);
                continue;
            }

            if ( ! g_data_output_stream_put_string(output, "MODE\n", NULL, &error) )
                break;

            if ( g_ascii_strcasecmp(line+5, "normal") == 0 )
                mode = EVENTD_CLIENT_MODE_NORMAL;
            else if ( g_ascii_strcasecmp(line+5, "ping-pong") == 0 )
                mode = EVENTD_CLIENT_MODE_PING_PONG;
            else
            {
                mode = EVENTD_CLIENT_MODE_UNKNOWN;
                g_warning("Unknown mode");
            }

            libeventd_client_set_mode(client, mode);
        }
        else if ( g_ascii_strncasecmp(line, "HELLO ", 6) == 0 )
        {
            gchar **hello = NULL;

            if ( ! g_data_output_stream_put_string(output, "HELLO\n", NULL, &error) )
                break;

            hello = g_strsplit(line+6, " ", 2);
            client = libeventd_client_new();
            libeventd_client_update(client, hello[0], hello[1]);
            g_strfreev(hello);
        }
        else if ( g_ascii_strncasecmp(line, "RENAME ", 7) == 0 )
        {
            gchar **rename = NULL;

            if ( client == NULL )
                break;

            if ( ! g_data_output_stream_put_string(output, "RENAMED\n", NULL, &error) )
                break;

            rename = g_strsplit(line+7, " ", 2);
            libeventd_client_update(client, rename[0], rename[1]);
            g_strfreev(rename);
        }
        else
            g_warning("Unknown message");

        g_free(line);
    }
    if ( error )
        g_warning("Can't read the socket: %s", error->message);
    g_clear_error(&error);

    libeventd_client_unref(client);

    if ( ! g_input_stream_close((GInputStream *)input, NULL, &error) )
        g_warning("Can't close the input stream: %s", error->message);
    g_clear_error(&error);

    if ( ! g_output_stream_close((GOutputStream *)output, NULL, &error) )
        g_warning("Can't close the output stream: %s", error->message);
    g_clear_error(&error);

    if ( ! g_io_stream_close((GIOStream *)connection, NULL, &error) )
        g_warning("Can't close the stream: %s", error->message);
    g_clear_error(&error);

    return TRUE;
}

int
eventd_service(GList *sockets, gboolean no_plugins)
{
    int retval = 0;
    GError *error = NULL;
    GList *socket = NULL;
    EventdService *service;

    service = g_new0(EventdService, 1);

#ifdef G_OS_UNIX
    g_unix_signal_add(SIGTERM, eventd_service_quit, service);
    g_unix_signal_add(SIGINT, eventd_service_quit, service);
#endif /* G_OS_UNIX */

    service->control = eventd_control_start(service, &sockets);

    if ( ! no_plugins )
        eventd_plugins_load();

    service->config = eventd_config_parser(service->config);

    service->queue = eventd_queue_new(service);

    service->service = g_threaded_socket_service_new(eventd_config_get_max_clients(service->config));

    for ( socket = g_list_first(sockets) ; socket ; socket = g_list_next(socket) )
    {
        if ( ! g_socket_listener_add_socket((GSocketListener *)service->service, socket->data, NULL, &error) )
            g_warning("Unable to add socket: %s", error->message);
        g_clear_error(&error);
    }

    g_signal_connect(service->service, "run", G_CALLBACK(_eventd_service_connection_handler), service);

    service->loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(service->loop);
    g_main_loop_unref(service->loop);

    service->loop = NULL;

    g_socket_service_stop(service->service);
    g_socket_listener_close((GSocketListener *)service->service);

    eventd_queue_free(service->queue);

    eventd_config_clean(service->config);

    eventd_control_stop(service->control);

    eventd_plugins_unload();

    return retval;
}
