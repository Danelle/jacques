/*
 * struct.h
 *
 * Copyright (C) 2015 - Wiky L  <wiiiky@yeah.net>
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
#ifndef __J_STRUCT_H__
#define __J_STRUCT_H__

#include <glib.h>


/* a client request */
typedef struct {
    gchar *addr;                /* client IP address */

    GByteArray *request;
    GByteArray *response;
} JaRequest;

JaRequest *ja_request_new(const void *data, guint len, const gchar * addr);

#define j_request_data(req) (req)->request->data
#define j_request_data_length(req)  (req)->request->len

#define j_response_data(req)    (req)->response->data
#define j_response_data_length(req) (req)->response->len

#define j_response_append(req,data,len)  g_byte_array_append( (req)->response, (data), (len) )
#define j_response_set(req,data,len)    do{g_byte_array_set_size ((req)->response, 0);j_response_append(req,(data),(len));}while(0)

typedef enum {
    J_RESPONSE = 0x1,           /* response client */
    J_IGNORE = 0x2,             /* ignore the request */

    J_DROP = 0x4,               /* drop the connection */
    J_KEEP = 0x8                /* keep the connection */
} JaResult;


typedef JaResult(*JaRequestHandler) (JaRequest * req);

typedef struct {
    JaRequestHandler req_handler;
} JHook;



/* the structures of a module */
typedef struct {
    gchar *name;
    JHook hooks;
} JaModule;


#endif
