 /* evilwm - Minimalist Window Manager for X
  * Copyright (C) 1999-2002 Ciaran Anscomb <evilwm@6809.org.uk>
  * see README for license and other details. */

#include "yeahwm.h"
#include <stdlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif
void shift_tab(Client * c);
void pirate(Client * c);
void fix_window(Client * c);
void fix_next_vdesk(Client * c);
void fix_prev_vdesk(Client * c);


void handle_key_event(XKeyEvent * e)
{
    KeySym key = XKeycodeToKeysym(dpy, e->keycode, 0);
    int xn = xin_screen();
    if (current) {
        switch (key) {
            /* Sorry about all these if (0)s, but they actually
             * *do* do something useful... */
        case KEY_LEFT:
            current->x -= 16;
            if (0)
        case KEY_DOWN:
                current->y += 16;
            if (0)
        case KEY_UP:
                current->y -= 16;
            if (0)
        case KEY_RIGHT:
                current->x += 16;
            snap_client(current, MOVE);
            move(current);
            setmouse(current->window, current->width - 1, current->height - 1);
            break;
        case KEY_TOPLEFT:
            {
       current->x = current->border;
       current->y = current->border;
       recalculate_sweep(current);
            }
            if (0)
        case KEY_TOPRIGHT:
                {
                    current->x = xinerama[xn].width + xinerama[xn].x_org - current->width - current->border;
                    current->y = current->border + xinerama[xn].y_org + current->title * theight;
                }
            if (0)
        case KEY_BOTTOMLEFT:
                {
                    current->x = current->border + xinerama[xn].x_org;
                    current->y = xinerama[xn].height + xinerama[xn].y_org - current->height - current->border;
                }
            if (0)
        case KEY_BOTTOMRIGHT:
                {
                    current->x = xinerama[xn].width + xinerama[xn].x_org - current->width - current->border;
                    current->y = xinerama[xn].height + xinerama[xn].y_org - current->height - current->border;
                }

            move(current);
            break;
        case KEY_GROWVERT:
            current->height += 16;
            recalculate_sweep(current);
            if (0)
        case KEY_SHRINKVERT:
                current->height -= 16;
            recalculate_sweep(current);
            if (0)
        case KEY_GROWHOR:
                current->width += 16;
            recalculate_sweep(current);
            if (0)
        case KEY_SHRINKHOR:
                current->width -= 16;
            if (0)
	case KEY_HALF:
                current->height /= 2;
                current->width  /= 2;
            recalculate_sweep(current);
            XRaiseWindow(dpy, current->parent);
            XRaiseWindow(dpy, current->tab);
            snap_client(current, RESIZE);
            resize(current);
            setmouse(current->window, current->width - 1, current->height - 1);
            break;
        case KEY_KILL:
            send_wm_delete(current);
            break;
        case KEY_LOWER:
            XLowerWindow(dpy, current->parent);
            XLowerWindow(dpy, current->tab);
            break;
        case KEY_RAISE:
            XRaiseWindow(dpy, current->parent);
            XRaiseWindow(dpy, current->tab);
            break;
        case KEY_TITLE:

            break;
        case KEY_SHADE:
            if (current->y = current->border) 
	     {
   		current->y += 16;
	        snap_client(current, MOVE);
	        move(current);
	        setmouse(current->window, current->width - 1, current->height - 1);
	     }
	    toggle_shade(current);
            break;
        case KEY_MAX:
            maximise_vert(current);
            maximise_horiz(current);
            resize(current);
            break;
        case KEY_MAXVERT:
            maximise_vert(current);
            resize(current);
            break;
        case KEY_MAX_HOR:
            maximise_horiz(current);
            resize(current);
            break;
        case KEY_FIX:
	    fix_window(current);
            break;
        case KEY_FIX_NEXT:
	    fix_next_vdesk(current);
            break;
        case KEY_FIX_PREV:
	    fix_prev_vdesk(current);
            break;
        }
    }
    switch (key) {
    case KEY_CMD1:
        spawn(opt_cmd1);
        break;
    case KEY_CMD2:
        spawn(opt_cmd2);
        break;
    case KEY_CMD3:
        spawn(opt_cmd3);
        break;
    case KEY_NEXT:
        next();
        break;
    case KEY_XOSD_VDESK:
        xosd_vdesk();
        break;

    case XK_1:
    case XK_2:
    case XK_3:
    case XK_4:
    case XK_5:
    case XK_6:
    case XK_7:
    case XK_8:
    case XK_9:
        if (KEY_TO_VDESK(key) > opt_vd)
            opt_vd = KEY_TO_VDESK(key);
        switch_vdesk(KEY_TO_VDESK(key));
        break;
    case KEY_PREVDESK:
        if (vdesk > KEY_TO_VDESK(XK_1))
            switch_vdesk(vdesk - 1);
        break;
    case KEY_NEXTDESK:
        if (vdesk < opt_vd)
            switch_vdesk(vdesk + 1);
        break;
    }
}

