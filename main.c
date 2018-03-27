/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2002 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include "yeahwm.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <locale.h>
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif
#include <X11/extensions/Xinerama.h>
XineramaScreenInfo *xinerama = NULL;
int xinerama_num = 0;
int xin;
Display *dpy;
int screen;
Client *current = NULL;
Window initialising = None;
Window root;
XFontStruct *font;
XColor fg, abg, bg, fc;
Client *head_client;
Atom xa_wm_state;
Atom xa_wm_change_state;
Atom xa_wm_protos;
Atom xa_wm_delete;
Atom xa_wm_cmapwins;
Cursor move_curs;
Cursor resize_curs;
Cursor pirate_curs;
Cursor tab_curs;
const char *opt_display = "";
const char *opt_wlist=NULL;
const char *opt_font ;
const char *opt_abg ;
const char *opt_fg ;
const char *opt_bg ;
const char *opt_cmd1;
const char *opt_cmd2;
const char *opt_cmd3;
int opt_bw = 0;
int opt_sores = 0;
int theight;
GC string_gc;
XFontStruct *font;
const char *opt_fc;
int vdesk = 1;
int opt_vd = 0;
int opt_snap = 0;

#ifdef SHAPE
int have_shape, shape_event;
#endif
int quitting = 0;
#ifdef MWM_HINTS
Atom mwm_hints;
#endif
unsigned int numlockmask = 0;
void get_defaults(void);
static void *xmalloc(size_t size);

static void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr) {
#ifdef STDIO
        fprintf(stderr, "out of memory, looking for %d bytes\n", size);
#endif
        exit(1);
    }
    return ptr;
}

int main(int argc, char *argv[])
{
   setlocale(LC_ALL,"");
   struct sigaction act;
    int i;
    XEvent ev;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-fn") && i + 1 < argc)
            opt_font = argv[++i];
        else if (!strcmp(argv[i], "-display") && i + 1 < argc) {
            opt_display = argv[++i];
        } else if (!strcmp(argv[i], "-fg") && i + 1 < argc)
            opt_fg = argv[++i];
        else if (!strcmp(argv[i], "-wlist") && i + 1 < argc)
            opt_wlist = argv[++i];
        else if (!strcmp(argv[i], "-bg") && i + 1 < argc)
            opt_bg = argv[++i];
        else if (!strcmp(argv[i], "-abg") && i + 1 < argc)
            opt_abg = argv[++i];
        else if (!strcmp(argv[i], "-fc") && i + 1 < argc)
            opt_fc = argv[++i];
        else if (!strcmp(argv[i], "-vd") && i + 1 < argc)
            opt_vd = atoi(argv[++i]);
         else if (!strcmp(argv[i], "-sr") && i + 1 < argc)
            opt_sores =  atoi(argv[++i]); 
        else if (!strcmp(argv[i], "-bw") && i + 1 < argc)
            opt_bw = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-cmd1") && i + 1 < argc)
            opt_cmd1 = argv[++i];
        else if (!strcmp(argv[i], "-cmd2") && i + 1 < argc)
            opt_cmd2 = argv[++i];
        else if (!strcmp(argv[i], "-cmd3") && i + 1 < argc)
            opt_cmd3 = argv[++i];
        else if (!strcmp(argv[i], "-snap") && i + 1 < argc) {
            opt_snap = atoi(argv[++i]);
#ifdef STDIO
        } else if (!strcmp(argv[i], "-V")) {
            printf("yeahwm version " VERSION "\n");
            exit(0);
#endif
        } else {
#ifdef STDIO
            printf("usage: yeahwm [-display display] [-title 0/1] [-cmd1 command1] [-cmd2 command2] [-cmd3 command3]\n");
            printf("\t [-fg foreground] [-bg background] [-abg active background] [-fc fixed color][-fn font] \n");
            printf("\t[-snap num] [-vd num] [-wlist wclass1,wclass2,...] [-sr 0/1] [-V]\n");
#endif
            exit(2);
        }
    }
    act.sa_handler = handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGHUP, &act, NULL);

    setup_display();

#ifdef SHAPE
    have_shape = XShapeQueryExtension(dpy, &shape_event, &i);
