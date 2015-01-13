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
    jcfg->groups =
        g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                              (GDestroyNotify) ja_directive_group_free);
    return jcfg;
}

void ja_config_free(JaConfig * jcfg)
{
    g_hash_table_unref(jcfg->groups);
    g_slice_free1(sizeof(JaConfig), jcfg);
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
    g_free(jdg->name);
    g_hash_table_unref(jdg->directives);
    g_slice_free1(sizeof(JaDirectiveGroup), jdg);
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
    JaDirectiveGroup *group = ja_config_lookup(jcfg, gname);
    if (group == NULL) {
        group = ja_directive_group_new(gname);
        g_hash_table_insert(jcfg->groups, group->name, group);
    }
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
    JaDirective *jd = (JaDirective *) ja_directive_group_lookup(jdg, name);
    if (jd != NULL) {
        ja_directive_set_args(jd, args);
        return jd;
    }
    jd = ja_directive_new(name, args);
    g_hash_table_insert(jdg->directives, jd->name, jd);
    return jd;
}

/*
 * Looks up a JaDirectiveGroup with name
 */
JaDirectiveGroup *ja_config_lookup(JaConfig * jcfg, const gchar * name)
{
    JaDirectiveGroup *jdg =
        (JaDirectiveGroup *) g_hash_table_lookup(jcfg->groups, name);
    return jdg;
}

/*
 * Looks up a JaDirective with name
 */
JaDirective *ja_directive_group_lookup(JaDirectiveGroup * jdg,
                                       const gchar * name)
{
    JaDirective *jd =
        (JaDirective *) g_hash_table_lookup(jdg->directives, name);
    return jd;
}

/*
 * Calls the given function in every JaDirective in JaDirectiveGroup
 */
gint ja_directive_group_foreach(JaDirectiveGroup * jdg, JaHFunc func,
                                void *user_data)
{
    GList *keys = g_hash_table_get_keys(jdg->directives);
    GList *ptr = keys;
    while (ptr) {
        JaDirective *jd =
            (JaDirective *) g_hash_table_lookup(jdg->directives,
                                                (void *) ptr->data);
        if (!func(jd, user_data)) {
            g_list_free(keys);
            return 0;
        }
        ptr = g_list_next(ptr);
    }
    g_list_free(keys);
    return 1;
}