void handle_button_event(XButtonEvent * e)
{
    if (current && (e->window == current->tab || e->window == current->parent)) {
        switch (e->button) {
        case Button1:
            if (e->window == current->tab && e->x > current->tab_width - DEF_PWIDTH) {
                pirate(current);
                return;
            }
            if (e->state & ShiftMask ) {
                shift_tab(current);
                return;
            }
            XRaiseWindow(dpy, current->parent);
            XRaiseWindow(dpy, current->tab);
            drag(current);
            break;
        case Button2:
            toggle_shade(current);
            break;
        case Button3:
            sweep(current);
            break;
        case Button4:
            XRaiseWindow(dpy, current->parent);
            XRaiseWindow(dpy, current->tab);
            break;
        case Button5:
            XLowerWindow(dpy, current->parent);
            XLowerWindow(dpy, current->tab);
        }
    } else {
        if (e->button == Button5 && vdesk > 1) {
            switch_vdesk(vdesk - 1);
            return;
        }
        if (e->button == Button4 && vdesk < opt_vd) {
            switch_vdesk(vdesk + 1);
            return;
        }
        if (e->button == Button1 || e->button == Button3)
            select_window();
    }
}

void handle_configure_request(XConfigureRequestEvent * e)
{
    Client *c = find_client(e->window);
    XWindowChanges wc;
    wc.sibling = e->above;
    wc.stack_mode = e->detail;
    if (c) {
        ungravitate(c);
        if (e->value_mask & CWWidth)
            c->width = e->width;
        if (e->value_mask & CWHeight)
            c->height = e->height;
        if (e->value_mask & CWX)
            c->x = e->x;
        if (e->value_mask & CWY)
            c->y = e->y;
        if (c->x == 0 && c->width >= DisplayWidth(dpy, screen)) {
            c->x -= c->border;
        }
        if (c->y == 0 && c->height >= DisplayHeight(dpy, screen)) {
            c->y -= c->border;
        }
        gravitate(c);
        wc.x = c->x - c->border;
        wc.y = c->y - c->border;
        wc.width = c->width + (c->border * 2);
        wc.height = c->height + (c->border * 2);
        wc.border_width = 0;

        XConfigureWindow(dpy, c->parent, e->value_mask, &wc);
        check_tab_pos_size(c);
        XMoveResizeWindow(dpy, c->tab, c->x - c->border + c->tab_offset, c->y - c->border - theight, c->tab_width,
                          theight);
        XRaiseWindow(dpy, c->tab);
        send_config(c);
#ifdef DEBUG
        fprintf(stderr,
                "handle_configure_request() : window configured to %dx%d+%d+%d\n", wc.width, wc.height, wc.x, wc.y);
#endif
    }
    wc.x = c ? c->border : e->x;
    wc.y = c ? c->border : e->y;
    wc.width = e->width;
    wc.height = e->height;
    XConfigureWindow(dpy, e->window, e->value_mask, &wc);
}

void handle_map_request(XMapRequestEvent * e)
{
    Client *c = find_client(e->window);
    if (c) {
        if (c->vdesk != vdesk) {
            switch_vdesk(c->vdesk);
        }
        unhide(c, RAISE);
    } else {
        XWindowAttributes attr;
#ifdef DEBUG
        fprintf(stderr, "handle_map_request() : don't know this window, calling make_new_client();\n");
#endif
        XGetWindowAttributes(dpy, e->window, &attr);
        make_new_client(e->window);
    }
}

