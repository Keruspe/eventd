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

#include "common.h"
#include "setters.h"

typedef struct {
    EventdEvent *event;
} SettersData;

static void
_init_data(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    data->event = eventd_event_new(EVENTD_EVENT_TEST_CATEGORY, EVENTD_EVENT_TEST_NAME);
}

static void
_clean_data(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    g_object_unref(data->event);
}

static void
_test_new_good_good(gpointer fixture, gconstpointer user_data)
{
    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_new(EVENTD_EVENT_TEST_CATEGORY, EVENTD_EVENT_TEST_NAME);
        exit(0);
    }
    g_test_trap_assert_passed();
}

static void
_test_new_null_good(gpointer fixture, gconstpointer user_data)
{
    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_new(NULL, EVENTD_EVENT_TEST_NAME);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_new_good_null(gpointer fixture, gconstpointer user_data)
{
    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_new(NULL, NULL);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_set_timeout_notnull_good(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_set_timeout(data->event, 1);
        exit(0);
    }
    g_test_trap_assert_passed();
}

static void
_test_set_timeout_null_good(gpointer fixture, gconstpointer user_data)
{
    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_set_timeout(NULL, EVENTD_EVENT_TEST_TIMEOUT);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_set_timeout_notnull_bad(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_set_timeout(data->event, -2);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_add_data_notnull_good_good(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_data(data->event, EVENTD_EVENT_TEST_DATA_NAME, EVENTD_EVENT_TEST_DATA_CONTENT);
        exit(0);
    }
    g_test_trap_assert_passed();
}

static void
_test_add_data_null_good_good(gpointer fixture, gconstpointer user_data)
{
    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_data(NULL, EVENTD_EVENT_TEST_DATA_NAME, EVENTD_EVENT_TEST_DATA_CONTENT);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_add_data_notnull_bad_good(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_data(data->event, NULL, EVENTD_EVENT_TEST_DATA_CONTENT);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_add_data_notnull_good_bad(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_data(data->event, EVENTD_EVENT_TEST_DATA_NAME, NULL);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_add_answer_notnull_good(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_answer(data->event, EVENTD_EVENT_TEST_ANSWER);
        exit(0);
    }
    g_test_trap_assert_passed();
}

static void
_test_add_answer_null_good(gpointer fixture, gconstpointer user_data)
{
    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_answer(NULL, EVENTD_EVENT_TEST_ANSWER);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_add_answer_notnull_bad(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_answer(data->event, NULL);
        exit(0);
    }
    g_test_trap_assert_failed();
}
static void
_test_add_answer_data_notnull_good_good(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_answer_data(data->event, EVENTD_EVENT_TEST_DATA_NAME, EVENTD_EVENT_TEST_DATA_CONTENT);
        exit(0);
    }
    g_test_trap_assert_passed();
}

static void
_test_add_answer_data_null_good_good(gpointer fixture, gconstpointer user_data)
{
    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_answer_data(NULL, EVENTD_EVENT_TEST_DATA_NAME, EVENTD_EVENT_TEST_DATA_CONTENT);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_add_answer_data_notnull_bad_good(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_answer_data(data->event, NULL, EVENTD_EVENT_TEST_DATA_CONTENT);
        exit(0);
    }
    g_test_trap_assert_failed();
}

static void
_test_add_answer_data_notnull_good_bad(gpointer fixture, gconstpointer user_data)
{
    SettersData *data = fixture;

    if ( ! g_test_undefined() )
            return;
    if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
        eventd_event_add_answer_data(data->event, EVENTD_EVENT_TEST_DATA_NAME, NULL);
        exit(0);
    }
    g_test_trap_assert_failed();
}

void
eventd_tests_unit_eventd_event_suite_setters(void)
{
    GTestSuite *suite;

    suite = g_test_create_suite("EventdEvent setters test suite");

    g_test_suite_add(suite, g_test_create_case("new(category, name)",                sizeof(SettersData), NULL, NULL,       _test_new_good_good,                     NULL));
    g_test_suite_add(suite, g_test_create_case("new(NULL, name)",                    sizeof(SettersData), NULL, NULL,       _test_new_null_good,                     NULL));
    g_test_suite_add(suite, g_test_create_case("new(category, NULL)",                sizeof(SettersData), NULL, NULL,       _test_new_good_null,                     NULL));

    g_test_suite_add(suite, g_test_create_case("set_timeout(event, timeout)",        sizeof(SettersData), NULL, _init_data, _test_set_timeout_notnull_good,          _clean_data));
    g_test_suite_add(suite, g_test_create_case("set_timeout(NULL,)",                 sizeof(SettersData), NULL, NULL,       _test_set_timeout_null_good,             NULL));
    g_test_suite_add(suite, g_test_create_case("set_timeout(event, NULL)",           sizeof(SettersData), NULL, _init_data, _test_set_timeout_notnull_bad,           _clean_data));

    g_test_suite_add(suite, g_test_create_case("add_data(event, name, data)",        sizeof(SettersData), NULL, _init_data, _test_add_data_notnull_good_good,        _clean_data));
    g_test_suite_add(suite, g_test_create_case("add_data(NULL,)",                    sizeof(SettersData), NULL, NULL,       _test_add_data_null_good_good,           NULL));
    g_test_suite_add(suite, g_test_create_case("add_data(event, NULL,)",             sizeof(SettersData), NULL, _init_data, _test_add_data_notnull_bad_good,         _clean_data));
    g_test_suite_add(suite, g_test_create_case("add_data(event, NULL, NULL)",        sizeof(SettersData), NULL, _init_data, _test_add_data_notnull_good_bad,         _clean_data));

    g_test_suite_add(suite, g_test_create_case("add_answer(event, name)",            sizeof(SettersData), NULL, _init_data, _test_add_answer_notnull_good,           _clean_data));
    g_test_suite_add(suite, g_test_create_case("add_answer(NULL,)",                  sizeof(SettersData), NULL, NULL,       _test_add_answer_null_good,              NULL));
    g_test_suite_add(suite, g_test_create_case("add_answer(event, NULL)",            sizeof(SettersData), NULL, _init_data, _test_add_answer_notnull_bad,            _clean_data));

    g_test_suite_add(suite, g_test_create_case("add_answer_data(event, name, data)", sizeof(SettersData), NULL, _init_data, _test_add_answer_data_notnull_good_good, _clean_data));
    g_test_suite_add(suite, g_test_create_case("add_answer_data(NULL,)",             sizeof(SettersData), NULL, NULL,       _test_add_answer_data_null_good_good,    NULL));
    g_test_suite_add(suite, g_test_create_case("add_answer_data(event, NULL,)",      sizeof(SettersData), NULL, _init_data, _test_add_answer_data_notnull_bad_good,  _clean_data));
    g_test_suite_add(suite, g_test_create_case("add_answer_data(event, NULL, NULL)", sizeof(SettersData), NULL, _init_data, _test_add_answer_data_notnull_good_bad,  _clean_data));

    g_test_suite_add_suite(g_test_get_root(), suite);
}
