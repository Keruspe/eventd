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

typedef enum {
    EVENTD_ND_STYLE_ANCHOR_TOP_LEFT,
    EVENTD_ND_STYLE_ANCHOR_TOP_RIGHT,
    EVENTD_ND_STYLE_ANCHOR_BOTTOM_LEFT,
    EVENTD_ND_STYLE_ANCHOR_BOTTOM_RIGHT
} EventdNdStyleAnchor;

EventdNdStyle *eventd_nd_style_new(void);
void eventd_nd_style_free(EventdNdStyle *style);

EventdNdStyleAnchor eventd_nd_style_get_bubble_anchor(EventdNdStyle *style);
gint eventd_nd_style_get_bubble_margin(EventdNdStyle *style);

#endif /* __EVENTD_ND_STYLE_H__ */
