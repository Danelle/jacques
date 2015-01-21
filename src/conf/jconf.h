/*
 * jconf.h
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
#ifndef __J_CONF_H__
#define __J_CONF_H__

#include <glib.h>
#include <stdlib.h>


typedef struct {
    gchar *name;
    gchar *value;
} JDirective;

#define j_directive_get_integer(jd)  atoi((jd)->value)
#define j_directive_get_string(jd)  ((jd)->value)


JDirective *j_directive_new(const gchar * name, const gchar * value);
JDirective *j_directive_new_take(gchar * name, gchar * value);
void j_directive_free(JDirective * jd);


typedef struct {
    gchar *name;
    GList *directives;
} JDirectiveGroup;

JDirectiveGroup *j_directive_group_new(const gchar * name);
void j_directive_group_insert(JDirectiveGroup * group,
                              const gchar * name, const gchar * value);
void j_directive_group_insert_take(JDirectiveGroup * group,
                                   gchar * name, gchar * value);
void j_directive_group_free(JDirectiveGroup * group);

JDirective *j_directive_group_lookup(JDirectiveGroup * group,
                                     const gchar * name);
gint j_directive_group_get_integer(JDirectiveGroup * group,
                                   const gchar * name);
const gchar *j_directive_group_get_string(JDirectiveGroup * group,
                                          const gchar * name);


#define J_GLOBAL_CONFIG_GROUP   "Global"
#define J_GLOBAL_INCLUDE_CONF   "IncludeConf"


typedef struct {
    GHashTable *tb;
    gchar *name;
} JConfig;

JConfig *j_config_new(const gchar * name);
void j_config_free(JConfig * cfg);


JDirectiveGroup *j_config_lookup(JConfig * cfg, const gchar * name);

/* 
 * group is optional NULl for global
 */
gint j_config_get_integer(JConfig * cfg, const gchar * group,
                          const gchar * name);
const gchar *j_config_get_string(JConfig * cfg, const gchar * group,
                                 const gchar * name);


void j_conf_parse_internal(const gchar * filepath, JConfig * cfg);
/*
 * Returns a GHashTable, which has conf group as key, a list of JDirective as value
 */
JConfig *j_conf_parse(const gchar * filepath);

#endif
