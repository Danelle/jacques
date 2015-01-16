/*
 * hooks.c
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
#include "hooks.h"



JaHook *ja_hook_new(void *ptr, JaHookType type)
{
    JaHook *hook = (JaHook *) g_slice_alloc(sizeof(JaHook));
    hook->ptr = ptr;
    hook->type = type;
    return hook;
}

void ja_hook_free(JaHook * hook)
{
    g_slice_free1(sizeof(JaHook), hook);
}
