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


#include <sys/uio.h>
#include <netinet/in.h>
#include <glib.h>
#include <jconf.h>


/* a client request */
typedef struct {
    GByteArray *request;
    GByteArray *response;

    struct sockaddr_storage addr;
    socklen_t addrlen;
} JaRequest;

JaRequest *ja_request_new(const void *data, guint len,
                          struct sockaddr *addr, socklen_t addrlen);

void ja_request_free(JaRequest * req);

#define ja_request_data(req) (req)->request->data
#define ja_request_data_length(req)  (req)->request->len

#define ja_response_data(req)    (req)->response->data
#define ja_response_data_length(req) (req)->response->len

#define ja_response_append(req,data,len)  g_byte_array_append( (req)->response, (data), (len) )
#define ja_response_set(req,data,len)    do{g_byte_array_set_size ((req)->response, 0);ja_response_append(req,(data),(len));}while(0)


/*******************************************************************/
typedef void (*JaModuleConfigurationHandler) (GList * global_directives,
                                              GList * directives);

/* Returns the module configuration structure */
typedef void (*JaModuleInit) (void);
/* Registers hooks */
typedef void (*JaModuleHooksInit) (void);


typedef struct {
    gchar *name;

    JaModuleInit init_func;
    JaModuleConfigurationHandler cfg_handler;
    JaModuleHooksInit hooks_init_func;
} JaModule;


#endif
