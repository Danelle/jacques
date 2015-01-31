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

/*
 * All hooks
 */
typedef enum {
    JA_HOOK_TYPE_REQUEST,
    JA_HOOK_TYPE_SERVER_QUIT,
} JaHookType;


typedef enum {
    JA_ACTION_IGNORE = 0x00,
    JA_ACTION_RESPONSE = 0x01,
    JA_ACTION_DROP = 0x02,      /* drop the connection */
    JA_ACTION_KEEP = 0x04,      /* keep the connection */
} JaAction;

/* handle request hook */
typedef JaAction(*JaRequestHandler) (JaRequest * req);


/*
 * @param name: the server's name
 * @param listen_port: the port that server listens on
 */
typedef void (*JaServerQuitHandler) (const gchar * name,
                                     gushort listen_port);


#endif                          /* __JA_HOOKS_H__ */
