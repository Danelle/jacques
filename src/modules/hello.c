/*
 * hello.c
 *
 * Copyright (C) 2015 - Wiky L
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
#include <jac.h>


static gint request(JaRequest * req)
{
    ja_response_append(req, "hello all", 9);
    return JA_ACTION_RESPONSE;
}

static void init()
{
    g_message("hello all");
}


static void hook_register()
{
    ja_hook_register(request, JA_HOOK_TYPE_REQUEST);
}

JA_MODULE_EXPORT JaModule hello_struct = {
    "hello all",
    init,
    hook_register,
};
