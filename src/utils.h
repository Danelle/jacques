/*
 * utils.h
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
#ifndef __JA_UTILS_H__
#define __JA_UTILS_H__


/*
 * Daemonizes current process
 * Returns pid in parent
 * Returns 0 in child
 * Returns -1 on error
 */
int daemonize(void);



/*
 * Closes all open file descriptor
 */
int close_fds(void);


/*
 * Check if the process is already running
 * Returns 1 if yes
 * Returns 0 if no
 * Returns -1 on error
 */
int already_running(void);


#endif
