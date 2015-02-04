/*
 * config.h
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

/* default user & group */
#ifndef CONFIG_USER
#define CONFIG_USER "jacques-user"
#endif


#ifndef CONFIG_RUNTIME_LOCATION
#define CONFIG_RUNTIME_LOCATION    "/var/run/jacques"
#endif


#ifndef CONFIG_PID_NAME
#define CONFIG_PID_NAME    "jacques.pid"
#endif


/* the file to save daemon's process id */
#define CONFIG_PID_FILE    CONFIG_RUNTIME_LOCATION "/" CONFIG_PID_NAME

/* the default log location */
#ifndef CONFIG_LOG_LOCATION
#define CONFIG_LOG_LOCATION "/var/log/jacques"
#endif


#ifndef CONFIG_LOG_NAME
#define CONFIG_LOG_NAME "normal.log"
#endif

#ifndef CONFIG_LOG_ERROR_NAME
#define CONFIG_LOG_ERROR_NAME "error.log"
#endif


#define CONFIG_LOG_FILE CONFIG_LOG_LOCATION "/" CONFIG_LOG_NAME
#define CONFIG_LOG_ERROR_FILE CONFIG_LOG_LOCATION "/" CONFIG_LOG_ERROR_NAME



/*
 * Parses configuration file CONFIG_FILEPATH
 * Returns NULL on error
 */
JConfParser *ja_config_load(void);


#define DIRECTIVE_LOADMODULE    "LoadModule"

/*
 * Loads modules based on configuration
 */
void ja_config_load_modules(JConfParser * cfg);

/*
 * Loads a module
 * Returns TRUE on success,otherwise FALSE
 */
gboolean ja_load_module(const gchar * filename);


#endif
