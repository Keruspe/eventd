/*
 * libeventd-event - Library to manipulate eventd events
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

#include <libeventd-event.h>

#define EVENTD_EVENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), EVENTD_TYPE_EVENT, EventdEventPrivate))

G_DEFINE_TYPE(EventdEvent, eventd_event, G_TYPE_OBJECT)

GType
eventd_event_end_reason_get_type()
{
    static volatile gsize g_define_type_id__volatile = 0;

    if ( g_once_init_enter(&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { EVENTD_EVENT_END_REASON_NONE,           "EVENTD_EVENT_END_REASON_NONE",           "none" },
            { EVENTD_EVENT_END_REASON_TIMEOUT,        "EVENTD_EVENT_END_REASON_TIMEOUT",        "timeout" },
            { EVENTD_EVENT_END_REASON_USER_DISMISS,   "EVENTD_EVENT_END_REASON_USER_DISMISS",   "user-dismiss" },
            { EVENTD_EVENT_END_REASON_CLIENT_DISMISS, "EVENTD_EVENT_END_REASON_CLIENT_DISMISS", "client-dismiss" },
            { EVENTD_EVENT_END_REASON_RESERVED,       "EVENTD_EVENT_END_REASON_RESERVED",       "reserved" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static(g_intern_static_string("EventdEventEndReason"), values);
        g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

struct _EventdEventPrivate {
    gchar *category;
    gchar *name;
    gint64 timeout;
    GHashTable *data;
};


enum {
    SIGNAL_ENDED,
    LAST_SIGNAL
};

static guint _eventd_event_signals[LAST_SIGNAL] = { 0 };


static void _eventd_event_finalize(GObject *object);

static void
eventd_event_class_init(EventdEventClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(EventdEventPrivate));

    eventd_event_parent_class = g_type_class_peek_parent(klass);

    object_class->finalize = _eventd_event_finalize;

    _eventd_event_signals[SIGNAL_ENDED] =
        g_signal_new("ended",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(EventdEventClass, ended),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__ENUM,
                     G_TYPE_NONE,
                     1, EVENTD_TYPE_EVENT_END_REASON);
}

EventdEvent *
eventd_event_new(const gchar *name)
{
    EventdEvent *event;

    g_return_val_if_fail(name != NULL, NULL);

    event = g_object_new(EVENTD_TYPE_EVENT, NULL);

    event->priv->name = g_strdup(name);

    return event;
}

static void
eventd_event_init(EventdEvent *self)
{
    self->priv = EVENTD_EVENT_GET_PRIVATE(self);

    self->priv->timeout = -1;
}

static void
_eventd_event_finalize(GObject *object)
{
    EventdEvent *self = EVENTD_EVENT(object);
    EventdEventPrivate *priv = self->priv;

    if ( priv->data != NULL )
        g_hash_table_unref(priv->data);
    g_free(priv->name);

    G_OBJECT_CLASS(eventd_event_parent_class)->finalize(object);
}

void
eventd_event_end(EventdEvent *self, EventdEventEndReason reason)
{
    g_return_if_fail(EVENTD_IS_EVENT(self));
    g_return_if_fail(reason > EVENTD_EVENT_END_REASON_NONE);

    g_signal_emit(self, _eventd_event_signals[SIGNAL_ENDED], 0, reason);
}

void
eventd_event_add_data(EventdEvent *self, gchar *name, gchar *content)
{
    g_return_if_fail(EVENTD_IS_EVENT(self));
    g_return_if_fail(name != NULL);
    g_return_if_fail(content != NULL);

    if ( self->priv->data == NULL )
        self->priv->data = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    g_hash_table_insert(self->priv->data, name, content);
}

void
eventd_event_set_timeout(EventdEvent *self, gint64 timeout)
{
    g_return_if_fail(EVENTD_IS_EVENT(self));
    g_return_if_fail(timeout >= -1);

    self->priv->timeout = timeout;
}

void
eventd_event_set_category(EventdEvent *self, const gchar *category)
{
    g_return_if_fail(EVENTD_IS_EVENT(self));
    g_return_if_fail(category != NULL);

    g_free(self->priv->category);
    self->priv->category = g_strdup(category);
}


const gchar *
eventd_event_get_category(EventdEvent *self)
{
    g_return_val_if_fail(EVENTD_IS_EVENT(self), NULL);

    return self->priv->category;
}

const gchar *
eventd_event_get_name(EventdEvent *self)
{
    g_return_val_if_fail(EVENTD_IS_EVENT(self), NULL);

    return self->priv->name;
}

gint64
eventd_event_get_timeout(EventdEvent *self)
{
    g_return_val_if_fail(EVENTD_IS_EVENT(self), G_MININT64);

    return self->priv->timeout;
}

const gchar *
eventd_event_get_data(EventdEvent *self, const gchar *name)
{
    g_return_val_if_fail(EVENTD_IS_EVENT(self), NULL);

    if ( self->priv->data == NULL )
        return NULL;

    return g_hash_table_lookup(self->priv->data, name);
}

GHashTable *
eventd_event_get_all_data(EventdEvent *self)
{
    g_return_val_if_fail(EVENTD_IS_EVENT(self), NULL);

    return self->priv->data;
}
