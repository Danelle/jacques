/*
 * utils.c
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
#include "utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>



#define PID_FILE    RUNTIME_LOCATION "/jacques.pid"
#define LOCKMODE    (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)


/* lock a file */
int lockfile(int fd);

/*
 * Daemonizes current process
 * Returns 1 on success
 * Otherwise 0
 */
int daemonize(void)
{
    umask(0);

    //创建子进程
    pid_t pid;
    if ((pid = fork()) < 0) {
        return 0;
    } else if (pid) {           /* parent */
        _exit(0);
    }

    setsid();                   /* create session */

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP, &sa, NULL);
    if ((pid = fork()) < 0) {
        return 0;
    } else if (pid) {
        _exit(0);
    }

    chdir("/");

    return 1;
}

/*
 * Closes all open file descriptor
 */
int close_fds(void)
{
    //获取文件描述符最大值
    struct rlimit r1;
    getrlimit(RLIMIT_NOFILE, &r1);

    /* close file descriptor */
    if (r1.rlim_max == RLIM_INFINITY) {
        r1.rlim_max = 1024;
    }
    int i;
    for (i = 0; i < r1.rlim_max; i++) {
        close(i);
    }
    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);
    if (fd0 != 0 || fd1 != 0 || fd2 != 0) {
        return 0;
    }
    return 1;
}

/*
 * Check if the process is already running
 * Returns 1 if no
 * Returns 0 if yes
 * Returns -1 on error
 */
int already_running(void)
{
    int fd = open(PID_FILE, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        return -1;
    }
    if (lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return 1;
        }
        return -1;
    }
    ftruncate(fd, 0);
    char buf[32];
    snprintf(buf, sizeof(buf), "%ld", (long) getpid());
    write(fd, buf, strlen(buf) + 1);
    return 0;
}


int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
}
