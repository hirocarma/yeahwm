/*
 * evilwm - Minimalist Window Manager for X Copyright (C) 1999-2002 Ciaran 
 * Anscomb <evilwm@6809.org.uk> see README for license and other details. 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "yeahwm.h"
#include <unistd.h>
#include <time.h>

#include "xosd.h"

#define THEIGHT (theight * c->title)

#define ITHEIGHT (theight * ci->title)

void show_info(Client * c)
{
    if (c->name)
        XDrawString(dpy, c->tab, string_gc, 2, theight - 3, c->name, strlen(c->name));
    XClearArea(dpy, c->tab, c->tab_width - 4, 0, 4, theight, False);
    XDrawLine(dpy, c->tab, string_gc, c->tab_width - 2, 1, c->tab_width - 2, theight);
}

void toggle_shade(Client * c)
{
   if (!c->shade) {
        XMapWindow(dpy, c->tab);
        c->title = 1;

        c->ignore_unmap += 2;
        XUnmapWindow(dpy, c->parent);
        XUnmapWindow(dpy, c->window);
        set_wm_state(c, IconicState);
        c->shade = 1;
    } else {
        XMapWindow(dpy, c->parent);
        XMapWindow(dpy, c->window);
        c->ignore_unmap = 0;
        c->shade = 0;
        set_wm_state(c, NormalState);
        
        XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
        c->ignore_unmap++;
        XUnmapWindow(dpy, c->tab);
        c->title = 0;
    }
}

void get_mouse_position(int *x, int *y)
{
    Window dw1, dw2;
    int t1, t2;
    unsigned int t3;
    XQueryPointer(dpy, root, &dw1, &dw2, x, y, &t1, &t2, &t3);
}

void recalculate_sweep(Client * c)
{
    int basex, basey;

    /*  c->width = abs(x1 - x2);
       c->height = abs(y1 - y2); */
    if (c->size->flags & PResizeInc) {
        basex =
            (c->size->flags & PBaseSize) ? c->size->base_width : (c->size->flags & PMinSize) ? c->size->min_width : 0;
        basey = (c->size->flags & PBaseSize) ? c->size->base_height : (c->size->flags & PMinSize)
            ? c->size->min_height : 0;
        c->width -= (c->width - basex) % c->size->width_inc;
        c->height -= (c->height - basey) % c->size->height_inc;
    }
    if (c->size->flags & PMinSize) {
        if (c->width < c->size->min_width)
            c->width = c->size->min_width;
        if (c->height < c->size->min_height)
            c->height = c->size->min_height;
    }
    if (c->size->flags & PMaxSize) {
        if (c->width > c->size->max_width)
            c->width = c->size->max_width;
        if (c->height > c->size->max_height)
            c->height = c->size->max_height;
    }
    if (c->size->flags & PAspect) {
        c->height = abs(c->width * c->size->min_aspect.y / c->size->min_aspect.x);
    }
    /* c->x = (x1 <= x2) ? x1 : x1 - c->width;
       c->y = (y1 <= y2) ? y1 : y1 - c->height; */
}

void sweep(Client * c)
{
    XEvent ev;
    if (!grab_pointer(root, MouseMask, resize_curs))
        return;
    if (c->shade)
        toggle_shade(c);
        c->oldx = 0;
        c->oldy = 0;            //forget old position when window is resized
        c->oldw = 0;
        c->oldh = 0;
    setmouse(c->window, c->width, c->height);
    for (;;) {
        XMaskEvent(dpy, ButtonMask | ExposureMask | PointerMotionHintMask, &ev);
        switch (ev.type) {
        case Expose:
            handle_expose_event(&ev.xexpose);
            break;
        case MotionNotify:
            c->width = abs(ev.xmotion.x - c->x);
            c->height = abs(ev.xmotion.y - c->y);
            snap_client(c, RESIZE);
            recalculate_sweep(c);
            XMoveResizeWindow(dpy, c->parent, c->x - c->border,
                              c->y - c->border, c->width + (c->border * 2), c->height + (c->border * 2));
            if (opt_sores)
                XMoveResizeWindow(dpy, c->window, c->border, c->border, c->width, c->height);
            check_tab_pos_size(c);
            XMoveResizeWindow(dpy, c->tab,
                              c->x - c->border + c->tab_offset, c->y - c->border - theight, c->tab_width, theight);
            break;
        case ButtonRelease:
            XMoveResizeWindow(dpy, c->window, c->border, c->border, c->width, c->height);
            send_config(c);
            XUngrabPointer(dpy, CurrentTime);
            return;

        }
    }
}

