/*
 * mod.h
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

#ifndef __JA_MOD_H__
#define __JA_MOD_H__


#include "hooks.h"
#include <gmodule.h>


void ja_module_register(JaModule * mod);


GList *ja_get_modules();


/* Functions to get all different hook lists */
GList *ja_get_request_hooks(void);
GList *ja_get_server_quit_hooks(void);


/* */
#define JA_MODULE_EXPORT G_MODULE_EXPORT


/* Register a hook */
void ja_hook_register(void *ptr, JaHookType type);


#endif
