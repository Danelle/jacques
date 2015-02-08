/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * lib.c
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
 *
 * libjconf is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libjconf is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jconf.h"
#include "private.h"
#include <gio/gio.h>
#include <stdarg.h>
#include <string.h>


static inline void fill_error(GError ** error, const gchar * fmt, ...);
static inline gchar *readall(const gchar * path, GError ** error);

/*
 * Parses a file and returns a JParser on success
 * Returns NULL on error
 */
JParser *j_parse(const gchar * path, GError ** error)
{
    JParser *p = j_parser_alloc(path);
    if (!j_parse_more(p, path, error)) {
        j_parser_free(p);
        return NULL;
    }
    return p;
}


typedef enum {
    J_STATE_NEW = 0,
    J_STATE_DIRECTIVE_NAME,
    J_STATE_DIRECTIVE_NAME_END,
    J_STATE_DIRECTIVE_VALUE,
    J_STATE_GROUP,
    J_STATE_GROUP_START,
    J_STATE_GROUP_START_NAME,
    J_STATE_GROUP_START_NAME_END,
    J_STATE_GROUP_START_VALUE,
    J_STATE_GROUP_END,
    J_STATE_GROUP_END_NAME,
    J_STATE_GROUP_END_NAME_END,
    J_STATE_COMMENT,
} JParserState;

#define j_isnewline(c)  ((c)=='\n')
#define j_iscomment(c)  ((c)=='#')
#define j_isalpha(c)    (g_ascii_isalpha(c))
#define j_isspace(c)  (g_ascii_isspace(c)||(c)=='\t')
#define j_isgroup(c)   ((c)=='<')
#define j_isname(c)     (g_ascii_isalnum(c)||c=='_')
#define j_isalnum(c)    (g_ascii_isalnum(c))
#define j_isgroupend(c)    (c=='/')
#define j_isclose(c)    ((c)=='>')


/*
 * Parses a file with an existing JParser.
 */
