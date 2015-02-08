/*
 * struct.c
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

#include "private.h"


gint j_group_compare(gconstpointer a, gconstpointer b);


/********************************Directive**********************************/

struct _JDirective {
    gchar *name;
    gchar *value;
};

const gchar *j_directive_get_name(JDirective * jd)
{
    return jd->name;
}

const gchar *j_directive_get_value(JDirective * jd)
{
    return jd->value;
}

JDirective *j_directive_alloc(const gchar * name, const gchar * value)
{
    return j_directive_alloc_take(g_strdup(name), g_strdup(value));
}

JDirective *j_directive_alloc_take(gchar * name, gchar * value)
{
    JDirective *jd = (JDirective *) g_slice_alloc(sizeof(JDirective));
    if (name) {
        name = g_strstrip(name);
    }
    if (value) {
        value = g_strstrip(value);
    }
    jd->name = name;
    jd->value = value;
    return jd;
}

void j_directive_free(JDirective * jd)
{
    g_free(jd->name);
    g_free(jd->value);
    g_slice_free1(sizeof(JDirective), jd);
}


/**********************************Group************************************/

struct _JGroup {
    gchar *name;
    gchar *value;
    GList *nodes;
};

/* joins two groups */
static void inline j_group_join(JGroup * g1, JGroup * g2)
{
    GList *ptr = g2->nodes;
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        j_group_append_node(g1, n);
        ptr = g_list_next(ptr);
    }
    g_list_free(g2->nodes);
    g2->nodes = NULL;
}

const gchar *j_group_get_name(JGroup * jg)
{
    return jg->name;
}

const gchar *j_group_get_value(JGroup * jg)
{
    return jg->value;
}

GList *j_group_get_nodes(JGroup * jg)
{
    return jg->nodes;
}

JNode *j_group_append_node(JGroup * g, JNode * n)
{
    if (j_node_is_group(n)) {
        GList *ele = g_list_find_custom(j_group_get_nodes(g),
                                        j_node_get_group(n),
                                        (GCompareFunc)
                                        j_group_compare);
        if (ele) {
            JNode *n2 = (JNode *) ele->data;
            j_group_join(j_node_get_group(n2), j_node_get_group(n));
            j_node_free(n);
            return n2;
        }
    }
    g->nodes = g_list_append(g->nodes, n);
    return n;
}

JGroup *j_group_append_group(JGroup * g, JGroup * g2)
{
    JNode *n = j_group_append_node(g,
                                   j_node_alloc(J_NODE_TYPE_GROUP,
                                                g2));
    return j_node_get_group(n);
}

JDirective *j_group_append_directive(JGroup * g, JDirective * d)
{
    JNode *n = j_group_append_node(g,
                                   j_node_alloc(J_NODE_TYPE_DIRECTIVE,
                                                d));
    return j_node_get_directive(n);
}

JGroup *j_group_alloc(const gchar * name, const gchar * value)
{
    return j_group_alloc_take(g_strdup(name), g_strdup(value));
}

JGroup *j_group_alloc_take(gchar * name, gchar * value)
{
    JGroup *g = (JGroup *) g_slice_alloc(sizeof(JGroup));
    if (name) {
        name = g_strstrip(name);
    }
    if (value) {
        value = g_strstrip(value);
    }
    g->name = name;
    g->value = value;
    g->nodes = NULL;
    return g;
}

void j_group_free(JGroup * jg)
{
    g_free(jg->name);
    g_free(jg->value);
    g_list_free_full(jg->nodes, (GDestroyNotify) j_node_free);
    g_slice_free1(sizeof(JGroup), jg);
}

/********************************Node***************************************/

struct _JNode {
    JNodeType type;
    gpointer data;
};

const gchar *j_node_get_name(JNode * n)
{
    if (j_node_is_group(n)) {
        return j_group_get_name(j_node_get_group(n));
    }
    return j_directive_get_name(j_node_get_directive(n));
}

JNodeType j_node_get_type(JNode * n)
{
    return n->type;
}

JDirective *j_node_get_directive(JNode * n)
{
    return (JDirective *) n->data;
}

JGroup *j_node_get_group(JNode * n)
{
    return (JGroup *) n->data;
}

JNode *j_node_alloc(JNodeType type, gpointer data)
{
    JNode *n = (JNode *) g_slice_alloc(sizeof(JNode));
    n->type = type;
    n->data = data;
    return n;
}

void j_node_free(JNode * n)
{
    if (j_node_is_group(n)) {
        j_group_free(j_node_get_group(n));
    } else {
        j_directive_free(j_node_get_directive(n));
    }
    g_slice_free1(sizeof(JNode), n);
}

