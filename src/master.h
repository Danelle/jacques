/*
 * master.h
 *
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
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
#ifndef __JA_CORE_H__
#define __JA_CORE_H__


/*
 * JaMaster, the controller of servers
 */

#include <glib.h>
#include "config.h"


typedef struct {
    JConfParser *cfg;
    GList *scfg;
    GList *children;
} JaMaster;


/*
 * Starts the core process of jacques
 * If fail, will call g_error() to terminate the process
 */
JaMaster *ja_master_create();


void ja_master_wait(JaMaster * core);


void ja_master_quit(JaMaster * core);




#endif                          /* __JA_CORE_H__ */