static int absmin(int a, int b)
{
    if (abs(a) < abs(b))
        return a;
    return b;
}

void snap_client(Client * c, int mode)
{
    int dx, dy;
    int xn = xin_screen();
    Client *ci;
    /*
     * snap to other windows 
     */
    dx = dy = opt_snap;
    for (ci = head_client; ci; ci = ci->next) {
        if (ci != c && (ci->vdesk == vdesk || ci->vdesk == STICKY) && !ci->shade) {
            if (ci->y - ci->border - c->border - c->height -
                c->y <= opt_snap && c->y - c->border - ci->border - ci->height - ci->y <= opt_snap) {
                if (mode == MOVE) {
                    dx = absmin(dx, ci->x + ci->width - c->x + c->border + ci->border);
                    dx = absmin(dx, ci->x - c->x);
                }
                dx = absmin(dx, ci->x + ci->width - c->x - c->width);
                dx = absmin(dx, ci->x - c->x - c->width - c->border - ci->border);

            }
            if (ci->x - ci->border - c->border - c->width -
                c->x <= opt_snap && c->x - c->border - ci->border - ci->width - ci->x <= opt_snap) {
                if (mode == MOVE) {
                    dy = absmin(dy, ci->y + ci->height - c->y + c->border + ci->border);
                    dy = absmin(dy, ci->y + ci->height - c->y + c->border + ci->border + THEIGHT);
                    dy = absmin(dy, ci->y - c->y);

                }
                dy = absmin(dy, ci->y + ci->height - c->y - c->height);
                dy = absmin(dy, ci->y - c->y - c->height - c->border - ci->border);
                dy = absmin(dy, ci->y - ITHEIGHT - c->y - c->height - c->border - ci->border);

            }
        }
    }
    if (abs(dx) < opt_snap) {
        if (mode == MOVE)
            c->x += dx;
        else
            c->width += dx;
    }
    if (abs(dy) < opt_snap) {
        if (mode == MOVE)
            c->y += dy;
        else
            c->height += dy;
    }
    if (abs(c->x + c->width + c->border - xinerama[xn].width - xinerama[xn].x_org) < opt_snap) {
        if (mode == MOVE)
            c->x = xinerama[xn].width + xinerama[xn].x_org - c->width - c->border;
        else
            c->width = xinerama[xn].width + xinerama[xn].x_org - c->x - c->border;
    }
    if (abs(c->y + c->height + c->border - xinerama[xn].height - xinerama[xn].y_org) < opt_snap) {
        if (mode == MOVE)
            c->y = xinerama[xn].height + xinerama[xn].y_org - c->height - c->border;
        else
            c->height = xinerama[xn].height + xinerama[xn].y_org - c->y - c->border;
    }
    if (mode == MOVE) {
        if (abs(c->y - c->border - THEIGHT - xinerama[xn].y_org) < opt_snap)
            c->y = c->border + THEIGHT + xinerama[xn].y_org;
        if (abs(c->x - c->border - xinerama[xn].x_org) < opt_snap)
            c->x = c->border + xinerama[xn].x_org;
    }
}

void drag(Client * c)
{
    XEvent ev;
    int x1, y1;
    int old_cx = c->x;
    int old_cy = c->y;
    int displaywidth;
    c->oldx = 0;
    c->oldy = 0;                //forget old position when window is moved
    c->oldw = 0;
    c->oldh = 0;
    if (!grab_pointer(root, MouseMask, move_curs))
        return;
    get_mouse_position(&x1, &y1);
    displaywidth = DisplayWidth(dpy, screen);
    for (;;) {
        XMaskEvent(dpy, ExposureMask | MouseMask, &ev);
        switch (ev.type) {
        case Expose:
            handle_expose_event(&ev.xexpose);
            break;
        case MotionNotify:
            c->x = old_cx + (ev.xmotion.x - x1);
            c->y = old_cy + (ev.xmotion.y - y1);
            if (opt_snap)
                snap_client(c, MOVE);
            {
                XMoveWindow(dpy, c->parent, c->x - c->border, c->y - c->border);
                XMoveWindow(dpy, c->tab, c->x - c->border + c->tab_offset, c->y - c->border - theight);
                send_config(c);
            }
            if (c->vdesk != STICKY) {
                if (ev.xmotion.x > displaywidth - 3 && vdesk < opt_vd) {
                    setmouse(root, 5, ev.xmotion.y);
                    c->vdesk = c->vdesk + 1;
                    switch_vdesk(vdesk + 1);
                }
                if (ev.xmotion.x < 3 && vdesk > 1) {
                    setmouse(root, displaywidth - 5, ev.xmotion.y);
                    c->vdesk = c->vdesk - 1;
                    switch_vdesk(vdesk - 1);
                }
            }
            break;
        case ButtonRelease:
            XUngrabPointer(dpy, CurrentTime);
            return;
        default:
            break;
        }
    }
}

