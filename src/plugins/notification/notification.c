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

#include <eventd-plugin.h>
#include <plugin-helper.h>

#include "notification.h"
#include "icon.h"
#include "notify.h"


static GHashTable *events = NULL;

static void
eventd_notification_start(gpointer user_data)
{
    eventd_notification_notify_init();

    eventd_plugin_helper_regex_init();
}

static void
eventd_notification_stop()
{
    eventd_plugin_helper_regex_clean();

    eventd_notification_notify_uninit();
}

static void
eventd_notification_event_clean(EventdNotificationEvent *event)
{
    if ( event->pixbuf != NULL )
        g_object_unref(event->pixbuf);
    g_free(event->icon);
    if ( event->overlay_pixbuf != NULL )
        g_object_unref(event->overlay_pixbuf);
    g_free(event->overlay_icon);
    g_free(event->message);
    g_free(event->title);
}

static void
eventd_notification_event_update(EventdNotificationEvent *event, gboolean disable, const char *title, const char *message, const char *icon, const char *overlay_icon, Int *scale, Int *timeout)
{
    eventd_notification_event_clean(event);
    event->disable = disable;
    if ( title != NULL )
        event->title = g_strdup(title);
    if ( message != NULL )
        event->message = g_strdup(message);
    if ( icon != NULL )
    {
        if ( g_str_has_prefix(icon, "file://") )
            event->pixbuf = eventd_notification_icon_get_pixbuf_from_file(icon+7);
        if ( event->pixbuf == NULL )
            event->icon = g_strdup(icon);
    }
    if ( overlay_icon != NULL )
    {
        if ( g_str_has_prefix(overlay_icon, "file://") )
            event->overlay_pixbuf = eventd_notification_icon_get_pixbuf_from_file(overlay_icon+7);
        if ( event->overlay_pixbuf == NULL )
            event->overlay_icon = g_strdup(overlay_icon);
    }
    if ( scale->set )
        event->scale = (gdouble)scale->value / 100.;
    if ( timeout->set )
        event->timeout = ( timeout->value > 0 ) ? ( timeout->value * 1000 ) : timeout->value;
}

static EventdNotificationEvent *
eventd_notification_event_new(gboolean disable, const char *title, const char *message, const char *icon, const char *overlay_icon, Int *scale, Int *timeout, EventdNotificationEvent *parent)
{
    EventdNotificationEvent *event = NULL;

    title = title ?: parent ? parent->title : "$client-name - $event-data[name]";
    message = message ?: parent ? parent->message : "$event-data[text]";
    icon = icon ?: parent ? parent->icon : "icon";
    overlay_icon = overlay_icon ?: parent ? parent->overlay_icon : "overlay-icon";
    scale->value = scale->set ? scale->value : parent ? parent->scale : 50;
    scale->set = TRUE;
    timeout->value = timeout->set ? timeout->value : parent ? parent->timeout : -1;
    timeout->set = TRUE;

    event = g_new0(EventdNotificationEvent, 1);

    eventd_notification_event_update(event, disable, title, message, icon, overlay_icon, scale, timeout);

    return event;
}

static void
eventd_notification_event_free(EventdNotificationEvent *event)
{
    eventd_notification_event_clean(event);
    g_free(event);
}

