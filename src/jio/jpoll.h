/*
 * jpoll.h
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

#ifndef __J_POLL_H__
#define __J_POLL_H__

#include "jsocket.h"


typedef struct _JPoll JPoll;


/*
 * Creates a JPoll
 * Returns NULL on error
 */
JPoll *j_poll_new();


/*
 * Registers the JSocket with event EPOLLIN
 * Returns 1 on success, otherwise 0
 */
int j_poll_addin(JPoll * jp, JSocket * jsock);

/*
 * Registers the JSocket with event EPOLLOUT
 * Returns 1 on success, otherwise 0
 */
int j_poll_addout(JPoll * jp, JSocket * jsock);


/*
 * Registers the JSocket with event EPOLLOUT|EPOLLIN
 * Returns 1 on success, otherwise 0
 */
int j_poll_addio(JPoll * jp, JSocket * jsock);

/*
 * Unregisters the JSocket
 * Returns 1 on success, otherwise 0
 */
int j_poll_delete(JPoll * jp, JSocket * jsock);



#endif                          /* __J_POLL_H__ */