void move(Client * c)
{
    XMoveWindow(dpy, c->parent, c->x - c->border, c->y - c->border);
    XMoveWindow(dpy, c->tab, c->x - c->border + c->tab_offset, c->y - c->border - theight);
    send_config(c);
}

void resize(Client * c)
{
    XMoveResizeWindow(dpy, c->parent, c->x - c->border,
                      c->y - c->border, c->width + (c->border * 2), c->height + (c->border * 2));
    XMoveResizeWindow(dpy, c->window, c->border, c->border, c->width, c->height);
    send_config(c);
}

void maximise_horiz(Client * c)
{
    int xn = xin_screen();
#ifdef DEBUG
    fprintf(stderr, "SCREEN: maximise_horiz()\n");
#endif
    if (c->oldw) {
        c->x = c->oldx;
        c->width = c->oldw;
        c->oldw = 0;
    } else {

        c->oldx = c->x;
        c->oldw = c->width;
        c->x = xinerama[xn].x_org + c->border;
        c->width = xinerama[xn].width - 2 * c->border;
        recalculate_sweep(c);
    }
}

void maximise_vert(Client * c)
{
    int xn = xin_screen();
#ifdef DEBUG
    fprintf(stderr, "SCREEN: maximise_vert()\n");
#endif
    if (c->oldh) {
        c->y = c->oldy;
        c->height = c->oldh;
        c->oldh = 0;
    } else {
        c->oldy = c->y;
        c->oldh = c->height;
        c->y = c->border + THEIGHT + xinerama[xn].y_org;
        c->height = xinerama[xn].height  - 2 * c->border - THEIGHT;
        recalculate_sweep(c);
    }
}

void hide(Client * c)
{
    if (c) {
        c->ignore_unmap += 3;
#ifdef XDEBUG
        fprintf(stderr, "screen:XUnmapWindow(parent); ");
#endif
        XUnmapWindow(dpy, c->parent);
#ifdef XDEBUG
        fprintf(stderr, "screen:XUnmapWindow(window); ");
#endif
        XUnmapWindow(dpy, c->window);
        XUnmapWindow(dpy, c->tab);
        set_wm_state(c, IconicState);
    }
}

void unhide(Client * c, int raise)
{
    if (raise) {
        XMapRaised(dpy, c->tab);
        if (!c->shade) {
            XMapRaised(dpy, c->parent);
            XMapRaised(dpy, c->window);
        }

    } else {
        if (!c->shade) {
            XMapWindow(dpy, c->parent);
            XMapWindow(dpy, c->window);
        }
/*        if (c->title)
            XMapWindow(dpy, c->tab); */
    }
    set_wm_state(c, NormalState);
}

void next(void)
{
    Client *newc = current;

    XClassHint *class;

/*this is a mess!!!!!!!!!!!!*/
    if (!newc) {
#ifdef DEBUG
        fprintf(stderr, "NEXT: no current window, looking on this desktop\n");
#endif
        newc = head_client;
        if (!newc)
            return;
       
        if (opt_wlist) {
	   class = XAllocClassHint();
	   if (class) {
	      XGetClassHint(dpy, newc, class);
	      if (class->res_class) {
		 if (strstr(opt_wlist, class->res_class))
		   return;
	      }
	      XFree(class);
	   }
	} 
       
       
       if (newc->vdesk != vdesk && newc->vdesk != STICKY) {
            do {
                newc = newc->next;
            }
            while (newc && newc->vdesk != vdesk && newc->vdesk != STICKY);
        }

    } else {

        do {

            newc = newc->next;
            if (current && !newc) 
               newc = head_client;
	}
        while (newc && newc->vdesk != vdesk && newc->vdesk != STICKY);

    }
    if (newc) {

        if (newc->vdesk == vdesk || newc->vdesk == STICKY) {
            unhide(newc, RAISE);
            focus(newc);
            XSync(dpy, False);
        }
    }
}

