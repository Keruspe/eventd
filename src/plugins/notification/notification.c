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

#include <eventd-plugin.h>
#include <libeventd-event.h>
#include <libeventd-client.h>
#include <libeventd-config.h>
#include <libeventd-regex.h>

#include "notification.h"
#include "icon.h"
#include "notify.h"


struct _EventdPluginContext {
    GHashTable *events;
};

static EventdPluginContext *
_eventd_notification_start(gpointer user_data)
{
    EventdPluginContext *context;

    context = g_new0(EventdPluginContext, 1);

    eventd_notification_notify_init();

    libeventd_regex_init();

    return context;
}

static void
_eventd_notification_stop(EventdPluginContext *context)
{
    libeventd_regex_clean();

    eventd_notification_notify_uninit();

    g_free(context);
}

static void
_eventd_notification_event_update(EventdNotificationEvent *event, gboolean disable, const char *title, const char *message, const char *icon, const char *overlay_icon, Int *scale)
{
    event->disable = disable;
    if ( title != NULL )
    {
        g_free(event->title);
        event->title = g_strdup(title);
    }
    if ( message != NULL )
    {
        g_free(event->message);
        event->message = g_strdup(message);
    }
    if ( icon != NULL )
    {
        g_free(event->icon);
        event->icon = g_strdup(icon);
    }
    if ( overlay_icon != NULL )
    {
        g_free(event->overlay_icon);
        event->overlay_icon = g_strdup(overlay_icon);
    }
    if ( scale->set )
        event->scale = (gdouble)scale->value / 100.;
}

static EventdNotificationEvent *
_eventd_notification_event_new(gboolean disable, const char *title, const char *message, const char *icon, const char *overlay_icon, Int *scale, EventdNotificationEvent *parent)
{
    EventdNotificationEvent *event = NULL;

    title = title ?: parent ? parent->title : "$client-name - $event-data[name]";
    message = message ?: parent ? parent->message : "$event-data[text]";
    icon = icon ?: parent ? parent->icon : "icon";
    overlay_icon = overlay_icon ?: parent ? parent->overlay_icon : "overlay-icon";
    scale->value = scale->set ? scale->value : parent ? parent->scale * 100 : 50;
    scale->set = TRUE;

    event = g_new0(EventdNotificationEvent, 1);

    _eventd_notification_event_update(event, disable, title, message, icon, overlay_icon, scale);

    return event;
}

static void
_eventd_notification_event_free(gpointer data)
{
    EventdNotificationEvent *event = data;

    g_free(event->icon);
    g_free(event->overlay_icon);
    g_free(event->message);
    g_free(event->title);
    g_free(event);
}

static void
_eventd_notification_event_parse(EventdPluginContext *context, const gchar *client_type, const gchar *event_name, GKeyFile *config_file)
{
    gboolean disable;
    gchar *name = NULL;
    gchar *title = NULL;
    gchar *message = NULL;
    gchar *icon = NULL;
    gchar *overlay_icon = NULL;
    Int scale;
    EventdNotificationEvent *event;

    if ( ! g_key_file_has_group(config_file, "notification") )
        return;

    if ( libeventd_config_key_file_get_boolean(config_file, "notification", "disable", &disable) < 0 )
        goto skip;
    if ( libeventd_config_key_file_get_string(config_file, "notification", "title", &title) < 0 )
        goto skip;
    if ( libeventd_config_key_file_get_string(config_file, "notification", "message", &message) < 0 )
        goto skip;
    if ( libeventd_config_key_file_get_string(config_file, "notification", "icon", &icon) < 0 )
        goto skip;
    if ( libeventd_config_key_file_get_string(config_file, "notification", "overlay-icon", &overlay_icon) < 0 )
        goto skip;
    if ( libeventd_config_key_file_get_int(config_file, "notification", "overlay-scale", &scale) < 0 )
        goto skip;

    name = libeventd_config_events_get_name(client_type, event_name);

    event = g_hash_table_lookup(context->events, name);
    if ( event != NULL )
        _eventd_notification_event_update(event, disable, title, message, icon, overlay_icon, &scale);
    else
        g_hash_table_insert(context->events, name, _eventd_notification_event_new(disable, title, message, icon, overlay_icon, &scale, g_hash_table_lookup(context->events, client_type)));

skip:
    g_free(overlay_icon);
    g_free(icon);
    g_free(message);
    g_free(title);
}

