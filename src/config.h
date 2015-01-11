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

/* the configuration structure */
typedef struct {
    gchar *user;                /* 以什么用户运行 */
    gchar *group;               /* 以什么组运行 */
    gchar *log_location;        /* 日志文件的存储位置 */
    guint32 backlog;            /* listen()的第二个参数，连接等待队列的长度 */
} JaConfig;


/*
 * Parses configuration file CONFIG_FILEPATH
 * Returns NULL on error
 */
JaConfig *ja_config_load();

#endif
