/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2002 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include "yeahwm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

void make_new_client(Window w)
{
    Client *c;
    XWindowAttributes attr;
    long dummy;
    XWMHints *hints;
    XClassHint *class;
    XGrabServer(dpy);
    c = (Client *) malloc(sizeof(Client));
    /* Don't crash the window manager, just fail the operation. */
    if (!c) {
#ifdef STDIO
        fprintf(stderr, "out of memory in new_client; limping onward\n");
#endif
        return;
    }
    /* We do this first of all as a test to see if the window actually
     * still exists by the time we've got to create a client structure
     * for it (sometimes they vanish too quickly or something, and lots
     * of pain ensues). */
    initialising = w;
    XFetchName(dpy, w, &c->name);
    /* If 'initialising' is now set to None, that means doing the
     * XFetchName raised BadWindow - the window has been removed before
     * we got a chance to grab the server. */
    if (initialising == None) {
#ifdef DEBUG
        fprintf(stderr, "make_new_client() : XError occurred for initialising window - aborting...\n");
#endif
        free(c);
        XSync(dpy, False);
        XUngrabServer(dpy);
        return;
    }
    initialising = None;
    c->next = head_client;
    head_client = c;
    c->window = w;
    c->ignore_unmap = 0;
    c->shade = 0;
    c->tab_offset = 0;

    //if(!c->name) c->name = "";//TODO
    c->tab_width = (c->name) ? (XTextWidth(font, c->name, strlen(c->name)) + 8) : 6;
    /* Jon Perkin reported a crash wish an app called 'sunpci' which we
     * traced to getting divide-by-zeros because it sets PResizeInc
     * but then has increments as 0.  So we check for 0s here and set them
     * to sensible defaults. */
    c->size = XAllocSizeHints();
    if (c->size->width_inc == 0)
        c->size->width_inc = 1;
    if (c->size->height_inc == 0)
        c->size->height_inc = 1;
#ifdef XDEBUG
    fprintf(stderr, "XGetWMNormalHints(); ");
#endif
    XGetWMNormalHints(dpy, c->window, c->size, &dummy);
    XGetWindowAttributes(dpy, c->window, &attr);
    c->x = attr.x;
    c->y = attr.y;
    c->width = attr.width;
    c->height = attr.height;

    c->border = opt_bw;
    c->oldw = c->oldh = 0;
    /* If we don't have MWM_HINTS (ie, lesstif) for a client to tell us
     * it has no border, I include this *really blatant hack* to remove
     * the border from XMMS. */
    c->title = (c->border) ? 1 : 0;
#ifdef COLOURMAP
    c->cmap = attr.colormap;
#endif
    c->vdesk = vdesk;
#ifdef DEBUG
    {
        Client *p;
        int i = 0;
        for (p = head_client; p; p = p->next)
            i++;
        fprintf(stderr,
                "make_new_client() : new window %dx%d+%d+%d, wincount=%d\n", c->width, c->height, c->x, c->y, i);
    }
#endif
    /* c->size = XAllocSizeHints();
       XGetWMNormalHints(dpy, c->window, c->size, &dummy); */
    if (attr.map_state == IsViewable) {
        c->ignore_unmap++;
    } else {
        init_position(c);
        if ((hints = XGetWMHints(dpy, w))) {
            if (hints->flags & StateHint)
                set_wm_state(c, hints->initial_state);
            XFree(hints);
        }
    }

    /* client is initialised */
    gravitate(c);

    reparent(c);

#ifdef DEBUG
    if (wm_state(c) == IconicState) {
        fprintf(stderr, "make_new_client() : client thinks it's iconised\n");
    } else {
        if (wm_state(c) == NormalState) {
            fprintf(stderr, "make_new_client() : client is in NormalState - good\n");
        } else {
            if (wm_state(c) == WithdrawnState) {
                fprintf(stderr, "make_new_client() : silly client!  it's in WithdrawnState\n");
            } else {
                fprintf(stderr, "make_new_client() : don't know what state client is in\n");
            }
        }
    }
#endif

    XMapWindow(dpy, c->window);
    XMapRaised(dpy, c->parent);
    c->ignore_unmap++;
    XUnmapWindow(dpy, c->tab);
    c->title = 0;
      
    set_wm_state(c, NormalState);
    XSync(dpy, False);
    XUngrabServer(dpy);
}

