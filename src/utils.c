/*
 * utils.c
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
#include <pwd.h>


#define LOCKMODE    (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)


/* lock a file */
gboolean lockfile(gint fd);

/*
 * Daemonizes current process
 * Returns TRUE on success
 * Otherwise FALSE
 */
gboolean daemonize(void)
{
    umask(0);

    //创建子进程
    pid_t pid;
    if ((pid = fork()) < 0) {
        return FALSE;
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
        return FALSE;
    } else if (pid) {
        _exit(0);
    }

    chdir("/");

    close_fds();

    return TRUE;
}

/*
 * Closes all open file descriptor
 */
gboolean close_fds(void)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        return FALSE;
    }
    gint i;
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++) {
        close(i);
    }
    gint fd0 = open("/dev/null", O_RDWR);
    gint fd1 = dup(fd0);
    gint fd2 = dup(fd0);
    if (fd0 != 0 || fd1 != 0 || fd2 != 0) {
        return FALSE;
    }
    return TRUE;
}


/*
 * Check if the process is already running
 * Returns 1 if yes
 * Returns 0 if no
 * Returns -1 on error
 */
gint already_running(void)
{
    gint fd = open(CONFIG_PID_FILE, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        return -1;
    }
    if (!lockfile(fd)) {
        if (errno == EACCES || errno == EAGAIN) {
            gchar buf[16];
            gint n = read(fd, buf, sizeof(buf) / sizeof(gchar));
            if (n > 0) {
                buf[n] = '\0';
                close(fd);
                gint pid = atoi(buf);
                if (pid > 0) {
                    return pid;
                }
                return -1;
            }
        }
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

/*
 * Locks the pid file
 */
gboolean lock_pidfile(void)
{
    gint fd = open(CONFIG_PID_FILE, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        return FALSE;
    }
    if (!lockfile(fd)) {
        close(fd);
        return FALSE;
    }
    ftruncate(fd, 0);
    gchar buf[32];
    snprintf(buf, sizeof(buf), "%ld", (long) getpid());
    write(fd, buf, strlen(buf) + 1);
    return TRUE;
}


gboolean lockfile(gint fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl) != 0 ? FALSE : TRUE;
}

/*
 * Sets the current process effective user as name
 * Returns 1 on success
 * 0 otherwise
 */
gboolean setuser(const gchar * name)
{
    struct passwd *pw = getpwnam(name);
    if (pw == NULL) {
        return FALSE;
    }
    if (setuid(pw->pw_uid) || seteuid(pw->pw_uid)) {
        return FALSE;
    }
    return TRUE;
}


/*
 * Opens a file for writing
 * Returns NULL on error
 */
gint open_appendable(const gchar * name)
{
    gint fd = open(name, O_WRONLY | O_APPEND | O_CREAT,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return fd;
}

extern gchar **environ;
static gchar *argstart = NULL;
static size_t maxarglen;        /* maximum available size of argument area */
static gint envmoved = 0;

void set_proctitle(gchar ** argv,
                   /* argv as passed to main, so args can be moved if necessary */
                   const gchar * fmt,   /* printf(3)-style format string for process title */
                   ... /* args to format string */ )
  /* something as close as possible to BSD setproctitle(3), but for Linux.
     Note I need argv as passed to main, in order to be able to poke the process
     arguments area. Also don't call routines like putenv(3) or setenv(3)
     prior to using this routine. */
{
    gchar title[512];           /* big enough? */
    ssize_t titlelen;
    {
        va_list args;
        va_start(args, fmt);
        titlelen = vsnprintf(title, sizeof title, fmt, args);
        va_end(args);
        if (titlelen < 0) {
            titlelen = 0;       /* ignore error */
            title[0] = 0;
        }                       /*if */
        titlelen += 1;          /* including trailing nul */
        if (titlelen > sizeof title) {
            title[sizeof title - 1] = '\0'; /* do I need to do this? */
            titlelen = sizeof title;
        }                       /*if */
    }
    if (argstart == NULL) {
        /* first call, find and initialize argument area */
        gchar **thisarg = argv;
        maxarglen = 0;
        argstart = *thisarg;
        while (*thisarg != NULL) {
            maxarglen += strlen(*thisarg++) + 1;    /* including terminating nul */
        }                       /*while */
        memset(argstart, 0, maxarglen); /* clear it all out */
    }                           /*if */
    if (titlelen > maxarglen && !envmoved) {
        /* relocate the environment strings and use that area for the command line
           as well */
        gchar **srcenv;
        gchar **dstenv;
        gchar **newenv;
        size_t envlen = 0;
        size_t nrenv = 1;       /* nr env strings + 1 for terminating NULL pointer */
        if (argstart + maxarglen == environ[0]) {   /* not already moved by e.g. libc */
            srcenv = environ;
            while (*srcenv != NULL) {
                envlen += strlen(*srcenv++) + 1;    /* including terminating nul */
                ++nrenv;        /* count 'em up */
            }                   /*while */
            newenv = (gchar **) malloc(sizeof(gchar *) * nrenv);    /* new env array, never freed! */
            srcenv = environ;
            dstenv = newenv;
            while (*srcenv != NULL) {
                /* copy the environment strings */
                *dstenv++ = strdup(*srcenv++);
            }                   /*while */
            *dstenv = NULL;     /* mark end of new environment array */
            memset(environ[0], 0, envlen);  /* clear old environment area */
            maxarglen += envlen;    /* this much extra space now available */
            environ = newenv;   /* so libc etc pick up new environment location */
        }                       /*if */
        envmoved = 1;
    }                           /*if */
    if (titlelen > maxarglen) {
        titlelen = maxarglen;   /* truncate to fit available area */
    }                           /*if */
    if (titlelen > 0) {
        /* set the new title */
        const size_t oldtitlelen = strlen(argstart) + 1;    /* including trailing nul */
        memcpy(argstart, title, titlelen);
        argstart[titlelen - 1] = '\0';  /* if not already done */
        if (oldtitlelen > titlelen) {
            /* wipe out remnants of previous title */
            memset(argstart + titlelen, 0, oldtitlelen - titlelen);
        }                       /*if */
    }                           /*if */
}                               /*setproctitle */
