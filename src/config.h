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
#include <jac.h>


#define JACQUES_VERSION_MAJOR   0
#define JACQUES_VERSION_MINOR   1

#define JACQUES_VERSION 0.1

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


/* the directory stores app server configurations */
#define CONFIG_APP_LOCATION CONFIG_LOCATION "/app-enabled"


/* the directory stores active modules */
#define CONFIG_MOD_ENABLED_LOCATION CONFIG_LOCATION "/modules"


/* the default log location */
#ifndef DEFAULT_LOG_LOCATION
#define DEFAULT_LOG_LOCATION "/var/log/jacques"
#endif

/* default user & group */
#ifndef DEFAULT_USER
#define DEFAULT_USER "jacques-user"
#endif

/* default listen() backlog */
#ifndef DEFAULT_BACKLOG
#define DEFAULT_BACKLOG 512
#endif

/*
 * Parses configuration file CONFIG_FILEPATH
 * Returns NULL on error
 */
#define ja_config_load()    j_conf_parse(CONFIG_FILEPATH)


#define RUNTIME_LOCATION    "/var/run/jacques"





#define DIRECTIVE_LOADMODULE	"LoadModule"

/*
 * Loads modules based on configuration
 */
void ja_config_load_modules(JConfig * cfg);

/*
 * Loads a module
 * Returns 1 on success,otherwise 0
 */
int ja_load_module(const gchar * filename, JConfig * cfg);


#endif
