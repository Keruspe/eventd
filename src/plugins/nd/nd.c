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

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <eventd-plugin.h>
#include <libeventd-event.h>
#include <libeventd-config.h>

#include <eventd-nd-types.h>
#include <eventd-nd-backend.h>
#include <eventd-nd-notification.h>
#include <eventd-nd-style.h>

struct _EventdPluginContext {
    GHashTable *events;
    EventdNdStyleAnchor bubble_anchor;
    gint bubble_margin;
    EventdNdStyle *style;
    GList *backends;
    GList *displays;
    GHashTable *surfaces;
};

typedef struct {
    EventdNdBackend *backend;
    EventdNdDisplay *display;
} EventdNdDisplayContext;

typedef struct {
    EventdNdBackend *backend;
    EventdNdSurface *surface;
} EventdNdSurfaceContext;

typedef struct {
    gchar *title;
    gchar *message;
    gchar *image;
    gchar *icon;
    EventdNdStyle *style;
} EventdNdEvent;

static void
_eventd_nd_backends_load_dir(EventdPluginContext *context, const gchar *backends_dir_name)
{
    GError *error;
    GDir *plugins_dir;
    const gchar *file;


#if DEBUG
    g_debug("Scannig notification backends dir: %s", backends_dir_name);
#endif /* DEBUG */

    plugins_dir = g_dir_open(backends_dir_name, 0, &error);
    if ( ! plugins_dir )
    {
        g_warning("Can’t read the plugins directory: %s", error->message);
        g_clear_error(&error);
        return;
    }

    while ( ( file = g_dir_read_name(plugins_dir) ) != NULL )
    {
        gchar *full_filename;
        EventdNdBackend *backend;
        EventdNdBackendGetInfoFunc backend_get_info;
        GModule *module;

        full_filename = g_build_filename(backends_dir_name, file, NULL);

        if ( g_file_test(full_filename, G_FILE_TEST_IS_DIR) )
        {
            g_free(full_filename);
            continue;
        }

        module = g_module_open(full_filename, G_MODULE_BIND_LAZY|G_MODULE_BIND_LOCAL);
        if ( module == NULL )
        {
            g_warning("Couldn’t load module '%s': %s", file, g_module_error());
            g_free(full_filename);
            continue;
        }
        g_free(full_filename);

        if ( ! g_module_symbol(module, "eventd_nd_backend_init", (void **)&backend_get_info) )
            continue;

#if DEBUG
        g_debug("Loading plugin '%s'", file);
#endif /* ! DEBUG */

        backend = g_new0(EventdNdBackend, 1);
        backend->module = module;
        backend_get_info(backend);

        backend->context = backend->init();

        context->backends = g_list_prepend(context->backends, backend);
    }
}

static void
_eventd_nd_backend_load(EventdPluginContext *context)
{
    const gchar *env_base_dir;
    gchar *plugins_dir;

    if ( ! g_module_supported() )
    {
        g_warning("Couldn’t load plugins: %s", g_module_error());
        return;
    }

    env_base_dir = g_getenv("EVENTD_NOTIFICATION_BACKENDS_DIR");
    if ( env_base_dir != NULL )
    {
        if ( env_base_dir[0] == '~' )
            plugins_dir = g_build_filename(g_get_home_dir(), env_base_dir+2, NULL);
        else
            plugins_dir = g_build_filename(env_base_dir,  NULL);

        if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
            _eventd_nd_backends_load_dir(context, plugins_dir);
        g_free(plugins_dir);
    }

    plugins_dir = g_build_filename(g_get_user_data_dir(), PACKAGE_NAME, "plugins", "nd", NULL);
    if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
        _eventd_nd_backends_load_dir(context, plugins_dir);
    g_free(plugins_dir);

    plugins_dir = g_build_filename(DATADIR, PACKAGE_NAME, "plugins", "nd", NULL);
    if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
        _eventd_nd_backends_load_dir(context, plugins_dir);
    g_free(plugins_dir);

    plugins_dir = g_build_filename(LIBDIR, PACKAGE_NAME, "plugins", "nd", NULL);
    if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
        _eventd_nd_backends_load_dir(context, plugins_dir);
    g_free(plugins_dir);
}

