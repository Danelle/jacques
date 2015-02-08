/*
 * struct.h
 *
 * Copyright (C) 2015 - Wiky L <wiiiiky@outlook.com>
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

#ifndef __J_STRUCT_H__
#define __J_STRUCT_H__


#include <glib.h>


typedef struct _JDirective JDirective;
typedef struct _JGroup JGroup;
typedef struct _JNode JNode;
typedef struct _JParser JParser;


const gchar *j_directive_get_name(JDirective * jd);
const gchar *j_directive_get_value(JDirective * jd);

const gchar *j_group_get_name(JGroup * jg);
const gchar *j_group_get_value(JGroup * jg);
GList *j_group_get_nodes(JGroup * jg);


typedef enum {
    J_NODE_TYPE_DIRECTIVE,
    J_NODE_TYPE_GROUP,
} JNodeType;

const gchar *j_node_get_name(JNode * n);
JNodeType j_node_get_type(JNode * n);
JDirective *j_node_get_directive(JNode * n);
JGroup *j_node_get_group(JNode * n);

#define j_node_is_directive(n)    \
        (j_node_get_type(n)==J_NODE_TYPE_DIRECTIVE)
#define j_node_is_group(n)        \
        (j_node_get_type(n)==J_NODE_TYPE_GROUP)


const gchar *j_parser_get_name(JParser * p);
GList *j_parser_get_root(JParser * p);
void j_parser_free(JParser * p);

GList *j_parser_get_directive(JParser * p, const gchar * name);
GList *j_parser_get_group(JParser * p, const gchar * name);

JDirective *j_parser_get_directive_last(JParser * p, const gchar * name);
JGroup *j_parser_get_group_last(JParser * p, const gchar * name);


#endif
