/*
 * jfile.c
 *
 * Copyright (C) 2015 - Wiky L
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
#include "jfile.h"
#include <stdio.h>


struct _JFile {
    FILE *fp;
    GByteArray *buf;
};


JFile *j_file_open(const gchar * path, const gchar * mode)
{
    FILE *fp = fopen(path, mode);
    if (!fp) {
        return NULL;
    }
    JFile *f = (JFile *) g_slice_alloc(sizeof(JFile));
    f->fp = fp;
    f->buf = g_byte_array_new();
    return f;
}

gchar *j_file_readline(JFile * jf)
{
    gchar buf[4096];
    gint i = 0;
    do {
        g_byte_array_append(jf->buf, buf, i);
        guint len = jf->buf->len;
        const gchar *data = (const gchar *) jf->buf->data;
        for (i = 0; i < len; i++) {
            if (data[i] == '\n') {
                gchar *line = g_strndup(data, i);
                g_byte_array_remove_range(jf->buf, 0, i + 1);
                return line;
            }
        }
    } while ((i =
              fread(buf, sizeof(gchar), sizeof(buf) / sizeof(gchar),
                    jf->fp)) > 0);

    guint len = jf->buf->len;
    const gchar *data = (const gchar *) jf->buf->data;
    if (len == 0) {
        return NULL;
    }
    gchar *line = g_strndup(data, len);
    g_byte_array_remove_range(jf->buf, 0, len);
    return line;
}

void j_file_close(JFile * jf)
{
    fclose(jf->fp);
    g_byte_array_free(jf->buf, TRUE);
    g_slice_free1(sizeof(JFile), jf);
}