static void
_eventd_nd_event_update(EventdNdEvent *event, GKeyFile *config_file)
{
    gchar *string;

    if ( libeventd_config_key_file_get_string(config_file, "notification", "title", &string) == 0 )
    {
        g_free(event->title);
        event->title = string;
    }
    if ( libeventd_config_key_file_get_string(config_file, "notification", "message", &string) == 0 )
    {
        g_free(event->message);
        event->message = string;
    }
    if ( libeventd_config_key_file_get_string(config_file, "notification", "image", &string) == 0 )
    {
        g_free(event->image);
        event->image = string;
    }
    if ( libeventd_config_key_file_get_string(config_file, "notification", "icon", &string) == 0 )
    {
        g_free(event->icon);
        event->icon = string;
    }

    eventd_nd_style_update(event->style, config_file);
}

static EventdNdEvent *
_eventd_nd_event_new(EventdPluginContext *context, EventdNdEvent *parent)
{
    EventdNdEvent *event = NULL;

    event = g_new0(EventdNdEvent, 1);

    if ( parent == NULL )
    {
        event->title = g_strdup("$name");
        event->message = g_strdup("$text");
        event->image = g_strdup("image");
        event->icon = g_strdup("icon");
        event->style = eventd_nd_style_new(context->style);
    }
    else
    {
        event->title = g_strdup(parent->title);
        event->message = g_strdup(parent->message);
        event->image = g_strdup(parent->image);
        event->icon = g_strdup(parent->icon);
        event->style = eventd_nd_style_new(parent->style);
    }

    return event;
}

static void
_eventd_nd_event_free(gpointer data)
{
    EventdNdEvent *event = data;

    g_free(event->image);
    g_free(event->icon);
    g_free(event->message);
    g_free(event->title);
    g_free(event);
}

static void
_eventd_nd_event_parse(EventdPluginContext *context, const gchar *event_category, const gchar *event_name, GKeyFile *config_file)
{
    gboolean disable;
    EventdNdEvent *event = NULL;

    if ( ! g_key_file_has_group(config_file, "notification") )
        return;

    if ( libeventd_config_key_file_get_boolean(config_file, "notification", "disable", &disable) < 0 )
        return;

    if ( ! disable )
    {
        event = _eventd_nd_event_new(context, ( event_name != NULL ) ? g_hash_table_lookup(context->events, event_category) : NULL);
        _eventd_nd_event_update(event, config_file);
    }

    g_hash_table_insert(context->events, libeventd_config_events_get_name(event_category, event_name), event);
}

static void
_eventd_nd_surface_hide(gpointer data)
{
    EventdNdSurfaceContext *surface = data;

    surface->backend->surface_hide(surface->surface);

    g_free(surface);
}

static void
_eventd_nd_surface_hide_all(gpointer data)
{
    GList *surfaces = data;

    g_list_free_full(surfaces, _eventd_nd_surface_hide);
}

static EventdPluginContext *
_eventd_nd_init(EventdCoreContext *core, EventdCoreInterface *interface)
{
    EventdPluginContext *context;

    context = g_new0(EventdPluginContext, 1);

    _eventd_nd_backend_load(context);

    context->events = libeventd_config_events_new(_eventd_nd_event_free);
    context->surfaces = g_hash_table_new_full(g_direct_hash, g_direct_equal, g_object_unref, _eventd_nd_surface_hide_all);

    eventd_nd_notification_init();

    /* default bubble position */
    context->bubble_anchor    = EVENTD_ND_STYLE_ANCHOR_TOP_RIGHT;
    context->bubble_margin    = 13;

    context->style = eventd_nd_style_new(NULL);

    return context;
}

static void
_eventd_nd_backend_free(gpointer data)
{
    EventdNdBackend *backend = data;

    backend->uninit(backend->context);

    g_module_close(backend->module);

    g_free(backend);
}

static void
_eventd_nd_backend_display_free(gpointer data)
{
    EventdNdDisplayContext *display = data;

    display->backend->display_free(display->display);

    g_free(display);
}

static void
_eventd_nd_uninit(EventdPluginContext *context)
{
    eventd_nd_style_free(context->style);

    eventd_nd_notification_uninit();

    g_hash_table_unref(context->surfaces);
    g_hash_table_unref(context->events);

    g_list_free_full(context->displays, _eventd_nd_backend_display_free);

    g_list_free_full(context->backends, _eventd_nd_backend_free);

    g_free(context);
}

