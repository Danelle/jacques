/*
 * jconfstruct.c
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

#include "jconfstruct.h"


static inline void j_conf_directive_merge(JConfDirective * d1,
                                          JConfDirective * d2);
static inline void j_conf_group_merge(JConfGroup * g1, JConfGroup * g2);
static inline void j_conf_node_merge(JConfNode * n1, JConfNode * n2);


JConfDirective *j_conf_directive_new(const gchar * name)
{
    return j_conf_directive_new_with_values(name, NULL);
}

JConfDirective *j_conf_directive_new_take(gchar * name)
{
    return j_conf_directive_new_with_values_take(name, NULL);
}

JConfDirective *j_conf_directive_new_with_values(const gchar * name,
                                                 const gchar * values)
{
    return j_conf_directive_new_with_values_take(g_strdup(name),
                                                 g_strdup(values));
}

JConfDirective *j_conf_directive_new_with_values_take(gchar * name,
                                                      gchar * value)
{
    JConfDirective *d =
        (JConfDirective *) g_slice_alloc(sizeof(JConfDirective));
    d->name = g_strstrip(name);
    if (value) {
        d->value = g_strstrip(value);
    } else {
        d->value = NULL;
    }
    return d;
}

void j_conf_directive_free(JConfDirective * d)
{
    g_free(d->name);
    g_free(d->value);
    g_slice_free1(sizeof(JConfDirective), d);
}

static inline void j_conf_directive_merge(JConfDirective * d1,
                                          JConfDirective * d2)
{
    if (g_strcmp0(j_conf_directive_get_name(d1),
                  j_conf_directive_get_name(d2)) != 0) {
        return;
    }
    g_free(d1->value);
    d1->value = g_strdup(d2->value);
    j_conf_directive_free(d2);
}


JConfGroup *j_conf_group_new(const gchar * name)
{
    return j_conf_group_new_take(g_strdup(name));
}

JConfGroup *j_conf_group_new_take(gchar * name)
{
    JConfGroup *g = (JConfGroup *) g_slice_alloc(sizeof(JConfGroup));
    if (name) {
        name = g_strstrip(name);
    }
    g->name = name;
    g->nodes = NULL;
    return g;
}

void j_conf_group_append_node(JConfGroup * g, gpointer node)
{
    g->nodes = g_list_append(g->nodes, node);
}

static gint j_conf_group_compare_directive(gconstpointer a,
                                           gconstpointer b)
{
    const JConfNode *node = (const JConfNode *) a;
    const gchar *name = (const gchar *) b;
    if (j_conf_node_is_directive(node)) {
        return g_strcmp0(name,
                         j_conf_directive_get_name
                         (j_conf_node_get_directive(node)));
    }
    return -1;
}

void j_conf_group_append_directive(JConfGroup * g, JConfDirective * d)
{
    JConfDirective *d3 = j_conf_group_get_directive(g,
                                                    j_conf_directive_get_name
                                                    (d));
    if (d3 == NULL) {
        j_conf_group_append_node(g,
                                 j_conf_node_new
                                 (J_CONF_NODE_TYPE_DIRECTIVE, d));
    } else {
        j_conf_directive_merge(d3, d);
    }
}

static gint j_conf_group_compare_group(gconstpointer a, gconstpointer b)
{
    const JConfNode *node = (const JConfNode *) a;
    const gchar *name = (const gchar *) b;
    if (j_conf_node_is_group(node)) {
        return g_strcmp0(name,
                         j_conf_group_get_name(j_conf_node_get_group
                                               (node)));
    }
    return -1;
}

void j_conf_group_append_group(JConfGroup * g, JConfGroup * g2)
{
    JConfGroup *g3 = j_conf_group_get_group(g, j_conf_group_get_name(g2));
    if (g3 == NULL) {
        j_conf_group_append_node(g,
                                 j_conf_node_new(J_CONF_NODE_TYPE_GROUP,
                                                 g2));
    } else {
        j_conf_group_merge(g3, g2);
    }
}

void j_conf_group_free(JConfGroup * g)
{
    g_free(g->name);
    g_list_free_full(g->nodes, (GDestroyNotify) j_conf_node_free);
    g_slice_free1(sizeof(JConfGroup), g);
}


static inline void j_conf_group_merge(JConfGroup * g1, JConfGroup * g2)
{
    if (g_strcmp0(j_conf_group_get_name(g1), j_conf_group_get_name(g2)) !=
        0) {
        return;
    }
    GList *n2s = j_conf_group_get_nodes(g2);
    while (n2s) {
        JConfNode *n2 = (JConfNode *) n2s->data;
        if (j_conf_node_is_group(n2)) {
            JConfGroup *g = j_conf_node_get_group(n2);
            j_conf_group_append_group(g1, g);
        } else {
            JConfDirective *d = j_conf_node_get_directive(n2);
            j_conf_group_append_directive(g1, d);
        }
        n2s = g_list_next(n2s);
    }
    g_list_free(j_conf_group_get_nodes(g2));
    g2->nodes = NULL;
    j_conf_group_free(g2);
}

JConfDirective *j_conf_group_get_directive(JConfGroup * g,
                                           const gchar * name)
{
    GList *ele = g_list_find_custom(j_conf_group_get_nodes(g),
                                    name,
                                    (GCompareFunc)
                                    j_conf_group_compare_directive);
    if (ele) {
        JConfNode *node = (JConfNode *) ele->data;
        return (JConfDirective *) j_conf_node_get_directive(node);
    }
    return NULL;
}

const gchar *j_conf_group_get_directive_value(JConfGroup * g,
                                              const gchar * name)
{
    JConfDirective *d = j_conf_group_get_directive(g, name);
    if (d == NULL) {
        return NULL;
    }
    return j_conf_directive_get_value(d);
}

gint32 j_conf_group_get_directive_integer(JConfGroup * g,
                                          const gchar * name)
{
    const gchar *value = j_conf_group_get_directive_value(g, name);
    if (value) {
        return atoi(value);
    }
    return 0;
}

JConfGroup *j_conf_group_get_group(JConfGroup * g, const gchar * name)
{
    GList *ele = g_list_find_custom(j_conf_group_get_nodes(g),
                                    name,
                                    (GCompareFunc)
                                    j_conf_group_compare_group);
    if (ele) {
        JConfNode *node = (JConfNode *) ele->data;
        return (JConfGroup *) j_conf_node_get_group(node);
    }
    return NULL;
}



JConfNode *j_conf_node_new(JConfNodeType type, gpointer data)
{
    JConfNode *n = (JConfNode *) g_slice_alloc(sizeof(JConfNode));
    n->type = type;
    if (n->type == J_CONF_NODE_TYPE_DIRECTIVE) {
        n->data.directive = data;
    } else {
        n->data.group = data;
    }
    return n;
}

void j_conf_node_free(JConfNode * n)
{
    if (n->type == J_CONF_NODE_TYPE_DIRECTIVE) {
        j_conf_directive_free(n->data.directive);
    } else {
        j_conf_group_free(n->data.group);
    }
    g_slice_free1(sizeof(JConfNode), n);
}

static inline void j_conf_node_merge(JConfNode * n1, JConfNode * n2)
{
    if (j_conf_node_get_type(n1) != j_conf_node_get_type(n2)) {
        return;
    }
    if (j_conf_node_get_type(n1) == J_CONF_NODE_TYPE_DIRECTIVE) {
        j_conf_directive_merge(j_conf_node_get_directive(n1),
                               j_conf_node_get_directive(n2));
    } else {
        j_conf_group_merge(j_conf_node_get_group(n1),
                           j_conf_node_get_group(n2));
    }
}

JConfParser *j_conf_parser_new(const gchar * name)
{
    JConfParser *p = (JConfParser *) g_slice_alloc(sizeof(JConfParser));
    p->name = g_strdup(name);
    p->root = j_conf_group_new(NULL);
    return p;
}

void j_conf_parser_free(JConfParser * parser)
{
    g_free(parser->name);
    j_conf_group_free(parser->root);
    g_slice_free1(sizeof(JConfParser), parser);
}
