/*
 * jconf.c
 *
 * Copyright (C) 2015 - Wiky L <wiiiky@yeah.net>
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
#include "jconf.h"
#include "../io/jio.h"
#include "../config.h"
#include <string.h>


static void j_directive_list_free(GList * list);


JDirective *j_directive_new(const gchar * name, const gchar * value)
{
    return j_directive_new_take(g_strdup(name), g_strdup(value));
}

JDirective *j_directive_new_take(gchar * name, gchar * value)
{
    JDirective *jd = (JDirective *) g_slice_alloc(sizeof(JDirective));
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

JDirectiveGroup *j_directive_group_new(const gchar * name)
{
    JDirectiveGroup *group =
        (JDirectiveGroup *) g_slice_alloc(sizeof(JDirectiveGroup));
    group->name = g_strdup(name);
    group->directives = NULL;
    return group;
}

void j_directive_group_insert(JDirectiveGroup * group,
                              const gchar * name, const gchar * value)
{
    group->directives =
        g_list_append(group->directives, j_directive_new(name, value));
}

void j_directive_group_insert_take(JDirectiveGroup * group,
                                   gchar * name, gchar * value)
{
    group->directives =
        g_list_append(group->directives,
                      j_directive_new_take(name, value));
}

JDirective *j_directive_group_lookup(JDirectiveGroup * group,
                                     const gchar * name)
{
    GList *ptr = group->directives;
    JDirective *ret = NULL;
    while (ptr) {
        JDirective *jd = (JDirective *) ptr->data;
        if (g_strcmp0(name, jd->name) == 0) {
            ret = jd;
        }
        ptr = g_list_next(ptr);
    }
    return ret;
}

gint j_directive_group_get_integer(JDirectiveGroup * group,
                                   const gchar * name)
{
    JDirective *jd = j_directive_group_lookup(group, name);
    if (jd) {
        return atoi(jd->value);
    }
    return -1;
}

const gchar *j_directive_group_get_string(JDirectiveGroup * group,
                                          const gchar * name)
{
    JDirective *jd = j_directive_group_lookup(group, name);
    if (jd) {
        return (const gchar *) jd->value;
    }
    return NULL;
}

void j_directive_group_free(JDirectiveGroup * group)
{
    g_free(group->name);
    g_list_free_full(group->directives, (GDestroyNotify) j_directive_free);
    g_slice_free1(sizeof(JDirectiveGroup), group);
}

JDirectiveGroup *j_config_lookup(JConfig * cfg, const gchar * name)
{
    if (name == NULL) {
        return (JDirectiveGroup *) g_hash_table_lookup(cfg->tb,
                                                       J_GLOBAL_CONFIG_GROUP);
    }
    JDirectiveGroup *group =
        (JDirectiveGroup *) g_hash_table_lookup(cfg->tb, name);
    return group;
}

/* 
 * group is optional NULl for global
 */
gint j_config_get_integer(JConfig * cfg, const gchar * gname,
                          const gchar * name)
{
    JDirectiveGroup *group = j_config_lookup(cfg, gname);
    if (group) {
        return j_directive_group_get_integer(group, name);
    }
    return -1;
}

const gchar *j_config_get_string(JConfig * cfg, const gchar * gname,
                                 const gchar * name)
{
    JDirectiveGroup *group = j_config_lookup(cfg, gname);
    if (group) {
        return j_directive_group_get_string(group, name);
    }
    return NULL;
}


static inline void j_config_insert_directive(JConfig * cfg,
                                             const gchar * gname,
                                             gchar * name, gchar * value)
{
    JDirectiveGroup *group =
        (JDirectiveGroup *) j_config_lookup(cfg, gname);
    if (group) {
        j_directive_group_insert_take(group, name, value);
    } else {
        group = j_directive_group_new(gname);
        j_directive_group_insert_take(group, name, value);
        g_hash_table_insert(cfg->tb, g_strdup(gname), group);
    }
}


#define str_remove_whitespace(str)  g_strchomp(g_strchug(str))