void handle_unmap_event(XUnmapEvent * e)
{
    Client *c = find_client(e->window);
    if (c) {
#ifdef DEBUG
        /* fprintf(stderr, "handle_unmap_event() : ignore_unmap = %d\n", c->ignore_unmap);
         * */
#endif
        if (c->ignore_unmap)
            c->ignore_unmap--;
        else
            remove_client(c);
    }
}

void handle_client_message(XClientMessageEvent * e)
{
    Client *c = find_client(e->window);
    if (c && e->message_type == xa_wm_change_state && e->format == 32 && e->data.l[0] == IconicState)
        hide(c);
}

#ifdef COLOURMAP
void handle_colormap_change(XColormapEvent * e)
{
    Client *c = find_client(e->window);
    if (c && e->new) {
        c->cmap = e->colormap;
        XInstallColormap(dpy, c->cmap);
    }
}
#endif

void handle_property_change(XPropertyEvent * e)
{
    Client *c = find_client(e->window);
    long dummy;
    if (c) {
        if (e->atom == XA_WM_NORMAL_HINTS)
            XGetWMNormalHints(dpy, c->window, c->size, &dummy);
        if (e->atom == XA_WM_NAME) {
            if (c->name)
                XFree(c->name);
            XFetchName(dpy, c->window, &c->name);
            check_tab_pos_size(c);
            XMoveResizeWindow(dpy, c->tab, c->x - c->border + c->tab_offset, c->y - c->border - theight, c->tab_width,
                              theight);
            XClearWindow(dpy, c->tab);
            show_info(c);
        }

    }
}

void handle_enter_event(XCrossingEvent * e)
{
    Client *c;
    int wdesk;
    if ((c = find_client(e->window))) {
       wdesk = c->vdesk;
        if (wdesk != vdesk && wdesk != STICKY)
            return;
#ifdef COLOURMAP
        XInstallColormap(dpy, c->cmap);
#endif
        if (c != current)      //avoid some flicker
            client_update_current(c);
            if (!c->shade)
                XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
            grab_button(c->parent, MOUSE_MOD, AnyButton);
            XSync(dpy, False);  //TODO needed or not?

        
    }
}

#ifdef SHAPE
void handle_shape_event(XShapeEvent * e)
{
    Client *c = find_client(e->window);
    if (c)
        set_shape(c);
}
#endif
void handle_expose_event(XExposeEvent * e)
{
    Client *c = find_client(e->window);
    if (c && c->title && e->count == 0 && e->window == c->tab)  //e->count (the number* of outstanding exposes) 
        show_info(c);
}


void shift_tab(Client * c)
{
    XEvent ev;
    int old_offset = c->tab_offset;
    int x1, y1;
    if (!grab_pointer(root, MouseMask, tab_curs))
        return;
    get_mouse_position(&x1, &y1);
    for (;;) {
        XMaskEvent(dpy, ExposureMask | MouseMask, &ev);
        switch (ev.type) {
        case Expose:
            handle_expose_event(&ev.xexpose);
            break;
        case MotionNotify:
            c->tab_offset = old_offset + ev.xmotion.x - x1;
            check_tab_pos_size(c);
            XMoveWindow(dpy, c->tab, c->x - c->border + c->tab_offset, c->y - c->border - theight);
            break;
        case ButtonRelease:
            XUngrabPointer(dpy, CurrentTime);
            return;
        }
    }
}

void pirate(Client * c)
{
    XEvent ev;
    if (XGrabPointer(dpy, root, False, ButtonMask, GrabModeAsync, GrabModeAsync, None, pirate_curs, 0))
        return;
    for (;;) {
        XMaskEvent(dpy, MouseMask, &ev);
        if (ev.type == ButtonRelease) {
            if (ev.xbutton.subwindow == c->parent) {
                send_wm_delete(c);
            }
            XUngrabPointer(dpy, CurrentTime);
            return;
        }
    }
}


void fix_window(Client * c) {
            current->vdesk = current->vdesk == STICKY ? vdesk : STICKY;
            client_update_current(c);
}


void fix_next_vdesk(Client * c) {
    fix_window(c);
    switch_vdesk(vdesk + 1);
    fix_window(c);
}

void fix_prev_vdesk(Client * c) {
    fix_window(c);
    switch_vdesk(vdesk - 1);
    fix_window(c);
}
