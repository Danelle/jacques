/*
 * log.h
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
#ifndef __JA_LOG_H__
#define __JA_LOG_H__

#include <glib.h>

/*
 * Initializes log handlers
 * Returns 1 on success
 * Returns 0 on error
 */
gboolean initialize_default_log(void);


/*
 * Re-set log handlers using specified files
 * @param normal, the file that logs normal message
 * @param error, the file that logs error message
 */
gboolean set_custom_log(const gchar * normal, const gchar * error);



#endif                          /* __JA_LOG_H__ */
