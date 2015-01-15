/*
 * mod.h
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

#ifndef __JA_MOD_H__
#define __JA_MOD_H__


#include "mod-pub.h"


GList *ja_get_modules();


/*
 * Loads a module
 * Returns 1 on success,otherwise 0
 */
int ja_load_module(const gchar * filename);


/*
 * Loads all modules under CONFIG_MOD_ENABLED_LOCATION
 */
void ja_load_all_modules();

#endif
