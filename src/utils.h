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

#include <glib.h>

/*
 * Daemonizes current process
 * Returns TRUE on success
 * Returns FALSE on error
 */
gboolean daemonize(void);



/*
 * Closes all open file descriptor
 */
gboolean close_fds(void);


/*
 * Check if the process is already running
 * Returns 1 if yes
 * Returns 0 if no
 * Returns -1 on error
 */
gint already_running(void);


/*
 * Sets the current process effective user as name
 * Returns TRUE on success
 * FALSE otherwise
 */
gboolean setuser(const gchar * name);


/*
 * Opens a file for writing
 * Returns NULL on error
 */
gint open_appendable(const gchar * name);


/*
 * Sets the process title
 * http://www.codecodex.com/wiki/Set_Process_Title
 */
void set_proctitle(gchar ** argv,
                   /* argv as passed to main, so args can be moved if necessary */
                   const gchar * fmt,   /* printf(3)-style format string for process title */
                   ... /* args to format string */ );


#endif
