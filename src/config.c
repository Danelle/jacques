/*
 * config.c
 * Copyright (C) 2015 Wiky L <wiiiky@yeah.net>
 * 
 * Jacques is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Jacques is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <gio/gio.h>


typedef void (*ParseLineFunc) (const gchar * line, gpointer user_data);
/*
 * Reads the configuration file, and call func() on each line
 * Returns 1 on success,otherwise 0
 */
static int read_config_file(const gchar * filepath, ParseLineFunc func,
                            gpointer user_data);


/*******************************************/
static inline JaConfig *ja_config_new_default();
static inline JaConfig *ja_config_new0();
static inline void ja_config_free(JaConfig * cfg);
static inline void ja_config_set_default(JaConfig * cfg);


static void parse_line(const gchar * line, gpointer user_data)
{
    g_printf("%s\n", line);
}

/*
 * Parses configuration file CONFIG_FILEPATH
 * Returns NULL on error
 */
JaConfig *ja_config_load()
{
    /* TODO */
    JaConfig *cfg = ja_config_new0();
    if (!read_config_file(CONFIG_FILEPATH, parse_line, cfg)) {
        ja_config_free(cfg);
        return NULL;
    }
    ja_config_set_default(cfg);
    return cfg;
}



static inline JaConfig *ja_config_new_default()
{
    JaConfig *cfg = (JaConfig *) g_malloc(sizeof(JaConfig));
    cfg->user = g_strdup(DEFAULT_USER);
    cfg->group = g_strdup(DEFAULT_GROUP);
    cfg->log_location = g_strdup(DEFAULT_LOG_LOCATION);
    cfg->backlog = DEFAULT_BACKLOG;
    return cfg;
}


#define if_null_dup(ptr,data)	if((ptr)==NULL){ \
									ptr=g_strdup(data);	 \
								}
#define if_zero_set(obj,data)	if((obj)==0){	\
									obj=data;	\
								}
static inline void ja_config_set_default(JaConfig * cfg)
{
    if_null_dup(cfg->user, DEFAULT_USER);
    if_null_dup(cfg->group, DEFAULT_GROUP);
    if_null_dup(cfg->log_location, DEFAULT_LOG_LOCATION);
    if_zero_set(cfg->backlog, DEFAULT_BACKLOG);
}

static inline JaConfig *ja_config_new0()
{
    JaConfig *cfg = (JaConfig *) g_malloc0(sizeof(JaConfig));
    return cfg;
}

static inline void ja_config_free(JaConfig * cfg)
{
    g_free(cfg->user);
    g_free(cfg->group);
    g_free(cfg->log_location);
    g_free(cfg);
}


static int read_config_file(const gchar * filepath, ParseLineFunc func,
                            gpointer user_data)
{
    GFile *file = g_file_new_for_path(filepath);
    GFileInputStream *input = g_file_read(file, NULL, NULL);
    if (input == NULL) {
        g_object_unref(file);
        return 0;
    }
    GString *all = g_string_new(NULL);
    gssize n;
    gchar buf[4096];
    while ((n =
            g_input_stream_read(G_INPUT_STREAM(input), buf, sizeof(buf),
                                NULL, NULL)) > 0) {
        g_string_append_len(all, buf, n);
    }
    g_object_unref(file);
    g_object_unref(input);
    if (n != 0) {
        g_string_free(all, TRUE);
        return 0;
    }

    /* all data read */
    guint i;
    guint start = 0;
    for (i = 0; i < all->len; i++) {
        if (all->str[i] == '\n') {
            gchar *line = g_strndup(all->str + start, i - start);
            func(line, user_data);
            g_free(line);
            start = i + 1;
        }
    }
    return 1;
}
