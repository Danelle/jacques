/*
 * pack.h
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

#ifndef __J_PACK_H__
#define __J_PACK_H__

#include <glib.h>

/*
 * functions for packing data
 */


/*
 * Converts integer to 4-bytes array
 * Memory returned is dynamically allocated
 * @param length can't be greater than 2^32
 */
gchar *pack_length4(guint32 length);

/*
 * Converts 4-bytes array to integer
 */
guint32 unpack_length4(gchar * bytes);


#endif                          /* __J_PACK_H__ */