gboolean j_parse_more(JParser * p, const gchar * path, GError ** error)
{
    gboolean ret = FALSE;

    gchar *all = readall(path, error);
    if (all == NULL) {
        return ret;
    }
    guint i, len = strlen(all);

    JParserState state = J_STATE_NEW;
    guint line = 1;
    guint start = 0;
    gchar *dname = NULL;
    GList *groups = NULL;       /* stack */
    for (i = 0; i < len; i++) {
        gchar c = all[i];
        switch (state) {
        case J_STATE_COMMENT:
            if (j_isnewline(c)) {
                state = J_STATE_NEW;
                line++;
            }
            break;
        case J_STATE_NEW:
            if (j_iscomment(c)) {
                state = J_STATE_COMMENT;
            } else if (j_isnewline(c)) {
                /* skip empty line */
                line++;
            } else if (j_isalpha(c)) {
                state = J_STATE_DIRECTIVE_NAME;
                start = i;
            } else if (j_isgroup(c)) {
                state = J_STATE_GROUP;
            } else if (!j_isspace(c)) {
                fill_error(error,
                           "directive name must start with "
                           "ascii letter, at %s:%u", path, line);
                goto OUT;
            }
            break;
        case J_STATE_DIRECTIVE_NAME:
            if (j_iscomment(c) || j_isnewline(c)) {
                dname = g_strndup(all + start, i - start);
                if (groups == NULL && g_strcmp0(INCLUDE_CONF, dname) == 0) {
                    fill_error(error, "missing file path, at %s: %u", path,
                               line);
                    goto OUT;
                }
                JDirective *d = j_directive_alloc_take(dname, NULL);
                dname = NULL;
                if (groups == NULL) {
                    j_parser_append_directive(p, d);
                } else {
                    j_group_append_directive((JGroup *)
                                             groups->data, d);
                }
                if (j_iscomment(c)) {
                    state = J_STATE_COMMENT;
                } else {
                    state = J_STATE_NEW;
                    line++;
                }
            } else if (j_isspace(c)) {
                state = J_STATE_DIRECTIVE_NAME_END;
                dname = g_strndup(all + start, i - start);
            } else if (!j_isname(c)) {
                fill_error(error,
                           "directive name must only contain letters,"
                           " digits or underline, at %s:%u", path, line);
                goto OUT;

            }
            break;
        case J_STATE_DIRECTIVE_NAME_END:
            if (j_iscomment(c) || j_isnewline(c)) {
                if (groups == NULL && g_strcmp0(INCLUDE_CONF, dname) == 0) {
                    fill_error(error, "missing file path, at %s: %u", path,
                               line);
                    goto OUT;
                }
                JDirective *d = j_directive_alloc_take(dname, NULL);
                if (groups == NULL) {
                    j_parser_append_directive(p, d);
                } else {
                    j_group_append_directive((JGroup *)
                                             groups->data, d);
                }
                dname = NULL;
                if (j_iscomment(c)) {
                    state = J_STATE_COMMENT;
                } else {
                    state = J_STATE_NEW;
                    line++;
                }
            } else if (!j_isspace(c)) {
                state = J_STATE_DIRECTIVE_VALUE;
                start = i;
            }
            break;
        case J_STATE_DIRECTIVE_VALUE:
            if (j_iscomment(c) || j_isnewline(c)) {
                if (groups == NULL && g_strcmp0(INCLUDE_CONF, dname) == 0) {
                    gchar *path =
                        g_strstrip(g_strndup(all + start, i - start));
                    gboolean more = j_parse_more(p, path, error);
                    g_free(path);
                    if (!more) {
                        goto OUT;
                    }
                    g_free(dname);
                } else {
                    JDirective *d = j_directive_alloc_take(dname,
                                                           g_strndup(all +
                                                                     start,
                                                                     i -
                                                                     start));
                    if (groups == NULL) {
                        j_parser_append_directive(p, d);
                    } else {
                        j_group_append_directive((JGroup *)
                                                 groups->data, d);
                    }
                }
                dname = NULL;
                if (j_iscomment(c)) {
                    state = J_STATE_COMMENT;
                } else {
                    state = J_STATE_NEW;
                    line++;
                }
            }
            break;
        case J_STATE_GROUP:
            if (j_isgroupend(c)) {
                state = J_STATE_GROUP_END;
            } else if (j_isspace(c)) {
                state = J_STATE_GROUP_START;
            } else if (j_isalpha(c)) {
                state = J_STATE_GROUP_START_NAME;
                start = i;
            } else {
                fill_error(error, "unexpected character %c, at %s: %u", c,
                           path, line);
                goto OUT;
            }
            break;
        case J_STATE_GROUP_START:
            if (j_isalpha(c)) {
                state = J_STATE_GROUP_START_NAME;
                start = i;
            } else if (!j_isspace(c)) {
                fill_error(error, "unexpected character %c, at %s: %u", c,
                           path, line);
                goto OUT;
            }
            break;
        case J_STATE_GROUP_START_NAME:
            if (j_iscomment(c)) {
                fill_error(error,
                           "unexpected character %c in group name, at %s: %u",
                           c, path, line);
                goto OUT;
            } else if (j_isnewline(c)) {
                fill_error(error, "unexpected EOL at %s: %u", path, line);
                goto OUT;
            } else if (j_isclose(c)) {
                JGroup *g =
                    j_group_alloc_take(g_strndup(all + start, i - start),
                                       NULL);
                if (groups == NULL) {
                    g = j_parser_append_group(p, g);
                } else {
                    g = j_group_append_group((JGroup *)
                                             groups->data, g);
                }
                groups = g_list_prepend(groups, g);
                state = J_STATE_NEW;
            } else if (j_isspace(c)) {
                state = J_STATE_GROUP_START_NAME_END;
                dname = g_strndup(all + start, i - start);
            } else if (!j_isname(c)) {
                fill_error(error, "unexpected character %c, at %s: %u", c,
                           path, line);
                goto OUT;
            }
            break;
        case J_STATE_GROUP_START_NAME_END:
            if (j_iscomment(c)) {
                fill_error(error, "unexpected character %c, at %s: %u", c,
                           path, line);
                goto OUT;
            } else if (j_isnewline(c)) {
                fill_error(error, "unexpected EOL at %s: %u", path, line);
                goto OUT;
            } else if (j_isclose(c)) {
                JGroup *g = j_group_alloc_take(dname, NULL);
                if (groups == NULL) {
                    g = j_parser_append_group(p, g);
                } else {
                    g = j_group_append_group((JGroup *)
                                             groups->data, g);
                }
                groups = g_list_prepend(groups, g);
                dname = NULL;
                state = J_STATE_NEW;
            } else if (!j_isspace(c)) {
                state = J_STATE_GROUP_START_VALUE;
                start = i;
            }
            break;
        case J_STATE_GROUP_START_VALUE:
            if (j_iscomment(c)) {
                fill_error(error, "unexpected character %c at %s: %u", c,
                           path, line);
                goto OUT;
            } else if (j_isnewline(c)) {
                fill_error(error, "unexpected EOL at %s: %u", path, line);
                goto OUT;
            } else if (j_isclose(c)) {
                JGroup *g = j_group_alloc_take(dname,
                                               g_strndup(all + start,
                                                         i - start));
                if (groups == NULL) {
                    g = j_parser_append_group(p, g);
                } else {
                    g = j_group_append_group((JGroup *)
                                             groups->data, g);
                }
                groups = g_list_prepend(groups, g);
                dname = NULL;
                state = J_STATE_NEW;
            }
            break;
        case J_STATE_GROUP_END:
            if (j_isalpha(c)) {
                state = J_STATE_GROUP_END_NAME;
                start = i;
            } else if (!j_isspace(c)) {
                fill_error(error, "unexpected character %c at %s: %u",
                           path, line);
                goto OUT;
            }
            break;
        case J_STATE_GROUP_END_NAME:
            if (j_isspace(c)) {
                dname = g_strndup(all + start, i - start);
                state = J_STATE_GROUP_END_NAME_END;
            } else if (j_isclose(c)) {
                dname = g_strndup(all + start, i - start);
                if (groups) {
                    JGroup *g = (JGroup *) groups->data;
                    if (g_strcmp0(dname, j_group_get_name(g))) {
                        fill_error(error,
                                   "group name doesn't match, at %s, %u",
                                   path, line);
                        goto OUT;
                    }
                    GList *next = g_list_next(groups);
                    g_list_free1(groups);
                    groups = next;
                    g_free(dname);
                    dname = NULL;
                } else {
                    fill_error(error, "unexpected group end, at %s: %u",
                               path, line);
                    goto OUT;
                }
                state = J_STATE_NEW;
            } else if (!j_isname(c)) {
                fill_error(error,
                           "unexpected character %c in group name, at %s: %u",
                           c, path, line);
                goto OUT;
            }
            break;
        case J_STATE_GROUP_END_NAME_END:
            if (j_isclose(c)) {
                dname = g_strndup(all + start, i - start);
                if (groups) {
                    JGroup *g = (JGroup *) groups->data;
                    if (g_strcmp0(dname, j_group_get_name(g))) {
                        fill_error(error,
                                   "group name doesn't match, at %s, %u",
                                   path, line);
                        goto OUT;
                    }
                    GList *next = g_list_next(groups);
                    g_list_free1(groups);
                    groups = next;
                    g_free(dname);
                    dname = NULL;
                } else {
                    fill_error(error, "unexpected group end, at %s: %u",
                               path, line);
                    goto OUT;
                }
                state = J_STATE_NEW;
            } else if (!j_isspace(c)) {
                fill_error(error, "unexpected character %c, at %s: %u",
                           path, line);
                goto OUT;
            }
            break;
        }
    }
    if (state != J_STATE_NEW) {
        fill_error(error, "unexpected EOF at %s: %u", path, line);
        goto OUT;
    } else if (groups != NULL) {
        fill_error(error, "invalid group end at %s", path);
        goto OUT;
    }
    ret = TRUE;
  OUT:
    g_free(all);
    g_list_free(groups);
    g_free(dname);
    return ret;
}


static inline void fill_error(GError ** error, const gchar * fmt, ...)
{
    if (error == NULL) {
        return;
    }
    va_list vl;
    va_start(vl, fmt);
    *error = g_error_new_valist(1, 1, fmt, vl);
    va_end(vl);
}

static inline gchar *readall(const gchar * path, GError ** error)
{
    GFile *file = g_file_new_for_path(path);
    GFileInputStream *input = g_file_read(file, NULL, error);
    g_object_unref(file);
    if (input == NULL) {
        return NULL;
    }
    gchar buffer[4096];
    gssize n;
    GString *data = g_string_new(NULL);
    while ((n = g_input_stream_read((GInputStream *) input,
                                    buffer,
                                    sizeof(buffer) / sizeof(gchar),
                                    NULL, error)) > 0) {
        g_string_append_len(data, buffer, n);
    }
    g_object_unref(input);
    g_string_append_c(data, '\n');
    return g_string_free(data, FALSE);
}
