/*
 * pack.c
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

#include "pack.h"
#include <stdlib.h>


/*
 * Converts integer to 4-bytes array
 * Memory returned is dynamically allocated
 */
char *pack_length4(unsigned int length)
{
    char *bytes = (char *) malloc(sizeof(char) * 4);
    bytes[0] = length % 0x100;
    bytes[1] = length % 0x10000 / 0x100;
    bytes[2] = length % 0x1000000 / 0x10000;
    bytes[3] = length / 0x1000000;

    return bytes;
}