static EventdNotificationNotification *
_eventd_notification_notification_new(EventdClient *client, GHashTable *data, EventdNotificationEvent *notification_event)
{
    EventdNotificationNotification *notification;
    gchar *tmp = NULL;

    notification = g_new0(EventdNotificationNotification, 1);

    tmp = libeventd_regex_replace_client_name(notification_event->title, libeventd_client_get_name(client));
    notification->title = libeventd_regex_replace_event_data(tmp, data, NULL);
    g_free(tmp);

    notification->message = libeventd_regex_replace_event_data(notification_event->message, data, NULL);

    eventd_notification_icon_get_pixbuf(data, notification_event, notification);

    return notification;
}


static void
_eventd_notification_notification_add_pong_data(EventdEvent *event, EventdNotificationNotification *notification)
{
    eventd_event_add_pong_data(event, g_strdup("notification-title"), g_strdup(notification->title));
    eventd_event_add_pong_data(event, g_strdup("notification-message"), g_strdup(notification->message));

    if ( notification->icon != NULL )
        eventd_event_add_pong_data(event, g_strdup("notification-icon"), eventd_notification_icon_get_base64(notification->icon));
    if ( notification->overlay_icon != NULL )
        eventd_event_add_pong_data(event, g_strdup("notification-overlay-icon"), eventd_notification_icon_get_base64(notification->overlay_icon));
    if ( notification->merged_icon != NULL )
        eventd_event_add_pong_data(event, g_strdup("notification-merged-icon"), eventd_notification_icon_get_base64(notification->merged_icon));
}

static void
_eventd_notification_notification_free(EventdNotificationNotification *notification)
{
    if ( notification->merged_icon != NULL )
        eventd_notification_icon_unref(notification->merged_icon);
    if ( notification->overlay_icon != NULL )
        eventd_notification_icon_unref(notification->overlay_icon);
    if ( notification->icon != NULL )
        eventd_notification_icon_unref(notification->icon);

    g_free(notification->message);
    g_free(notification->title);

    g_free(notification);
}

static void
_eventd_notification_event_action(EventdPluginContext *context, EventdClient *client, EventdEvent *event)
{
    EventdNotificationEvent *notification_event;
    EventdNotificationNotification *notification;

    notification_event = libeventd_config_events_get_event(context->events, libeventd_client_get_type(client), eventd_event_get_name(event));
    if ( notification_event == NULL )
        return;

    if ( notification_event->disable )
        return;

    notification = _eventd_notification_notification_new(client, eventd_event_get_data(event), notification_event);

    switch ( libeventd_client_get_mode(client) )
    {
    case EVENTD_CLIENT_MODE_PING_PONG:
        _eventd_notification_notification_add_pong_data(event, notification);
    break;
    default:
        eventd_notification_notify_event_action(event, notification);
    }

    _eventd_notification_notification_free(notification);
}


static void
_eventd_notification_config_init(EventdPluginContext *context)
{
    context->events = libeventd_config_events_new(_eventd_notification_event_free);
}

static void
_eventd_notification_config_clean(EventdPluginContext *context)
{
    g_hash_table_unref(context->events);
}

void
eventd_plugin_get_info(EventdPlugin *plugin)
{
    plugin->start = _eventd_notification_start;
    plugin->stop = _eventd_notification_stop;

    plugin->config_init = _eventd_notification_config_init;
    plugin->config_clean = _eventd_notification_config_clean;

    plugin->event_parse = _eventd_notification_event_parse;
    plugin->event_action = _eventd_notification_event_action;
}

