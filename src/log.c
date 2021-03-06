/*
 * log.c
 *
 * Copyright (C) 2015 - Wiky L <wiiiky@outlook.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "log.h"
#include "config.h"
#include "utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


static void warning_log_handler(const gchar * domain,
                                GLogLevelFlags level,
                                const gchar * message, gpointer user_data);
static void message_log_handler(const gchar * domain,
                                GLogLevelFlags level,
                                const gchar * message, gpointer user_data);


static gint error_fd = -1;
static gint message_fd = -1;


static inline gint log_open(const gchar * name);


gboolean initialize_default_log(void)
{
    g_log_set_handler(NULL, G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL
                      | G_LOG_FLAG_RECURSION, warning_log_handler, NULL);
    g_log_set_handler(NULL, G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL
                      | G_LOG_FLAG_RECURSION, warning_log_handler, NULL);
    g_log_set_handler(NULL, G_LOG_LEVEL_MESSAGE, message_log_handler,
                      NULL);

    error_fd = open_appendable(CONFIG_LOG_ERROR_FILE);
    message_fd = open_appendable(CONFIG_LOG_FILE);

    if (error_fd < 0 || message_fd < 0) {
        return FALSE;
    }
    return TRUE;
}


/*
 * Re-set log handlers using specified files
 * @param normal, the file that logs normal message
 * @param error, the file that logs error message
 */
gboolean set_custom_log(const gchar * normal, const gchar * error)
{
    if (normal) {
        close(message_fd);
        message_fd = log_open(normal);
    }
    if (error) {
        close(error_fd);
        error_fd = log_open(error);
    }
    if (error_fd < 0 || message_fd < 0) {
        return FALSE;
    }
    return TRUE;
}


static inline gint log_open(const gchar * name)
{
    if (g_str_has_prefix(name, "/")) {  /* absolute path */
        return open_appendable(name);
    }
    gchar buf[4096];
    g_snprintf(buf, sizeof(buf) / sizeof(gchar), "%s/%s",
               CONFIG_LOG_LOCATION, name);
    return open_appendable(buf);
}



static void inline log_write(gint output, const gchar * message)
{
    GDateTime *dt = g_date_time_new_now_local();
    gchar *time = g_date_time_format(dt, "[%x %X]");

    gchar buf[4096];
    g_snprintf(buf, sizeof(buf) / sizeof(gchar), "%s - %s\n", time,
               message);
    g_free(time);
    g_date_time_unref(dt);

    write(output, buf, strlen(buf));
}

static void warning_log_handler(const gchar * domain,
                                GLogLevelFlags level,
                                const gchar * message, gpointer user_data)
{
    if (error_fd < 0) {
        return;
    }

    log_write(error_fd, message);

    if (level & G_LOG_LEVEL_ERROR) {
        exit(-1);
    }
}

static void message_log_handler(const gchar * domain,
                                GLogLevelFlags level,
                                const gchar * message, gpointer user_data)
{
    if (message_fd < 0) {
        return;
    }

    log_write(message_fd, message);
}
