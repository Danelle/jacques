/*
 * config.c
 * Copyright (C) 2015 Wiky L <wiiiky@yeah.net>
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

#include "config.h"
#include "ge/ge.h"

/**********************************/
static inline JaConfig *ja_config_new();
static inline JaDirectiveGroup *ja_config_add_group(JaConfig * jcfg,
                                                    const gchar * gname);

static inline JaDirectiveGroup *ja_directive_group_new(const gchar * name);
static inline void ja_directive_group_free(JaDirectiveGroup * jdg);
static inline JaDirective *ja_directive_group_add(JaDirectiveGroup * jdg,
                                                  const gchar * name,
                                                  const gchar * args);

static inline JaDirective *ja_directive_new(const gchar * name,
                                            const gchar * value);
static inline void ja_directive_free(JaDirective * jd);
static inline void ja_directive_set_args(JaDirective * jd,
                                         const gchar * args);


/*
 * Parses configuration file CONFIG_FILEPATH
 * Returns NULL on error
 */
JaConfig *ja_config_load()
{
    /* TODO */
    GKeyFile *kf = g_key_file_new();
    if (g_key_file_load_from_file
        (kf, CONFIG_FILEPATH, G_KEY_FILE_NONE, NULL) == FALSE) {
        g_key_file_free(kf);
        return NULL;
    }
    JaConfig *cfg = ja_config_new();

    gchar **groups = g_key_file_get_groups(kf, NULL);
    gchar *group;
    gint i = 0;
    while ((group = groups[i]) != NULL) {
        gchar **keys = g_key_file_get_keys(kf, group, NULL, NULL);
        gchar *key;
        gint j = 0;
        JaDirectiveGroup *jdg = ja_config_add_group(cfg, group);
        while ((key = keys[j]) != NULL) {
            gchar *value = g_key_file_get_value(kf, group, key, NULL);
            ja_directive_group_add(jdg, key, value);
            g_free(value);
            j++;
        }
        g_strfreev(keys);
        i++;
    }
    g_strfreev(groups);
    g_key_file_free(kf);


    return cfg;
}


static inline JaConfig *ja_config_new()
{
    JaConfig *jcfg = (JaConfig *) g_slice_alloc(sizeof(JaConfig));
    jcfg->groups = NULL;
    return jcfg;
}

static inline JaDirectiveGroup *ja_directive_group_new(const gchar * name)
{
    JaDirectiveGroup *group =
        (JaDirectiveGroup *) g_slice_alloc(sizeof(JaDirectiveGroup));
    group->name = g_strdup(name);
    group->directives =
        g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                              (GDestroyNotify) ja_directive_free);

    return group;
}

static inline void ja_directive_group_free(JaDirectiveGroup * jdg)
{
}

static inline JaDirective *ja_directive_new(const gchar * name,
                                            const gchar * value)
{
    JaDirective *jd = (JaDirective *) g_slice_alloc(sizeof(JaDirective));
    jd->name = g_strdup(name);
    jd->args = g_strdup(value);
    return jd;
}

static inline void ja_directive_free(JaDirective * jd)
{
    g_free(jd->name);
    g_free(jd->args);
    g_slice_free1(sizeof(JaDirective), jd);
}

static inline void ja_directive_set_args(JaDirective * jd,
                                         const gchar * args)
{
    g_free(jd->args);
    jd->args = g_strdup(args);
}

/*
 * If group $gname already exists in the JConfig, it will be returned directly
 * Otherwise, a new group named $gname is created and returned
 */
static inline JaDirectiveGroup *ja_config_add_group(JaConfig * jcfg,
                                                    const gchar * gname)
{
    GList *ptr = jcfg->groups;
    while (ptr) {
        JaDirectiveGroup *group = (JaDirectiveGroup *) ptr->data;
        if (g_strcmp0(group->name, gname) == 0) {
            return group;
        }
        ptr = g_list_next(ptr);
    }
    JaDirectiveGroup *group = ja_directive_group_new(gname);
    jcfg->groups = g_list_append(jcfg->groups, group);
    return group;
}

/*
 * If directive with name already exists, it will be updated and returned directly
 * Otherwise, a new JaDirective is returned
 */
static inline JaDirective *ja_directive_group_add(JaDirectiveGroup * jdg,
                                                  const gchar * name,
                                                  const gchar * args)
{
    JaDirective *jd =
        (JaDirective *) g_hash_table_lookup(jdg->directives, name);
    if (jd != NULL) {
        ja_directive_set_args(jd, args);
        return jd;
    }
    jd = ja_directive_new(name, args);
    g_hash_table_insert(jdg->directives, jd->name, jd);
    return jd;
}