void switch_vdesk(int v)
{
    Client *c;
    int wdesk;
#ifdef DEBUG
    int hidden = 0, raised = 0;
#endif
    if (v == vdesk)
        return;
    if (current && current->vdesk != STICKY && current->vdesk != v) {
        client_update_current(NULL);
    }
#ifdef DEBUG
    fprintf(stderr, "switch_vdesk() : Switching to desk %d\n", v);
#endif
   if (head_client) {
        for (c = head_client; c; c = c->next) {
            wdesk = c->vdesk;
            if (wdesk == vdesk) {
                hide(c);
#ifdef DEBUG
                hidden++;
#endif
            } else if (wdesk == v) {
                unhide(c, NO_RAISE);
#ifdef DEBUG
                raised++;
#endif

            }
        }
    }
    vdesk = v;
// add xosd support
    xosd_vdesk();
}

void xosd_vdesk()
{
   
 xosd *osd;
    osd = xosd_create(1);
    xosd_set_outline_offset(osd, 1);
    xosd_set_colour(osd, "green");
    xosd_set_outline_colour(osd, "black");
    xosd_set_font(osd, "-adobe-utopia-bold-r-normal-*-*-720-*-*-p-*-adobe-standard");
    xosd_set_align(osd, XOSD_center);
    xosd_set_pos(osd, XOSD_middle);
    xosd_set_timeout(osd, 1);
   
    char buf[3];
   
    snprintf(buf, 2, "%d", vdesk);
           
    xosd_display(osd, 0, XOSD_string, buf);
    xosd_wait_until_no_display(osd);
    xosd_destroy(osd);
}


void focus(Client * c)
{
    if (!c->shade)
     {
	XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
        c->ignore_unmap++;
        XUnmapWindow(dpy, c->tab); 
        c->title = 0; 
     }
   
    client_update_current(c);
}

int xin_screen()
{
    int x, y, n;
    get_mouse_position(&x, &y);
    for (n = 0; n <= xinerama_num; n++) {
        if (xinerama[n].x_org <= x &&
            xinerama[n].x_org + xinerama[n].width > x &&
            xinerama[n].y_org <= y && xinerama[n].y_org + xinerama[n].height > y)
            return n;
    }
    return 0;
}

void select_window()
{
    XEvent ev;
    Client *c;
    XGrabPointer(dpy, RootWindow(dpy, 0), False, ButtonMask, GrabModeAsync, GrabModeAsync, None, resize_curs, 0);
    for (;;) {
        XMaskEvent(dpy, ButtonMask | ExposureMask, &ev);
        switch (ev.type) {
        case Expose:
            handle_expose_event(&ev.xexpose);
            break;
        case ButtonPress:
            c = find_client(ev.xbutton.subwindow);
            //XSync(dpy, False);
            if (c) {
                switch (ev.xbutton.button) {
                case Button4:
                    XRaiseWindow(dpy, c->parent);
                    XRaiseWindow(dpy, c->tab);
                    continue;
                case Button5:
                    XLowerWindow(dpy, c->parent);
                    XLowerWindow(dpy, c->tab);
                    continue;
                case Button2:
                    toggle_shade(c);
                    break;
                case Button1:
                    XRaiseWindow(dpy, c->parent);
                    XRaiseWindow(dpy, c->tab);
                    drag(c);
                    break;
                case Button3:
                    sweep(c);
                    break;
                }
            }
            XUngrabPointer(dpy, CurrentTime);
            return;
        }
    }
}

void check_tab_pos_size(Client * c)
{
    if (c->name)
        c->tab_width = XTextWidth(font, c->name, strlen(c->name)) + 8;
    if (c->tab_offset && c->tab_width + c->tab_offset > c->width)
        c->tab_offset = c->width - c->tab_width;
    if (c->tab_width + c->tab_offset > c->width)
        c->tab_width = c->width - c->tab_offset;
    if (c->tab_offset < 0)
        c->tab_offset = 0;
}
