/*
 * struct.c
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
#include "struct.h"
#include <string.h>


JaRequest *ja_request_new(const void *data, guint len,
                          struct sockaddr *addr, socklen_t addrlen)
{
    JaRequest *req = (JaRequest *) g_slice_alloc(sizeof(JaRequest));
    req->request = g_byte_array_new();
    g_byte_array_append(req->request, data, len);

    req->response = g_byte_array_new();

    if (addr) {
        memcpy(&req->addr, addr, addrlen);
        req->addrlen = addrlen;
    } else {
        req->addrlen = 0;
        memset(&req->addr, 0, sizeof(struct sockaddr_storage));
    }
    return req;
}

void ja_request_free(JaRequest * req)
{
    g_byte_array_free(req->request, TRUE);
    g_byte_array_free(req->response, TRUE);
    g_slice_free1(sizeof(JaRequest), req);
}