/*******************************Parser *************************************/

struct _JParser {
    gchar *name;                /* filename */
    GList *nodes;               /* nodes */
};

JParser *j_parser_alloc(const gchar * name)
{
    JParser *p = (JParser *) g_slice_alloc(sizeof(JParser));
    p->name = g_strdup(name);
    p->nodes = NULL;
    return p;
}

void j_node_freee(JNode * n)
{
    if (n->type == J_NODE_TYPE_DIRECTIVE) {
        j_directive_free(j_node_get_directive(n));
    } else {
        j_group_free(j_node_get_group(n));
    }
    g_slice_free1(sizeof(JNode), n);
}

const gchar *j_parser_get_name(JParser * p)
{
    return p->name;
}

GList *j_parser_get_root(JParser * p)
{
    return p->nodes;
}

JNode *j_parser_append_node(JParser * p, JNode * n)
{
    if (j_node_is_group(n)) {
        GList *ele = g_list_find_custom(j_parser_get_root(p),
                                        j_node_get_group(n),
                                        (GCompareFunc)
                                        j_group_compare);
        if (ele) {
            JNode *n2 = (JNode *) ele->data;
            j_group_join(j_node_get_group(n2), j_node_get_group(n));
            j_node_free(n);
            return n2;
        }
    }
    p->nodes = g_list_append(p->nodes, n);
    return n;
}

JDirective *j_parser_append_directive(JParser * p, JDirective * jd)
{
    JNode *n = j_parser_append_node(p,
                                    j_node_alloc(J_NODE_TYPE_DIRECTIVE,
                                                 jd));
    return j_node_get_directive(n);
}

JGroup *j_parser_append_group(JParser * p, JGroup * jg)
{
    JNode *n = j_parser_append_node(p,
                                    j_node_alloc(J_NODE_TYPE_GROUP,
                                                 jg));
    return j_node_get_group(n);
}

void j_parser_free(JParser * p)
{
    g_free(p->name);
    g_list_free_full(j_parser_get_root(p), (GDestroyNotify) j_node_free);
    g_slice_free1(sizeof(JParser), p);
}

GList *j_parser_get_directive(JParser * p, const gchar * name)
{
    GList *ret = NULL;
    GList *ptr = j_parser_get_root(p);
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        if (j_node_is_directive(n)) {
            JDirective *d = j_node_get_directive(n);
            if (g_strcmp0(name, j_directive_get_name(d)) == 0) {
                ret = g_list_append(ret, d);
            }
        }
        ptr = g_list_next(ptr);
    }
    return ret;
}

GList *j_parser_get_group(JParser * p, const gchar * name)
{
    GList *ret = NULL;
    GList *ptr = j_parser_get_root(p);
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        if (j_node_is_group(n)) {
            JGroup *g = j_node_get_group(n);
            if (g_strcmp0(name, j_group_get_name(g)) == 0) {
                ret = g_list_append(ret, g);
            }
        }
        ptr = g_list_next(ptr);
    }
    return ret;
}

JDirective *j_parser_get_directive_last(JParser * p, const gchar * name)
{
    JDirective *ret = NULL;
    GList *ptr = j_parser_get_root(p);
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        if (j_node_is_directive(n)) {
            JDirective *d = j_node_get_directive(n);
            if (g_strcmp0(name, j_directive_get_name(d)) == 0) {
                ret = d;
            }
        }
        ptr = g_list_next(ptr);
    }
    return ret;
}

JGroup *j_parser_get_group_last(JParser * p, const gchar * name)
{
    JGroup *ret = NULL;
    GList *ptr = j_parser_get_root(p);
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        if (j_node_is_group(n)) {
            JGroup *g = j_node_get_group(n);
            if (g_strcmp0(name, j_group_get_name(g)) == 0) {
                ret = g;
            }
        }
        ptr = g_list_next(ptr);
    }
    return ret;
}


gint j_group_compare(gconstpointer a, gconstpointer b)
{
    JNode *n = (JNode *) a;
    JGroup *g1 = (JGroup *) b;
    if (j_node_is_group(n)) {
        JGroup *g2 = j_node_get_group(n);
        const gchar *v1 = j_group_get_value(g1);
        const gchar *v2 = j_group_get_value(g2);
        const gchar *n1 = j_group_get_name(g1);
        const gchar *n2 = j_group_get_name(g2);
        return g_strcmp0(v1, v2) + g_strcmp0(n1, n2);
    }
    return -1;
}
