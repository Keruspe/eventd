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

#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include <eventd-plugin.h>
#include <libeventd-event.h>

static void
_eventd_test_event_action(EventdPluginContext *context, EventdEvent *event)
{
    GError *error = NULL;

    const gchar *filename;
    const gchar *contents;

    filename = eventd_event_get_data(event, "file");
    contents = eventd_event_get_data(event, "test");

    if ( ( filename == NULL ) || ( contents == NULL ) )
        return;

    if ( ! g_file_set_contents(filename, contents, -1, &error) )
        g_warning("Couldn’t write to file: %s", error->message);
    g_clear_error(&error);
}

const gchar *eventd_plugin_id = "eventd-test-plugin";
void
eventd_plugin_get_info(EventdPlugin *plugin)
{
    plugin->event_action = _eventd_test_event_action;
}