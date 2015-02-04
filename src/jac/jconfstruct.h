/*
 * jconfstruct.h
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

#ifndef __J_CONF_STRUCTURE_H__
#define __J_CONF_STRUCTURE_H__

#include <glib.h>

typedef enum {
    J_CONF_NODE_TYPE_DIRECTIVE,
    J_CONF_NODE_TYPE_GROUP,
} JConfNodeType;


typedef struct {
    gchar *name;
    gchar *value;               /* allow NULL */
} JConfDirective;
#define j_conf_directive_get_name(d)    ((d)->name)
#define j_conf_directive_get_value(d)  ((d)->value)

JConfDirective *j_conf_directive_new(const gchar * name);
JConfDirective *j_conf_directive_new_take(gchar * name);
JConfDirective *j_conf_directive_new_with_values(const gchar * name, const gchar * value);  /* takes the values */
JConfDirective *j_conf_directive_new_with_values_take(gchar * name,
                                                      gchar * value);
void j_conf_directive_free(JConfDirective * d);


/******************** JConfGroup ***************************/
typedef struct {
    gchar *name;
    GList *nodes;
} JConfGroup;
#define j_conf_group_get_name(g)    ((g)->name)
#define j_conf_group_get_nodes(g)   ((g)->nodes)

JConfDirective *j_conf_group_get_directive(JConfGroup * g,
                                           const gchar * name);
const gchar *j_conf_group_get_directive_value(JConfGroup * g,
                                              const gchar * name);
gint32 j_conf_group_get_directive_integer(JConfGroup * g,
                                          const gchar * name);
JConfGroup *j_conf_group_get_group(JConfGroup * g, const gchar * name);

JConfGroup *j_conf_group_new(const gchar * name);
JConfGroup *j_conf_group_new_take(gchar * name);
void j_conf_group_append_node(JConfGroup * g, gpointer node);
void j_conf_group_append_directive(JConfGroup * g, JConfDirective * d);
void j_conf_group_append_group(JConfGroup * g, JConfGroup * g2);
void j_conf_group_free(JConfGroup * g);

/******************** JConfNode ***************************/
typedef struct {
    JConfNodeType type;
    union {
        JConfDirective *directive;
        JConfGroup *group;
    } data;
} JConfNode;
#define j_conf_node_is_directive(n) ((n)->type==J_CONF_NODE_TYPE_DIRECTIVE)
#define j_conf_node_is_group(n)     ((n)->type==J_CONF_NODE_TYPE_GROUP)
#define j_conf_node_get_type(n) ((n)->type)
#define j_conf_node_get_directive(n)    ((n)->data.directive)
#define j_conf_node_get_group(n)    ((n)->data.group)

JConfNode *j_conf_node_new(JConfNodeType type, gpointer data);
void j_conf_node_free(JConfNode * n);


/********************** JConfParser **********************/
typedef JConfGroup JConfRoot;

typedef struct {
    gchar *name;                /* filename */
    JConfRoot *root;            /* groups */
} JConfParser;
#define j_conf_parser_get_name(p)   ((p)->name)
#define j_conf_parser_get_root(p)   ((p)->root)

JConfParser *j_conf_parser_new(const gchar * name);
void j_conf_parser_free(JConfParser * parser);


#endif                          /* __J_CONF_STRUCTURE_H__ */
