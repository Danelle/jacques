/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
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

#include "core.h"
#include "utils.h"

int main(int argc, const char *argv[])
{
    int running = already_running();
    if (running == 1) {
        g_error("jacqueas is already running!!!");
    } else if (running < 0) {
        g_error("fail to open pid file!!!");
    }

    JaCore *core = ja_core_create();
    ja_core_wait(core);
    ja_core_quit(core);

    return (0);
}
