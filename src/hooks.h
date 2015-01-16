/*
 * hooks.h
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
#ifndef __JA_HOOKS_H__
#define __JA_HOOKS_H__

#include "struct.h"

typedef enum {
    JA_ACTION_IGNORE = 0x00,
    JA_ACTION_ECHO = 0x01,
    JA_ACTION_RESPONSE = 0x02,
    JA_ACTION_DROP = 0x04,      /* drop the connection */
    JA_ACTION_KEEP = 0x08,      /* keep the connection */
} JaAction;


typedef JaAction(*JaRequestHandler) (JaRequest * req);



typedef enum {
    JA_HOOK_TYPE_REQUEST,
} JaHookType;


typedef struct {
    JaHookType type;
    void *ptr;
} JaHook;


JaHook *ja_hook_new(void *ptr, JaHookType type);
void ja_hook_free(JaHook * hook);


#endif                          /* __JA_HOOKS_H__ */
