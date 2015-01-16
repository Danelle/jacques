/*
 * jfile.h
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
#ifndef __J_FILE_H__
#define __J_FILE_H__

#include <glib.h>


typedef struct _JFile JFile;


JFile *j_file_open(const gchar * path, const gchar * mode);

gchar *j_file_readline(JFile * jf);

void j_file_close(JFile * jf);



#endif