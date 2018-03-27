#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "config.h"
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif
#include <X11/extensions/Xinerama.h>
#include <stdio.h>

/* readability stuff */
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

#define STICKY 0                /* Desktop number for sticky clients */
#define KEY_TO_VDESK( key ) ( ( key ) - XK_1 + 1 )
#define THEIGHT (theight * c->title) 
#define RAISE		1
#define NO_RAISE	0       /* for unhide() */
#define RESIZE		1
#define MOVE    	0
#define DEF_SORES       0
/* some coding shorthand */

#define ChildMask	(SubstructureRedirectMask|SubstructureNotifyMask)
#define ButtonMask	(ButtonPressMask|ButtonReleaseMask)
#define MouseMask	(ButtonMask|PointerMotionMask)

#define grab_pointer(w, mask, curs) \
	(XGrabPointer(dpy, w, False, mask, GrabModeAsync, GrabModeAsync, \
	None, curs, CurrentTime) == GrabSuccess)
#define grab_keysym(w, mask, keysym) \
	XGrabKey(dpy, XKeysymToKeycode(dpy, keysym), mask, w, True, \
			GrabModeAsync, GrabModeAsync); \
	XGrabKey(dpy, XKeysymToKeycode(dpy, keysym), LockMask|mask, w, True, \
			GrabModeAsync, GrabModeAsync); \
	if (numlockmask) { \
		XGrabKey(dpy, XKeysymToKeycode(dpy, keysym), numlockmask|mask, \
				w, True, GrabModeAsync, GrabModeAsync); \
		XGrabKey(dpy, XKeysymToKeycode(dpy, keysym), \
				numlockmask|LockMask|mask, w, True, \
				GrabModeAsync, GrabModeAsync); \
	}
#define grab_button(w, mask, button) \
	XGrabButton(dpy, button, mask, w, False, ButtonMask, \
			GrabModeAsync, GrabModeSync, None, None); \
	XGrabButton(dpy, button, LockMask|mask, w, False, ButtonMask, \
			GrabModeAsync, GrabModeSync, None, None); \
	if (numlockmask) { \
		XGrabButton(dpy, button, numlockmask|mask, w, False, \
				ButtonMask, GrabModeAsync, GrabModeSync, \
				None, None); \
		XGrabButton(dpy, button, numlockmask|LockMask|mask, w, False, \
				ButtonMask, GrabModeAsync, GrabModeSync, \
				None, None); \
	}
#define setmouse(w, x, y) \
	XWarpPointer(dpy, None, w, 0, 0, 0, 0, x, y)
#define gravitate(c) \
	change_gravity(c, 1)
#define ungravitate(c) \
	change_gravity(c, -1)

/* client structure */

typedef struct Client Client;

struct Client {
    Client *next;
    Window window;
    Window parent;
    Window tab;
    int tab_width;
    int tab_offset;
    /*  ScreenInfo *screen; */

#ifdef COLOURMAP
    Colormap cmap;
#endif
    XSizeHints *size;
    int ignore_unmap;
    int title;                  /*toggle titlebar */
    int shade;                  /*toggle shade */
    int x, y, width, height;
    int oldx, oldy, oldw, oldh; /* used when maximising */
    int border;
    char *name;
    int vdesk;

};

/* declarations for global variables in main.c */

extern Display *dpy;
extern int screen;
extern Window root;
extern GC string_gc;
XColor fg, abg, bg, fc;
extern Client *current;
extern Window initialising;
extern XFontStruct *font;
extern Client *head_client;
extern Atom xa_wm_state;
extern Atom xa_wm_change_state;
extern Atom xa_wm_protos;
extern Atom xa_wm_delete;
extern Atom xa_wm_cmapwins;
extern Cursor move_curs;
extern Cursor resize_curs;
extern Cursor pirate_curs;
extern Cursor tab_curs;
extern const char *opt_display;
extern const char *opt_wlist;
extern const char *opt_font;
extern const char *opt_fg;
extern const char *opt_bg;
extern const char *opt_cmd1;
extern const char *opt_cmd2;
extern const char *opt_cmd3;
extern const char **opt_key;
extern int opt_bw;
extern int opt_sores;
extern XineramaScreenInfo *xinerama;
extern int xinerama_num;
extern int xin;
extern const char *opt_fc;
extern int vdesk;
extern int opt_snap;

#ifdef SHAPE
extern int have_shape, shape_event;
#endif
extern int quitting;
#ifdef MWM_HINTS
extern Atom mwm_hints;
#endif
extern unsigned int numlockmask;

/* client.c */

Client *find_client(Window w);
int wm_state(Client * c);
void change_gravity(Client * c, int multiplier);
void remove_client(Client * c);
void send_config(Client * c);
void send_wm_delete(Client * c);
void set_wm_state(Client * c, int state);
void set_shape(Client * c);
void client_update_current(Client * c);

/* events.c */

void handle_key_event(XKeyEvent * e);
void handle_button_event(XButtonEvent * e);
/* static void handle_client_message(XClientMessageEvent *e); */
#ifdef COLOURMAP
void handle_colormap_change(XColormapEvent * e);
#endif
void handle_configure_request(XConfigureRequestEvent * e);
void handle_enter_event(XCrossingEvent * e);
void handle_leave_event(XCrossingEvent * e);
void handle_map_request(XMapRequestEvent * e);
void handle_property_change(XPropertyEvent * e);
void handle_unmap_event(XUnmapEvent * e);
void handle_expose_event(XExposeEvent * e);
#ifdef SHAPE
void handle_shape_event(XShapeEvent * e);
#endif

/* main.c */

int main(int argc, char *argv[]);
void scan_windows(void);
void setup_display(void);
int opt_vd;
int theight;

/* misc.c */
int handle_xerror(Display * dsply, XErrorEvent * e);
int ignore_xerror(Display * dsply, XErrorEvent * e);
void dump_clients(void);
void spawn(const char *cmd);
void handle_signal(int signo);
#ifdef DEBUG
void show_event(XEvent e);
#endif

/* new.c */

void init_position(Client * c);
void make_new_client(Window w);
void reparent(Client * c);

/* screen.c */

void drag(Client * c);
void draw_outline(Client * c);
void get_mouse_position(int *x, int *y);
void move(Client * c);
void recalculate_sweep(Client * c);
void resize(Client * c);
void maximise_vert(Client * c);
void maximise_horiz(Client * c);
void show_info(Client * c);
void remove_info(Client * c);
void sweep(Client * c);
void unhide(Client * c, int raise);
void next(void);
void toggle_shade(Client * c);
void toggle_title(Client * c);
void snap_client(Client * c, int mode);
void focus(Client * c);
void hide(Client * c);
void switch_vdesk(int v);
int xin_screen(void);
void select_window(void);
void check_tab_pos_size(Client * c);
void xosd_vdesk(void);