static void
_eventd_nd_control_command(EventdPluginContext *context, const gchar *command)
{
    const gchar *target = command+20;
    EventdNdDisplay *display;
    GList *backend_;

    if ( ! g_str_has_prefix(command, "notification-daemon ") )
        return;

    for ( backend_ = context->backends ; backend_ != NULL ; backend_ = g_list_next(backend_) )
    {
        EventdNdBackend *backend = backend_->data;

        if ( ! backend->display_test(backend->context, target) )
            continue;

        display = backend->display_new(backend->context, target, context->bubble_anchor, context->bubble_margin);
        if ( display == NULL )
            g_warning("Couldn’t initialize display for '%s'", target);
        else
        {
            EventdNdDisplayContext *display_context;

            display_context = g_new(EventdNdDisplayContext, 1);
            display_context->backend = backend;
            display_context->display = display;

            context->displays = g_list_prepend(context->displays, display_context);
        }
    }
}

static void
_eventd_nd_global_parse(EventdPluginContext *context, GKeyFile *config_file)
{
    if ( g_key_file_has_group(config_file, "nd") )
    {
        Int integer;
        gchar *string;

        if ( libeventd_config_key_file_get_string(config_file, "nd", "anchor", &string) == 0 )
        {
            if ( g_strcmp0(string, "top left") == 0 )
                context->bubble_anchor = EVENTD_ND_STYLE_ANCHOR_TOP_LEFT;
            else if ( g_strcmp0(string, "top right") == 0 )
                context->bubble_anchor = EVENTD_ND_STYLE_ANCHOR_TOP_RIGHT;
            else if ( g_strcmp0(string, "bottom left") == 0 )
                context->bubble_anchor = EVENTD_ND_STYLE_ANCHOR_BOTTOM_LEFT;
            else if ( g_strcmp0(string, "bottom right") == 0 )
                context->bubble_anchor = EVENTD_ND_STYLE_ANCHOR_BOTTOM_RIGHT;
            else
                g_warning("Wrong anchor value '%s'", string);
            g_free(string);
        }

        if ( libeventd_config_key_file_get_int(config_file, "nd", "margin", &integer) == 0 )
            context->bubble_margin = integer.value;
    }

    eventd_nd_style_update(context->style, config_file);
}

static void
_eventd_nd_event_ended(EventdEvent *event, EventdEventEndReason reason, EventdPluginContext *context)
{
    g_hash_table_remove(context->surfaces, event);
}

static void
_eventd_nd_event_action(EventdPluginContext *context, EventdEvent *event)
{
    EventdNdEvent *nd_event;
    EventdNdNotification *notification;
    GList *display_;
    GList *surfaces = NULL;

    nd_event = libeventd_config_events_get_event(context->events, eventd_event_get_category(event), eventd_event_get_name(event));
    if ( nd_event == NULL )
        return;

    notification = eventd_nd_notification_new(event, nd_event->title, nd_event->message, nd_event->image, nd_event->icon);

    for ( display_ = context->displays ; display_ != NULL ; display_ = g_list_next(display_) )
    {
        EventdNdDisplayContext *display = display_->data;
        EventdNdSurface *surface;
        surface = display->backend->surface_show(event, display->display, notification, nd_event->style);
        if ( surface != NULL )
        {
            EventdNdSurfaceContext *surface_context;

            surface_context = g_new(EventdNdSurfaceContext, 1);
            surface_context->backend = display->backend;
            surface_context->surface = surface;

            surfaces = g_list_prepend(surfaces, surface_context);
        }
    }

    if ( surfaces != NULL )
    {
        g_signal_connect(event, "ended", G_CALLBACK(_eventd_nd_event_ended), context);

        /*
         * TODO: Update an existing bubble
         */

        g_hash_table_insert(context->surfaces, g_object_ref(event), surfaces);
    }

    eventd_nd_notification_free(notification);
}

static void
_eventd_nd_config_reset(EventdPluginContext *context)
{
    g_hash_table_remove_all(context->events);
}

void
eventd_plugin_get_info(EventdPlugin *plugin)
{
    plugin->init = _eventd_nd_init;
    plugin->uninit = _eventd_nd_uninit;

    plugin->control_command = _eventd_nd_control_command;

    plugin->config_reset = _eventd_nd_config_reset;

    plugin->global_parse = _eventd_nd_global_parse;
    plugin->event_parse = _eventd_nd_event_parse;
    plugin->event_action = _eventd_nd_event_action;
}