static void
eventd_notification_event_parse(const gchar *client_type, const gchar *event_type, GKeyFile *config_file)
{
    gboolean disable;
    gchar *name = NULL;
    gchar *title = NULL;
    gchar *message = NULL;
    gchar *icon = NULL;
    gchar *overlay_icon = NULL;
    Int scale;
    Int timeout;
    EventdNotificationEvent *event;

    if ( ! g_key_file_has_group(config_file, "notification") )
        return;

    if ( eventd_plugin_helper_config_key_file_get_boolean(config_file, "notification", "disable", &disable) < 0 )
        goto skip;
    if ( eventd_plugin_helper_config_key_file_get_string(config_file, "notification", "title", &title) < 0 )
        goto skip;
    if ( eventd_plugin_helper_config_key_file_get_string(config_file, "notification", "message", &message) < 0 )
        goto skip;
    if ( eventd_plugin_helper_config_key_file_get_string(config_file, "notification", "icon", &icon) < 0 )
        goto skip;
    if ( eventd_plugin_helper_config_key_file_get_string(config_file, "notification", "overlay-icon", &overlay_icon) < 0 )
        goto skip;
    if ( eventd_plugin_helper_config_key_file_get_int(config_file, "notification", "overlay-scale", &scale) < 0 )
        goto skip;
    if ( eventd_plugin_helper_config_key_file_get_int(config_file, "notification", "timeout", &timeout) < 0 )
        goto skip;

    if ( event_type != NULL )
        name = g_strconcat(client_type, "-", event_type, NULL);
    else
        name = g_strdup(client_type);

    event = g_hash_table_lookup(events, name);
    if ( event != NULL )
        eventd_notification_event_update(event, disable, title, message, icon, overlay_icon, &scale, &timeout);
    else
        g_hash_table_insert(events, name, eventd_notification_event_new(disable, title, message, icon, overlay_icon, &scale, &timeout, g_hash_table_lookup(events, client_type)));

skip:
    g_free(overlay_icon);
    g_free(icon);
    g_free(message);
    g_free(title);
}

static void
eventd_notification_event_action_get_notification(EventdEvent *event, EventdNotificationEvent *notification_event, EventdNotificationNotification *notification)
{
    gchar *tmp = NULL;

    tmp = eventd_plugin_helper_regex_replace_client_name(notification_event->title, event->client->name);
    notification->title = eventd_plugin_helper_regex_replace_event_data(tmp, event->data, NULL);
    g_free(tmp);

    notification->message = eventd_plugin_helper_regex_replace_event_data(notification_event->message, event->data, NULL);

    notification->icon = eventd_notification_icon_get_pixbuf(event, notification_event);

    notification->timeout = notification_event->timeout;
}

static GHashTable *
eventd_notification_event_action(EventdEvent *event)
{
    gchar *name;
    GError *error = NULL;
    EventdNotificationEvent *notification_event;
    EventdNotificationNotification notification;
    GHashTable *ret = NULL;

    name = g_strconcat(event->client->type, "-", event->type, NULL);
    notification_event = g_hash_table_lookup(events, name);
    g_free(name);
    if ( ( notification_event == NULL ) && ( ( notification_event = g_hash_table_lookup(events, event->client->type) ) == NULL ) )
        return NULL;

    if ( notification_event->disable )
        return NULL;

    eventd_notification_event_action_get_notification(event, notification_event, &notification);

    switch ( event->client->mode )
    {
    case EVENTD_CLIENT_MODE_PING_PONG:
        ret = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
        g_hash_table_insert(ret, g_strdup("title"), notification.title);
        g_hash_table_insert(ret, g_strdup("message"), notification.message);
        if ( notification.icon != NULL )
            g_hash_table_insert(ret, g_strdup("icon"), eventd_notification_icon_get_base64(notification.icon));
        g_hash_table_insert(ret, g_strdup("timeout"), g_strdup_printf("%lld", notification.timeout));
    break;
    default:
        eventd_notification_notify_event_action(&notification);
    }

    return ret;
}


static void
eventd_notification_config_init()
{
    events = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)eventd_notification_event_free);
}

static void
eventd_notification_config_clean()
{
    g_hash_table_unref(events);
}

void
eventd_plugin_get_info(EventdPlugin *plugin)
{
    plugin->id = "notification";

    plugin->start = eventd_notification_start;
    plugin->stop = eventd_notification_stop;

    plugin->config_init = eventd_notification_config_init;
    plugin->config_clean = eventd_notification_config_clean;

    plugin->event_parse = eventd_notification_event_parse;
    plugin->event_action = eventd_notification_event_action;
}
