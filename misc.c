/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2002 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include "yeahwm.h"
#include <X11/Xproto.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

void spawn(const char *cmd)
{
    pid_t pid;

    if (!(pid = fork())) {
        setsid();
        switch (fork()) {

        case 0:
            execlp("/bin/sh", "sh", "-c", cmd, NULL);
        default:
            _exit(0);
        }
    }
    if (pid > 0)
        wait(NULL);
}

void handle_signal(int signo)
{

    /* SIGCHLD check no longer necessary */
    /* Quit Nicely */
    quitting = 1;
    while (head_client)
        remove_client(head_client);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    if (font)
        XFreeFont(dpy, font);
    XFreeGC(dpy, string_gc);
    XInstallColormap(dpy, DefaultColormap(dpy, screen));
    XCloseDisplay(dpy);
    exit(0);
}

int handle_xerror(Display * dsply, XErrorEvent * e)
{
    Client *c = find_client(e->resourceid);

    /* If this error actually occurred while setting up the new
     * window, best let make_new_client() know not to bother */
    if (initialising != None && e->resourceid == initialising) {
#ifdef DEBUG
        fprintf(stderr, "\t **SAVED?** handle_xerror() caught error %d while initialising\n", e->error_code);
#endif
        initialising = None;
        return 0;
    }
#ifdef DEBUG
    fprintf(stderr, "**ERK** handle_xerror() caught an XErrorEvent: %d\n", e->error_code);
#endif
    /* if (e->error_code == BadAccess && e->resourceid == root) { */
    if (e->error_code == BadAccess && e->request_code == X_ChangeWindowAttributes) {
#ifdef STDIO
        fprintf(stderr, "root window unavailable (maybe another wm is running?)\n");
#endif
        exit(1);
    }
#ifdef XDEBUG
    fprintf(stderr, "XError %x ", e->error_code);
#endif
    /* Kludge around IE misbehaviour */
    if (e->error_code == 0x8 && e->request_code == 0x0c && e->minor_code == 0x00) {
#ifdef DEBUG
        fprintf(stderr, "\thandle_xerror() : IE kludge - ignoring XError\n");
#endif
        return 0;
    }

    if (c) {
#ifdef DEBUG
        fprintf(stderr, "\thandle_xerror() : calling remove_client()\n");
#endif
        remove_client(c);
    }
    return 0;
}

int ignore_xerror(Display * dsply, XErrorEvent * e)
{
#ifdef DEBUG
    fprintf(stderr, "ignore_xerror() caught an XErrorEvent: %d\n", e->error_code);
#endif
    return 0;
}

#ifdef DEBUG
void dump_clients()
{
    Client *c;

    for (c = head_client; c; c = c->next)
        fprintf(stderr, "MISC: (%d, %d) @ %d,%d\n", wm_state(c), c->ignore_unmap, c->x, c->y);
}
#endif                          /* DEBUG */
