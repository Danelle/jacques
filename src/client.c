/*
 * client.c
 *
 * Copyright (C) 2015 - Wiky L <wiiiky@yeah.net>
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

#include "jio/jio.h"
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>

#include <errno.h>
int main(int argc, const char *argv[])
{
    const gchar *remote = "127.0.0.1";
    gushort port = 0;
    if (argc == 3) {
        remote = argv[1];
        port = (gushort) atoi(argv[2]);
    } else if (argc == 2) {
        port = (gushort) atoi(argv[1]);
    } else {
        return 0;
    }

    JSocket *jsock = j_client_socket_new(remote, port);
    if (jsock == NULL) {
        g_message("fail to connect to server");
        return -1;
    }

    while (j_socket_write(jsock, (const void *) "hello world", 12) == 0) {
    }

    sleep(1);

    while (j_socket_read(jsock) == 0) {
    }

    /*g_message ("errno: %d",errno); */

    const gchar *data = (const gchar *) j_socket_data(jsock);
    g_message("%s", data);

    j_socket_close(jsock);

    return 0;
}
