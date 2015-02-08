/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * lib.h
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
 * 
 * libjconf is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libjconf is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __J_CONF_H___
#define __J_CONF_H___

#include "struct.h"


#define INCLUDE_CONF    "IncludeConf"

/*
 * Parses a file and returns a JParser on success
 * Returns NULL on error
 */
JParser *j_conf_parse(const gchar * path, GError ** error);


/*
 * Parses a file with an existing JParser.
 */
gboolean j_conf_parse_more(JParser * p, const gchar * path,
                           GError ** error);


#endif                          /* __J_CONF_H___ */
