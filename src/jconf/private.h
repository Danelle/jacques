/*
 * private.h
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

#ifndef __J_PRIVATE_H__
#define __J_PRIVATE_H__


#include "struct.h"

JDirective *j_directive_alloc(const gchar * name, const gchar * value);
JDirective *j_directive_alloc_take(gchar * name, gchar * value);
void j_directive_free(JDirective * jd);


JGroup *j_group_alloc(const gchar * name, const gchar * value);
JGroup *j_group_alloc_take(gchar * name, gchar * value);
void j_group_free(JGroup * jg);
JNode *j_group_append_node(JGroup * g, JNode * n);
JGroup *j_group_append_group(JGroup * g, JGroup * g2);
JDirective *j_group_append_directive(JGroup * g, JDirective * d);

JNode *j_node_alloc(JNodeType type, gpointer data);
void j_node_free(JNode * n);

JParser *j_parser_alloc(const gchar * name);
JNode *j_parser_append_node(JParser * p, JNode * n);
JDirective *j_parser_append_directive(JParser * p, JDirective * jd);
JGroup *j_parser_append_group(JParser * p, JGroup * jg);

#endif                          /* __J_PRIVATE_H__ */