#endif

    /* main event loop here */
    for (;;) {
        XNextEvent(dpy, &ev);
        switch (ev.type) {
        case KeyPress:
            handle_key_event(&ev.xkey);
            break;
        case ButtonPress:
            handle_button_event(&ev.xbutton);
            break;
        case ConfigureRequest:
            handle_configure_request(&ev.xconfigurerequest);
            break;
        case MapRequest:
            handle_map_request(&ev.xmaprequest);
            break;
            /* case ClientMessage:
               handle_client_message (&ev.xclient);
               break;
             */
#ifdef COLOURMAP
        case ColormapNotify:
            handle_colormap_change(&ev.xcolormap);
            break;
#endif
        case EnterNotify:
            handle_enter_event(&ev.xcrossing);
            break;
        case PropertyNotify:
            handle_property_change(&ev.xproperty);
            break;
        case UnmapNotify:
            handle_unmap_event(&ev.xunmap);
            break;
        case Expose:
            if (ev.xexpose.count == 0);
            handle_expose_event(&ev.xexpose);
            break;
        default:
#ifdef SHAPE
            if (have_shape && ev.type == shape_event)
                handle_shape_event((XShapeEvent *) & ev);
        }
#endif

    }
    return 1;
}

void setup_display()
{
    int event_base, error_base;
    XGCValues gv;
    XSetWindowAttributes attr;
    XColor dummy;
    XModifierKeymap *modmap;
    KeySym *keysym;
    KeySym keys_to_grab[] = {
        KEY_CMD1, KEY_CMD2, KEY_CMD3, KEY_KILL,
        KEY_TOPLEFT, KEY_TOPRIGHT, KEY_BOTTOMLEFT, KEY_BOTTOMRIGHT,
        KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP,
        KEY_LOWER, KEY_RAISE, KEY_TITLE, KEY_MAXVERT, KEY_MAX,
        KEY_GROWVERT, KEY_GROWHOR, KEY_SHRINKVERT, KEY_SHRINKHOR,
        KEY_SHADE, KEY_MAX_HOR,
        KEY_FIX, KEY_PREVDESK, KEY_NEXTDESK,
	KEY_XOSD_VDESK, KEY_FIX_NEXT, KEY_FIX_PREV,KEY_HALF,
        XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9,
        0
    };
    /* used in scanning windows (XQueryTree) */
    unsigned int i, j, nwins;
    Window dw1, dw2, *wins;
    XWindowAttributes winattr;
    dpy = XOpenDisplay(opt_display);
    if (!dpy) {
#ifdef STDIO
        fprintf(stderr, "can't open display %s\n", opt_display);
#endif
        exit(1);
    }
    get_defaults();

    XSetErrorHandler(handle_xerror);
    xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
    xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
    xa_wm_protos = XInternAtom(dpy, "WM_PROTOCOLS", False);
    xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
#ifdef COLOURMAP
    xa_wm_cmapwins = XInternAtom(dpy, "WM_COLORMAP_WINDOWS", False);
#endif
#ifdef MWM_HINTS
    mwm_hints = XInternAtom(dpy, _XA_MWM_HINTS, False);
#endif
    font = XLoadQueryFont(dpy, opt_font);
    if (!font)
        font = XLoadQueryFont(dpy, DEF_FONT);
    theight = font-> /* max_bounds. */ ascent + font-> /* max_bounds. */ descent + 2;   /*title height */
    move_curs = XCreateFontCursor(dpy, XC_fleur);
    resize_curs = XCreateFontCursor(dpy, XC_plus);
    pirate_curs = XCreateFontCursor(dpy, XC_pirate);
    tab_curs = XCreateFontCursor(dpy, XC_sb_h_double_arrow);
    xin = XineramaQueryExtension(dpy, &event_base, &error_base);
    if (xin) {
        xin = XineramaIsActive(dpy);
        xinerama = XineramaQueryScreens(dpy, &xinerama_num);
    }
    /* find out which modifier is NumLock - we'll use this when grabbing
     * every combination of modifiers we can think of */
    modmap = XGetModifierMapping(dpy);
    for (i = 0; i < 8; i++) {
        for (j = 0; j < modmap->max_keypermod; j++) {
            if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock)) {
                numlockmask = (1 << i);
#ifdef DEBUG
                fprintf(stderr, "setup_display() : XK_Num_Lock is (1<<0x%02x)\n", i);
#endif
            }
        }
    }
    XFreeModifiermap(modmap);
    /* set up GC parameters */
    gv.function = GXinvert;
    gv.subwindow_mode = IncludeInferiors;
    gv.line_width = 2;          /* opt_bw */
    gv.font = font->fid;

    /* set up root window attributes - same for each screen */
    attr.event_mask = ChildMask | PropertyChangeMask | EnterWindowMask | LeaveWindowMask