static void j_conf_parse_file(const gchar * file, JConfig * cfg)
{
    if (g_path_is_absolute(file)) {
        j_conf_parse_internal(file, cfg);
    } else {
        gchar buf[4096];
        g_snprintf(buf, sizeof(buf), "%s/%s", CONFIG_LOCATION, file);
        j_conf_parse_internal(buf, cfg);
    }
}

void j_conf_parse_internal(const gchar * filepath, JConfig * cfg)
{
    JFile *jf = j_file_open(filepath, "r");
    if (jf == NULL) {
        g_warning("fail to open conf file: %s", filepath);
        return;
    }
    gchar *line;
    guint linenumber = 0;
    gchar *group = NULL;
    while ((line = j_file_readline(jf)) != NULL) {
        linenumber++;
        gchar *pound = strchr(line, '#');
        if (pound) {
            *pound = '\0';
        }
        line = str_remove_whitespace(line);
        guint len = strlen(line);
        if (len == 0) {
            continue;
        }
        if (g_str_has_prefix(line, "</")) {
            if (!g_str_has_suffix(line, ">")) {
                g_warning("invalid syntax in %s:%d", filepath, linenumber);
            } else if (group != NULL) {
                /* end group */
                gchar *name = g_strndup(line + 2, len - 3);
                name = str_remove_whitespace(name);
                if (g_strcmp0(name, group) != 0) {
                    g_warning("group name not match in %s:%d", filepath,
                              linenumber);
                }
                g_free(name);
                g_free(group);
                group = NULL;
            } else {
                g_warning("unexpected group close! %s:%d", filepath,
                          linenumber);
            }
        } else if (g_str_has_prefix(line, "<")) {
            if (!g_str_has_suffix(line, ">")) {
                g_warning("invalid syntax in %s:%d", filepath, linenumber);
            } else if (group == NULL) {
                /* start group */
                group = g_strndup(line + 1, len - 2);
                group = str_remove_whitespace(group);
            } else {
                g_warning("group in group is not supported! %s:%d",
                          filepath, linenumber);
            }
        } else {
            gchar *white = strchr(line, ' ');
            gchar *name = NULL;
            gchar *value = NULL;
            if (white) {
                name = g_strndup(line, white - line);
                value = g_strdup(str_remove_whitespace(white + 1));
            } else {
                name = g_strdup(line);
            }
            if (g_strcmp0(name, J_GLOBAL_INCLUDE_CONF) == 0) {
                if (value == NULL) {
                    g_warning("%s: conf file not specified",
                              J_GLOBAL_INCLUDE_CONF);
                } else {
                    j_conf_parse_file(value, cfg);
                }
                g_free(name);
                g_free(value);
            } else if (group) { /* this directive is in a group */
                j_config_insert_directive(cfg, group, name, value);
            } else {
                j_config_insert_directive(cfg, J_GLOBAL_CONFIG_GROUP, name,
                                          value);
            }
        }
        g_free(line);
    }
    j_file_close(jf);

    if (group) {
        g_warning("group %s not closed! %s", group);
        g_free(group);
    }
}

/*
 * Returns a GHashTable, which has conf group as key, a list of JDirective as value
 */
JConfig *j_conf_parse(const gchar * filepath)
{
    JConfig *cfg = j_config_new(filepath);

    j_conf_parse_internal(filepath, cfg);

    return cfg;
}


JConfig *j_config_new(const gchar * name)
{
    JConfig *cfg = (JConfig *) g_slice_alloc(sizeof(JConfig));
    cfg->tb = g_hash_table_new_full(g_str_hash, g_str_equal,
                                    (GDestroyNotify) g_free,
                                    (GDestroyNotify)
                                    j_directive_group_free);
    g_hash_table_insert(cfg->tb, g_strdup(J_GLOBAL_CONFIG_GROUP),
                        j_directive_group_new(J_GLOBAL_CONFIG_GROUP));
    cfg->name = g_strdup(name);
    return cfg;
}

void j_config_free(JConfig * cfg)
{
    if (cfg == NULL) {
        return;
    }
    g_free(cfg);
    g_hash_table_unref(cfg->tb);
    g_slice_free1(sizeof(JConfig), cfg);
}
