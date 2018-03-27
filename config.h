#ifndef _CONFIG_H
#define _CONFIG_H


/* default settings */

#define DEF_FONT	"variable"
#define DEF_ABG		"steel blue"    /*background for active windows*/
#define DEF_BG		"grey50"        /*background for inactive windows*/
#define DEF_FG		"snow"          /*color for window title*/
#define DEF_BW		2               /*borderwidth*/
#define DEF_FC		"blue"          /*color for fixed windows*/
#define MINSIZE		15              /*minimal size size for windows*/
#define DEF_CMD1	"urxvt"         /*which terminal to spawn*/
#define DEF_CMD2	"/usr/bin/google-chrome"         /*specify a program */
#define DEF_CMD3	""              /*specify a program */
#define DEF_VD          1               /*initial workspace count*/
#define DEF_SNAP        10              /*proximity for window snapping*/
#define DEF_PWIDTH      10              /*width to be sensitive for window closing*/
/* #define DEF_TERM	"x-terminal-emulator"*/ /*debian users might want uncomment this one*/


/* You can save a few bytes if you know you won't need colour map support
(eg for 16 or more bit displays) */

#define COLOURMAP

/* keybindings */

#define KEY_NEXT	XK_Tab
#define KEY_CMD1	XK_Return
#define KEY_CMD2	XK_f
#define KEY_CMD3	XK_p
#define KEY_TOPLEFT	XK_n
#define KEY_TOPRIGHT	XK_r
#define KEY_BOTTOMLEFT	XK_comma
#define KEY_BOTTOMRIGHT	XK_period
#define KEY_LEFT	XK_h
#define KEY_RIGHT	XK_l
#define KEY_DOWN	XK_j
#define KEY_UP		XK_k
#define KEY_LOWER	XK_Down
#define KEY_RAISE	XK_Up
#define KEY_TITLE	XK_t
#define KEY_MAXVERT	XK_a
#define KEY_MAX_HOR	XK_b
#define KEY_MAX		XK_m
#define KEY_FIX		XK_x
#define KEY_PREVDESK	XK_Left
#define KEY_NEXTDESK	XK_Right
#define KEY_GROWVERT    XK_i
#define KEY_GROWHOR	XK_v
#define KEY_SHRINKVERT  XK_o
#define KEY_SHRINKHOR   XK_u
#define KEY_SHADE       XK_s
#define KEY_KILL	XK_q
#define KEY_XOSD_VDESK	XK_w
#define KEY_FIX_NEXT	XK_apostrophe
#define KEY_FIX_PREV	XK_semicolon
#define KEY_HALF	XK_slash

/*Modifier*/

/*uncomment Mod4Mask if you want to use your windows key for something useful*/

#define MOD_MASK        ControlMask | Mod1Mask
/*#define MOD_MASK        Mod4Mask*/

#define MOUSE_MOD     Mod1Mask
/*#define MOUSE_MOD       Mod4Mask*/

/*Modifier for cycling windows*/

#define KEY_NEXT_MASK    Mod1Mask
/*#define KEY_NEXT_MASK    ControlMask | Mod1Mask*/
/*#define KEY_NEXT_MASK    Mod4Mask*/

#endif
