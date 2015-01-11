/*
 * geslice.c
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

#include "geslice.h"
#include <string.h>

/*
 * Copys a string using slice allocator
 */
gchar *ge_slice_strdup(const gchar * name)
{
    if (name == NULL) {
        return NULL;
    }
    guint32 size = strlen(name) + 1;
    gchar *copy = (gchar *) g_slice_alloc(size);
    strncpy(copy, name, size);
    return copy;
}
