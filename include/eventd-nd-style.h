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

#ifndef __EVENTD_ND_STYLE_H__
#define __EVENTD_ND_STYLE_H__

typedef struct {
    gdouble r;
    gdouble g;
    gdouble b;
    gdouble a;
} Colour;

typedef struct _EventdNdStyle EventdNdStyle;

EventdNdStyle *eventd_nd_style_new(EventdNdStyle *parent);
void eventd_nd_style_free(EventdNdStyle *style);

gint eventd_nd_style_get_bubble_min_width(EventdNdStyle *style);
gint eventd_nd_style_get_bubble_max_width(EventdNdStyle *style);

gint eventd_nd_style_get_bubble_padding(EventdNdStyle *style);
gint eventd_nd_style_get_bubble_radius(EventdNdStyle *style);
Colour eventd_nd_style_get_bubble_colour(EventdNdStyle *style);

gint eventd_nd_style_get_icon_max_width(EventdNdStyle *style);
gint eventd_nd_style_get_icon_max_height(EventdNdStyle *style);
gint eventd_nd_style_get_icon_spacing(EventdNdStyle *style);
gdouble eventd_nd_style_get_overlay_scale(EventdNdStyle *style);

const gchar *eventd_nd_style_get_title_font(EventdNdStyle *style);
Colour eventd_nd_style_get_title_colour(EventdNdStyle *style);

gint eventd_nd_style_get_message_spacing(EventdNdStyle *style);
guint8 eventd_nd_style_get_message_max_lines(EventdNdStyle *style);
const gchar *eventd_nd_style_get_message_font(EventdNdStyle *style);
Colour eventd_nd_style_get_message_colour(EventdNdStyle *style);

#endif /* __EVENTD_ND_STYLE_H__ */