void init_position(Client * c)
{
    int x, y;
    int xn = xin_screen();
    int xmax = DisplayWidth(dpy, screen);
    int ymax = DisplayHeight(dpy, screen);
    if (c->width < MINSIZE)
        c->width = MINSIZE;
    if (c->height < MINSIZE)
        c->height = MINSIZE;
    if (c->width > xmax)
        c->width = xmax;
    if (c->height > ymax)
        c->height = ymax;
    if (c->x + c->y == 0)       //has the window a initial position specified by its attributes?
    {
        if (c->size->flags & (PPosition | USPosition)) {
            c->x = c->size->x;
            c->y = c->size->y;
            if (c->x < 0 || c->y < 0 || c->x > xmax || c->y > ymax)
                c->x = c->y = c->border;
        } else {
            get_mouse_position(&x, &y);
            c->x = (x * (xmax - c->border - c->width)) / xmax;
            c->y = (y * (ymax - c->border - c->height)) / ymax;
        }
        /* reposition if maximised horizontally or vertically */
        if (c->x == 0 && c->width == xmax)
            c->x = -c->border;
        if (c->y == 0 && c->height == ymax)
            c->y = -c->border;
    }
    if (c->x + c->width + c->border > xinerama[xn].width + xinerama[xn].x_org)
        c->x = xinerama[xn].width - c->width - 2 * c->border;
    if (c->x - c->border < xinerama[xn].x_org)
        c->x = xinerama[xn].x_org /* + c->border */ ;
    if (c->y + c->border + c->height > xinerama[xn].height + xinerama[xn].y_org)
        c->y = xinerama[xn].height + xinerama[xn].y_org - 2 * c->border - c->height;
    if (c->y - c->border < xinerama[xn].y_org)
        c->y = xinerama[xn].y_org /* + c->border */  ;

}

void reparent(Client * c)
{
    XSetWindowAttributes p_attr;
#ifdef COLOURMAP
    XSelectInput(dpy, c->window, ColormapChangeMask | EnterWindowMask | PropertyChangeMask);
#else
    XSelectInput(dpy, c->window, EnterWindowMask | PropertyChangeMask);
#endif
    p_attr.override_redirect = True;
    p_attr.background_pixel = bg.pixel;
    p_attr.event_mask = ChildMask | ButtonPressMask /* | ExposureMask */  |
        EnterWindowMask;
    c->parent =
        XCreateWindow(dpy, root, c->x - c->border, c->y - c->border,
                      c->width + (c->border * 2),
                      c->height + (c->border * 2), 0, DefaultDepth(dpy,
                                                                   screen),
                      CopyFromParent, DefaultVisual(dpy, screen),
                      CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
    p_attr.event_mask = ChildMask | ButtonPressMask | ExposureMask | EnterWindowMask;
    if (c->tab_width > c->width)
        c->tab_width = c->width;
    c->tab = XCreateWindow(dpy, root, c->x - c->border,
                           c->y - c->border - theight, c->tab_width,
                           theight, 0, DefaultDepth(dpy, screen),
                           CopyFromParent, DefaultVisual(dpy, screen),
                           CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
#ifdef SHAPE
    if (have_shape) {
        XShapeSelectInput(dpy, c->window, ShapeNotifyMask);
        set_shape(c);
    }
#endif
    XAddToSaveSet(dpy, c->window);
    XSetWindowBorderWidth(dpy, c->window, 0);
    XReparentWindow(dpy, c->window, c->parent, c->border, c->border);
    send_config(c);
}



