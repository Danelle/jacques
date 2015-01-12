/*
 * config.h
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
#ifndef __JA_CONFIG_H__
#define __JA_CONFIG_H__

#include <glib.h>

/* absulote configuration directory */
#ifndef CONFIG_LOCATION
#define CONFIG_LOCATION "/etc/jacques"
#endif

/* configuration filename */
#ifndef CONFIG_FILENAME
#define CONFIG_FILENAME "jacques.conf"
#endif

/* absulote configuration file path */
#define CONFIG_FILEPATH CONFIG_LOCATION "/" CONFIG_FILENAME


/* the default log location */
#ifndef DEFAULT_LOG_LOCATION
#define DEFAULT_LOG_LOCATION "/var/log/jacques"
#endif

/* default user & group */
#ifndef DEFAULT_USER
#define DEFAULT_USER "jacques-app"
#endif

#ifndef DEFAULT_GROUP
#define DEFAULT_GROUP "jacques-app"
#endif

/* default listen() backlog */
#ifndef DEFAULT_BACKLOG
#define DEFAULT_BACKLOG 512
#endif



typedef struct {
    gchar *name;                /* directive name */
    gchar *args;                /* directive arguments */
} JaDirective;


typedef struct {
    gchar *name;                /* directive group name */
    GHashTable *directives;
} JaDirectiveGroup;

/* all directive structure */
typedef struct {
    GList *groups;
    /* reserverd */
} JaConfig;


/*
 * Parses configuration file CONFIG_FILEPATH
 * Returns NULL on error
 */
JaConfig *ja_config_load();

/*
 * Frees all the memory used by JaConfig
 */
void ja_config_free(JaConfig * jcfg);

#endif