#ifdef COLOURMAP
        | ColormapChangeMask
#endif
        | ButtonMask;

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_fg, &fg, &dummy);
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_abg, &abg, &dummy);
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_bg, &bg, &dummy);
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_fc, &fc, &dummy);
    gv.foreground = fg.pixel;
    gv.function = GXcopy;
    string_gc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
    //gv.function = GXinvert;
    XChangeWindowAttributes(dpy, root, CWEventMask, &attr);
    /* Unfortunately grabbing AnyKey under Solaris seems not to work */
    /* XGrabKey(dpy, AnyKey, ControlMask|Mod1Mask, root, True, GrabModeAsync, GrabModeAsync); */
    /* So now I grab each and every one. */
    for (keysym = keys_to_grab; *keysym; keysym++) {
        grab_keysym(root, MOD_MASK, *keysym);
    }
    grab_keysym(root, KEY_NEXT_MASK, KEY_NEXT);

    /* scan all the windows on this screen */
#ifdef XDEBUG
    fprintf(stderr, "main:XQueryTree(); ");
#endif
    XQueryTree(dpy, root, &dw1, &dw2, &wins, &nwins);
#ifdef XDEBUG
    fprintf(stderr, "%d windows\n", nwins);
#endif
    for (j = 0; j < nwins; j++) {
        XGetWindowAttributes(dpy, wins[j], &winattr);
        if (!winattr.override_redirect && winattr.map_state == IsViewable)
            make_new_client(wins[j]);
    }
    XFree(wins);
    /*wrap xinerama for non xinerama setups */
    if (!xin) {
        xinerama = (XineramaScreenInfo *) xmalloc(sizeof(XineramaScreenInfo));
        xinerama[0].x_org = 0;
        xinerama[0].y_org = 0;
        xinerama[0].width = DisplayWidth(dpy, screen);
        xinerama[0].height = DisplayHeight(dpy, screen);
    }
}

void get_defaults()
{
    char *opt;
    if (!opt_font) {
        opt = XGetDefault(dpy, "yeahwm", "font");
        opt_font = opt ? opt : DEF_FONT;
    }
    if (!opt_fg) {
        opt = XGetDefault(dpy, "yeahwm", "foreground");
        opt_fg = opt ? opt : DEF_FG;
    }
    if (!opt_wlist) {
        opt = XGetDefault(dpy, "yeahwm", "tablessWindows");
        opt_wlist = opt ? opt : NULL;
    }
    if (!opt_bg) {
        opt = XGetDefault(dpy, "yeahwm", "background");
        opt_bg = opt ? opt : DEF_BG;
    }
    if (!opt_abg) {
        opt = XGetDefault(dpy, "yeahwm", "activeBackground");
        opt_abg = opt ? opt : DEF_ABG;
    }
    if (!opt_fc) {
        opt = XGetDefault(dpy, "yeahwm", "fixedColor");
        opt_fc = opt ? opt : DEF_FC;
    }
    if (!opt_vd) {
        opt = XGetDefault(dpy, "yeahwm", "virtualDesktops");
        opt_vd = opt ? atoi(opt) : DEF_VD;
    }
    if (!opt_bw) {
        opt = XGetDefault(dpy, "yeahwm", "borderWidth");
        opt_bw =  opt ? atoi(opt) : DEF_BW;
    }
    if (!opt_cmd1) {
        opt = XGetDefault(dpy, "yeahwm", "command1");
        opt_cmd1 = opt ? opt : DEF_CMD1;
    }
    if (!opt_cmd2) {
        opt = XGetDefault(dpy, "yeahwm", "command2");
        opt_cmd2 = opt ? opt : DEF_CMD2;
    }
    if (!opt_cmd3) {
        opt = XGetDefault(dpy, "yeahwm", "command3");
        opt_cmd3 = opt ? opt : DEF_CMD3;
    }
    if (!opt_snap) {
        opt = XGetDefault(dpy, "yeahwm", "windowSnap");
        opt_snap = opt ? atoi(opt) : DEF_SNAP;
    }
    if (!opt_sores) {
        opt = XGetDefault(dpy, "yeahwm", "solidResize");
        opt_sores = opt ? atoi(opt) : DEF_SORES;
    }

}
