/*
 * libeventd - Internal helper
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

#ifndef __LIBEVENTD_REGEX_H__
#define __LIBEVENTD_REGEX_H__

void libeventd_regex_init();
void libeventd_regex_clean();

gchar *libeventd_regex_replace_client_name(const gchar *target, const gchar *clieant_name);
gchar *libeventd_regex_replace_event_name(const gchar *target, const gchar *event_name);
gchar *libeventd_regex_replace_event_data(const gchar *target, GHashTable *event_data, GRegexEvalCallback callback);

#endif /* __LIBEVENTD_REGEX_H__ */