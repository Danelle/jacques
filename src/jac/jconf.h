/*
 * jconf.h
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
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

#ifndef __J_CONF_H__
#define __J_CONF_H__


#include "jconfstruct.h"

/*
 * Parses a file,
 * Returns a JConfParser on success
 * Otherwise NULL
 */
JConfParser *j_conf_parse(const gchar * filepath);


gboolean j_conf_parser_parse(JConfParser * p, const gchar * filepath);

#endif                          /* __J_CONF_H__ */
