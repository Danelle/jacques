/*
 * jconf.c
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
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

#include "jconf.h"
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>


static inline gchar *j_str_preprocess(gchar * line)
{
    line = g_strstrip(line);
    gchar *c = g_strstr_len(line, -1, "#");
    if (c) {
        *c = '\0';
    }
    gint len = strlen(line);
    gint i = 0;
    for (; i < len; i++) {
        if (line[i] == '\t') {
            line[i] = ' ';
        } else if (line[i] == '\n') {
            line[i] = ' ';
        }
    }
    return line;
}

static inline gchar *j_file_readall(const gchar * file);


static inline gboolean j_conf_parse_data(const gchar * data,
                                         JConfGroup * group);


/*
 * Parses a file,
 * Returns a JConfParser on success
 * Otherwise NULL
 */
JConfParser *j_conf_parse(const gchar * filepath)
{
    JConfParser *p = j_conf_parser_new(filepath);
    if (j_conf_parser_parse(p, filepath)) {
        return p;
    }
    j_conf_parser_free(p);
    return NULL;
}

gboolean j_conf_parser_parse(JConfParser * p, const gchar * filepath)
{
    gchar *data = j_file_readall(filepath);
    if (data == NULL) {
        g_warning("Unable to open %s", filepath);
        return FALSE;
    }
    JConfGroup *group = j_conf_parser_get_root(p);
    gboolean ret = j_conf_parse_data(data, group);
    g_free(data);
    if (!ret) {
        g_warning("Fail to parse %s", filepath);
        return FALSE;
    }
    return TRUE;
}

static inline gchar *j_file_readall(const gchar * filepath)
{
    GFile *file = g_file_new_for_path(filepath);
    GFileInputStream *input = g_file_read(file, NULL, NULL);
    g_object_unref(file);
    if (input == NULL) {
        return NULL;
    }
    GString *string = g_string_new(NULL);
    gchar buffer[4096];
    gssize n;
    while ((n = g_input_stream_read((GInputStream *) input,
                                    buffer,
                                    sizeof(buffer) / sizeof(gchar),
                                    NULL, NULL)) > 0) {
        string = g_string_append_len(string, (const gchar *) buffer, n);
    }
    g_object_unref(input);

    gchar *data = g_string_free(string, FALSE);
    string = g_string_new(NULL);
    gchar **lines = g_strsplit(data, "\n", -1);
    gchar **line = lines;
    n = 0;
    while (*line) {
        n++;
        gchar *tmp = g_strdup(*line);
        if (g_str_has_prefix(tmp, "#include ")) {
            const gchar *path = tmp + 9;
            gchar *more = j_file_readall(path);
            if (more) {
                string = g_string_append(string, more);
                g_free(more);
            } else {
                g_warning("Unable to open %s", path);
            }
        } else {
            tmp = j_str_preprocess(tmp);
            string = g_string_append(string, tmp);
            string = g_string_append_c(string, '\n');
        }
        g_free(tmp);
        line++;
    }
    g_strfreev(lines);
    g_free(data);

    data = g_string_free(string, FALSE);
    return data;
}

typedef enum {
    J_CONF_PARSE_STATE_START,
    J_CONF_PARSE_STATE_NAME,
    J_CONF_PARSE_STATE_NAME_END,
    J_CONF_PARSE_STATE_GROUP_START,
    J_CONF_PARSE_STATE_VALUE_START,
} JConfParseState;

static inline gboolean j_conf_parse_data(const gchar * data,
                                         JConfGroup * group)
{
    const gchar *ptr = data;
    const gchar *value_ptr = NULL;
    const gchar *group_ptr = NULL;
    guint group_count = 0;
    gchar *name = NULL;
    gchar *value = NULL;
    JConfParseState state = J_CONF_PARSE_STATE_START;
    while (*ptr) {
        gchar c = *ptr;
        switch (state) {
        case J_CONF_PARSE_STATE_START:
            if (g_ascii_isalpha(c)) {
                state = J_CONF_PARSE_STATE_NAME;
            } else if (!g_ascii_isspace(c) && c != ';') {
                goto OUT;
            }
            break;
        case J_CONF_PARSE_STATE_NAME:
            if (g_ascii_isspace(c)) {
                name = g_strndup(data,
                                 (gsize) ((gpointer) ptr -
                                          (gpointer) data));
                state = J_CONF_PARSE_STATE_NAME_END;
            } else if (c == '{') {
                name = g_strndup(data,
                                 (gsize) ((gpointer) ptr -
                                          (gpointer) data));
                group_ptr = ptr + 1;
                group_count = 1;
                state = J_CONF_PARSE_STATE_GROUP_START;
            } else if (c == ';') {
                name = g_strndup(data,
                                 (gsize) ((gpointer) ptr -
                                          (gpointer) data));
                JConfDirective *d = j_conf_directive_new_take(name);
                j_conf_group_append_directive(group, d);
                value_ptr = NULL;
                group_ptr = NULL;
                value = NULL;
                name = NULL;
                data = ptr + 1;
                state = J_CONF_PARSE_STATE_START;
            } else if (!g_ascii_isalnum(c)) {
                goto OUT;
            }
            break;
        case J_CONF_PARSE_STATE_NAME_END:
            if (c == '{') {
                state = J_CONF_PARSE_STATE_GROUP_START;
                group_count = 1;
                group_ptr = ptr + 1;
            } else if (!g_ascii_ispunct(c) && g_ascii_isprint(c)) {
                state = J_CONF_PARSE_STATE_VALUE_START;
                value_ptr = ptr;
            } else {
                goto OUT;
            }
            break;
        case J_CONF_PARSE_STATE_VALUE_START:
            if (c == '{' || c == '}') {
                goto OUT;
            } else if (c == ';') {
                value = g_strndup(value_ptr,
                                  (gsize) ((gpointer) ptr -
                                           (gpointer) value_ptr));
                JConfDirective *d =
                    j_conf_directive_new_with_values_take(name, value);
                j_conf_group_append_directive(group, d);
                state = J_CONF_PARSE_STATE_START;
                value_ptr = NULL;
                group_ptr = NULL;
                value = NULL;
                name = NULL;
                data = ptr + 1;
            }
            break;
        case J_CONF_PARSE_STATE_GROUP_START:
            if (c == '}') {
                group_count--;
                if (group_count == 0) {
                    gchar *gdata = g_strndup(group_ptr,
                                             (gsize) ((gpointer) ptr -
                                                      (gpointer)
                                                      group_ptr));
                    JConfGroup *g = j_conf_group_new_take(name);
                    value_ptr = NULL;
                    group_ptr = NULL;
                    value = NULL;
                    name = NULL;
                    data = ptr + 1;
                    state = J_CONF_PARSE_STATE_START;
                    if (j_conf_parse_data(gdata, g)) {
                        j_conf_group_append_group(group, g);
                    } else {
                        j_conf_group_free(g);
                        g_free(gdata);
                        goto OUT;
                    }
                    g_free(gdata);
                } else if (group_count < 0) {
                    goto OUT;
                }
            } else if (c == '{') {
                group_count++;
            }
            break;
        }
        ptr++;
    }

    if (value_ptr || group_ptr) {
        goto OUT;
    }
    return TRUE;
  OUT:
    g_free(name);
    g_free(value);
    return FALSE;
}
