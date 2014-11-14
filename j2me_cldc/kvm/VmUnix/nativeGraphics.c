/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 * 
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 * 
 */

/*=========================================================================
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Native function interface
 * FILE:      nativeGraphics.c
 * OVERVIEW:  This file allows the Solaris version of the KVM
 *            to emulate a "Palm lookalike" graphical user interface
 *            on Solaris.  This feature is provided simply as a 
 *            convenience for the user and to enable the debugging
 *            of the VM on a platform where good debugging tools
 *            are available.
 * AUTHOR:    Frank Yellin
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>

#define NeedFunctionPrototypes 1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/bitmaps/gray>

#if USE_XPM
#include <X11/xpm.h>
#include "blank.xpm"
#endif

#include <PalmFont0.h>
#include <Duke.h>
#include <cursorSource.h>
#include <cursorMask.h>


/*=========================================================================
 * Definitions needed only in this file
 *=======================================================================*/

#define PALM_TITLE_FONT "-*-*-medium-r-narrow-sans-20-*-*-*-*-*-*-*"
#define PALM_ALTERNATIVE_TITLE_FONT "-*-*-medium-*-*-*-20-*-*-*-*-*-*-*"
#define PALM_BACKGROUND_COLOR "Gray"

#define PAINT_WIDTH 160
#define PAINT_HEIGHT 160

/*  Constants used in the drawing functions. Should correlate with those */
/*  defined in spotless/Graphics.java. */
#define MODE_PLAIN  1
#define MODE_GRAY   2
#define MODE_ERASE  3
#define MODE_INVERT 4

typedef enum {
    PALM_power=0x01, 
    PALM_hard1 = 0x204,
    PALM_hard2 = 0x205,
    PALM_hard3 = 0x206,
    PALM_hard4 = 0x207,
    PALM_pageUp =  0x0B,
    PALM_pageDown = 0x0C
} PalmKeyType;

typedef struct  { 
	PalmKeyType button;
	XRectangle bounds;
} Key;

/*=========================================================================
 * Global variables
 *=======================================================================*/

extern int wantPalmSysKeys; /* true if Spotlet wants ALL Palm hard buttons */

/*=========================================================================
 * Local variables
 *=======================================================================*/

static bool_t windowSystemInitialized = FALSE;

/*  The X Display. */
static Display      *kauaiDisplay;
static Screen       *kauaiScreen;
unsigned long        kauaiForeground, kauaiBackground;

/*  The Window onto which all graphics should be written.
 *  if hasBackingStore is true, then paintDrawable is the same as paintWindow.
 *  if hasBackingStore is false, then paintDrawable is a separate pixmap that
 *     is the "background" of the paintWindow, and the paintWindow needs 
 *     to be cleared after each writing operation to refresh its background.
 */    
static Window       paintWindow;
static Drawable     paintDrawable;
static bool_t       hasBackingStore;

static Drawable     offScreenDrawable;

/*  Graphics contexts for writing onto the Graphics Window */
static GC            gcNormal, gcErase, gcInvert, gcGray;

/* A bitmap of the Palm PalmFont0 */
static Pixmap        palmFontBitmap;
/* The image to use when we're iconified */
static Pixmap        dukeIcon;

/*  An atom that the window manager sends to ask the process to close */
static Atom          deleteAtom;

const static Key PalmKeys[] = {
    { PALM_power,    {0, 281, 16, 22}},
    { PALM_pageUp,   {100, 280, 25, 12}},
    { PALM_pageDown, {100, 302, 25, 12}},
    { PALM_hard1,    {24, 277, 28, 29}},
    { PALM_hard2,    {63, 277, 28, 29}},
    { PALM_hard3,    {134, 277, 28, 29}},
    { PALM_hard4,    {173, 277, 28, 29}}
} ;



/*=========================================================================
 * Local functions
 *=======================================================================*/

static GC     modeToGC(int mode);
static Pixmap pixmapFromPilotData(short *data, int width, int height, int rowBytes);

static void   initImagesAndGCs();
static void   refreshPaintWindow();
static Pixmap getBackgroundPixmap(char *progName);
static Cursor getCursor();
static void   setWindowManagerHints(Window window, 
									char *windowName, char *iconName,
									int width, int height);

static int    kauaiXErrorHandler(Display *disp, XErrorEvent *err);
static int    kauaiXIOErrorHandler(Display *disp);

/*=========================================================================
 * Native functions of class spotless.Graphics
 *=======================================================================*/


/*=========================================================================
 * FUNCTION:      initialize()V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      initialize the graphics sub system.
 * INTERFACE (operand stack manipulation):
 *   parameters:  none
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_initialize()
{
    InitializeWindowSystem();
}

/*=========================================================================
 * FUNCTION:      drawLine(IIIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a line. The action performed will depend on
 *                the mode parameter
 * INTERFACE (operand stack manipulation):
 *   parameters:  srcX, srcY - the src point of the line
 *                dstX, dstY - the end point of the line
 *                mode - an integer describing the type of drawing action                
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawLine()
{
    Display *display = kauaiDisplay;
    int mode = popStack();
    int dstY = popStack();
    int dstX = popStack();
    int srcY = popStack();
    int srcX = popStack();
    GC  gc   = modeToGC(mode);
    XDrawLine(display, paintDrawable, gc, srcX, srcY, dstX, dstY);
    refreshPaintWindow();
}

/*=========================================================================
 * FUNCTION:      drawRectangle(IIIIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a rectangle. The action performed will depend on
 *                the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the upper left corner coordinates
 *                width,height - the width and height of the rectangle
 *                mode - an integer describing the type of drawing action
 *                cornerDiam - diameter or circle used to give rounded corners
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawRectangle() {
    Display *display = kauaiDisplay;
    Window  paint    = paintDrawable;
    int cornerDiam   = popStack();
    int mode         = popStack();
    int height       = popStack();
    int width        = popStack();
    int y            = popStack();
    int x            = popStack();
    GC gc            = modeToGC(mode);
    if (width <= 0 || height <= 0) { 
	    /*  do nothing */
    } else if (cornerDiam == 0) { 
        XFillRectangle(display, paint, gc, x, y, width, height); 
    } else { 
	long arcWidth = 2 * cornerDiam;
	long arcHeight = arcWidth;
	if (arcWidth > width) 
	    arcWidth = width;
	if (arcHeight > height) 
	    arcHeight = height;
	if (arcHeight == height && arcWidth == width) { 
	    /*  We're drawing an oval or a circle. */
	    XFillArc(display, paint, gc, x, y, width, height, 0, 360 * 64);
	} else { 
	    int halfAW = arcWidth >> 1;
	    int halfAH = arcHeight >> 1;
	    XArc arc[4];
	    XRectangle rect[3];
	    int i;
	    for (i = 0; i < 4; i++) {
		arc[i].x = (i == 1 || i == 2) ? x : x + width - arcWidth;
		arc[i].y = (i == 0 || i == 1) ? y : y + height - arcHeight;
		arc[i].width = arcWidth;
		arc[i].height = arcHeight;
		arc[i].angle1 = i * 90 * 64;
		arc[i].angle2 = 90 * 64;
	    }
	    XFillArcs(display, paint, gc, arc, 4);
	    /* The center vertical piece */
	    rect[0].x = (x + halfAW);  	 rect[0].width = width - arcWidth; 
	    rect[0].y = y;               rect[0].height = height;
	    /* The notch on the left */
	    rect[1].x = x;               rect[1].width = halfAW;
	    rect[1].y = y + halfAH   ;   rect[1].height = height - arcHeight;
	    /* The notch on the right */
	    rect[2].x = x + width - halfAW;  rect[2].width = halfAW;
	    rect[2].y = y + halfAH;      rect[2].height = height - arcHeight;
	    XFillRectangles(display, paint, gc, rect, 3);
	}
    }
    refreshPaintWindow();
}

/*=========================================================================
 * FUNCTION:      drawBorder(IIIIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a border around a rectangle. The action
 *                performed will depend on the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the upper left corner coordinates
 *                width,height - the width and height of the rectangle
 *                mode - an integer describing the type of drawing action
 *                borderType - the type of the border
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawBorder()
{
    Display *display = kauaiDisplay;
    int frameType = popStack();	/* SIMPLE = 01, RAISED = 0x0205 */
    int mode       = popStack();
    int height     = popStack();
    int width      = popStack();
    int y          = popStack();
    int x          = popStack();
    frameType = frameType;
    XDrawRectangle(display, paintDrawable, modeToGC(mode), 
		            x-1, y-1, width+1, height+1);
    refreshPaintWindow();
}

/*=========================================================================
 * FUNCTION:      drawString(Ljava/lang/String;III)I (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a string at a given position. The action
 *                performed will depend on the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  text - the String to draw
 *                left,top - the top left bound of the first character
 *                mode - an integer describing the type of drawing action
 *   returns:     the left parameter plus the width of the string in pixels
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawString()
{
    Display *display = kauaiDisplay;
    int mode = popStack();
    int y  = popStack();
    int x  = popStack();
    INSTANCE string = (INSTANCE)topStack;
    SHORTARRAY thisArray = (SHORTARRAY)string->data[0].cellp;
    int        offset    = string->data[1].cell;
    int        length    = string->data[2].cell;
    int        i;
    GC         gc = modeToGC(mode);

    for (i = 0; i < length; i++) { 
	char c = (char)thisArray->sdata[offset + i];
	int start = PalmFont0_widths[(int)c];
	int end = PalmFont0_widths[(int)(c + 1)];
	if (end > start) { 
	    XCopyPlane(display, palmFontBitmap, paintDrawable, gc,
		       start, 0, end-start, PalmFont0_height, x, y, 1);
	    x += (end - start);
	}
    }
    topStack = (cell)x;
    refreshPaintWindow();
}

/*=========================================================================
 * FUNCTION:      getWidth(Ljava/lang/String;)I (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Return the width of a String in pixels.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a String object
 *   returns:     the width of the string in pixels
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_getWidth()
{
    INSTANCE string = (INSTANCE)topStack;
    SHORTARRAY thisArray = (SHORTARRAY)string->data[0].cellp;
    int        offset    = string->data[1].cell;
    int        length    = string->data[2].cell;
    int        width = 0;
    int        i;
    for (i = 0; i < length; i++) { 
	char c = thisArray->sdata[offset + i];
	int start = PalmFont0_widths[(int)c];
	int end = PalmFont0_widths[(int)(c + 1)];
	if (end > start) {
	    width += (end - start);
	}
    }
    topStack = (cell)width;
}

/*=========================================================================
 * FUNCTION:      getHeight(Ljava/lang/String;)I (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Return the height of a String in pixels.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a String object
 *   returns:     the height of the string in pixels
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_getHeight()
{
    topStack = (cell)PalmFont0_height;
}

/*=========================================================================
 * FUNCTION:      setDrawRegion(IIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Set the clipping rectangle.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the top left corner of the clipping rectangle
 *                width,height - the dimensions of the clipping rectangle
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_setDrawRegion()
{
    Display *display = kauaiDisplay;
    XRectangle rect;
    rect.height = popStack();
    rect.width =  popStack();
    rect.y =      popStack();
    rect.x =      popStack();
    XSetClipRectangles(display, gcNormal, 0, 0, &rect, 1, Unsorted);
    XSetClipRectangles(display, gcErase,  0, 0, &rect, 1, Unsorted);
    XSetClipRectangles(display, gcInvert, 0, 0, &rect, 1, Unsorted);
    XSetClipRectangles(display, gcGray,   0, 0, &rect, 1, Unsorted);
}

/*=========================================================================
 * FUNCTION:      resetDrawRegion()V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Reset the clipping rectangle to be the whole window.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_resetDrawRegion()
{
    Display       *display = kauaiDisplay;
    XSetClipMask(display, gcNormal, None);
    XSetClipMask(display, gcErase, None);
    XSetClipMask(display, gcInvert, None);
    XSetClipMask(display, gcGray, None);
}



/*=========================================================================
 * FUNCTION:      copyRegion(IIIIIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Copy a region to another region (possibly the same location)
 * INTERFACE (operand stack manipulation):
 *   parameters:  srcX,srcY - the origin of the copy
 *                width,height - the dimensions of the region to be copied
 *                dstX,dstY - the destination of the copy
 *                mode - the copy mode to be used
 *   returns:     <nothing>
 *=======================================================================*/
    
#define MODE_OVERWRITE 0
#define MODE_AND 1
#define MODE_AND_NOT 2
#define MODE_XOR 3
#define MODE_OR 4
#define MODE_NOT 5


static char regionMode[] = { GXcopy, GXand, GXandInverted, 
			     GXxor, GXor, GXcopyInverted };

void Java_com_sun_kjava_Graphics_copyRegion()
{
    Display *display = kauaiDisplay;
    int mode       = popStack(); 
    int dstY       = popStack();
    int dstX       = popStack();
    int height     = popStack();
    int width      = popStack();
    int y          = popStack();
    int x          = popStack();
    int newMode    = (0 <= mode && mode <= 5) ? regionMode[mode] : GXcopy;
    
    /* Rather than allocating special GC's for this, let's just use
     * gcNormal. This operation doesn't happen that often. */
    XSetFunction(display, gcNormal, newMode);
    XCopyArea(display, paintDrawable, paintDrawable, gcNormal, 
	          x, y, width, height, dstX, dstY);
    XSetFunction(display, gcNormal, GXcopy);
    refreshPaintWindow();
}

/*=========================================================================
 * FUNCTION:      copyOffScreenRegion(IIIIIIIII)V (STATIC)
 * CLASS:         spotless.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Copy a region to another region (possibly the same location,
 *                  possibly between windows)
 * INTERFACE (operand stack manipulation):
 *   parameters:  srcX,srcY - the origin of the copy
 *                width,height - the dimensions of the region to be copied
 *                dstX,dstY - the destination of the copy
 *                mode - the copy mode to be used
 *                scrWin,dstWin - the windows to copy to/from
 *   returns:     <nothing>
 *=======================================================================*/

#define ONSCREEN_WINDOW   0
#define OFFSCREEN_WINDOW  1
	
void 
Java_com_sun_kjava_Graphics_copyOffScreenRegion(void)
{
    Display *display = kauaiDisplay;

    Drawable dstDrawable = 
	(popStack() == ONSCREEN_WINDOW) ? paintDrawable : offScreenDrawable;
    Drawable srcDrawable =
	(popStack() == ONSCREEN_WINDOW) ? paintDrawable : offScreenDrawable;
    int mode       = popStack(); 
    int dstY       = popStack();
    int dstX       = popStack();
    int height     = popStack();
    int width      = popStack();
    int y          = popStack();
    int x          = popStack();

    int newMode    = (0 <= mode && mode <= 5) ? regionMode[mode] : GXcopy;
    
    /*  Rather than allocating special GC's for this, let's just use gcNormal.
     *  This operation doesn't happen that often. 
     */
    XSetFunction(display, gcNormal, newMode);
    XCopyArea(display, srcDrawable, dstDrawable, gcNormal, 
	      x, y, width, height, dstX, dstY);
    XSetFunction(display, gcNormal, GXcopy);
    refreshPaintWindow();
}

void 
Java_com_sun_kjava_Graphics_playSound(void)
{ 
    oneLess;
}


/*=========================================================================
 * FUNCTION:      drawBitmap(IILspotless/Bitmap;)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw a bitmap
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - top left corner of bitmap
 *                bitmap - the bitmap to be drawn
 *                pixels- the actual pixels in the bitmap
 *   returns:     <nothing>
 *=======================================================================*/


void Java_com_sun_kjava_Graphics_drawBitmap()
{
    Display *display = kauaiDisplay;
    INSTANCE object  = (INSTANCE)popStack();
    int y            = popStack();
    int x            = popStack();

    SHORTARRAY sarray = (SHORTARRAY)object->data[0].cellp;
    short *bitmap    = sarray->sdata;
    int width        = bitmap[0];
    int height       = bitmap[1];
    int rowBytes     = bitmap[2];
    short *data      = &bitmap[8]; 

    Pixmap p = pixmapFromPilotData(data, width, height, rowBytes);
    XCopyPlane(display, p, paintDrawable, gcNormal, 0, 0, width, height, x, y, 1);
    refreshPaintWindow();
    XFreePixmap(display, p);
}


static Pixmap 
pixmapFromPilotData(short *data, int width, int height, int rowBytes)
{
    Display *display = kauaiDisplay;
    Window rootWindow = RootWindowOfScreen(kauaiScreen);
    
    Pixmap p = XCreatePixmap(display, rootWindow, width, height, 1);
    GC gc = XCreateGC(display, p, (unsigned long) 0, (XGCValues *) 0);
    XImage ximage;

    ximage.height = height;
    ximage.width = width;
    ximage.depth = 1;
    ximage.xoffset = 0;
    ximage.format = ZPixmap;
    ximage.data = (char *)data;
    ximage.byte_order = ximage.bitmap_bit_order = MSBFirst;
    ximage.bitmap_unit = ximage.bitmap_pad = 16;
    ximage.bytes_per_line = rowBytes;
    XPutImage(display, p, gc, &ximage, 0, 0, 0, 0, width, height);
    XFreeGC(display, gc);
    return p;
}


/*=========================================================================
 * FUNCTION:      modeToGC()
 * TYPE:          private nativeX operation
 * OVERVIEW:      Given a SpotLess graphics mode, it returns
 *                the X graphics context that most nearly does the same
 *                thing
 * INTERFACE:
 *   parameters:  A SpotLess graphics mode.
 *   returns:     A graphics context
 * COMMENTS:      <none>

 *=======================================================================*/

static GC 
modeToGC(int mode) { 
    switch (mode) {
        default:          return gcNormal;
        case MODE_ERASE:  return gcErase;
        case MODE_GRAY:   return gcGray;
        case MODE_INVERT: return gcInvert;
    }
}

static void 
refreshPaintWindow() { 
    if (!hasBackingStore)
	XClearWindow(kauaiDisplay, paintWindow);
}

static bool_t hasFocus = FALSE;
static bool_t isMapped = FALSE;


/*=========================================================================
 * FUNCTION:      Get the next event
 * TYPE:          event handler
 * OVERVIEW:      Wait for an external event.
 * INTERFACE 
 *   parameters:  evt:   a KVMEventType structure into which to stuff the
 *                         the return event.
 *                forever: if TRUE, this function should loop forever until
 *                         it gets an event.  If FALSE, use the waitUntil
 *                         argument.
 *                waitUntil: The time (as returned by CurrentTime_md()) at
 *                         which this function should give up waiting for
 *                         events.
 * 
 *   returns:     TRUE if an event was found, false otherwise.
 *=======================================================================*/


bool_t
GetNextKVMEvent(KVMEventType *evt, bool_t forever, ulong64 waitUntil) 
{
    Display *display = kauaiDisplay;
    XEvent event;

    /*  Needed to convert Key events into actual characters. */
    static char buff[10];
    static short buff_index = 0;
    static short buff_count = 0;
    static XComposeStatus status;

    for (;;) { 
        if (buff_index < buff_count) { 
	    /*  There are characters pending from a KeyPress event. */
	    evt->type = keyDownKVMEvent;
	    evt->chr = buff[buff_index++];
	    return TRUE;
	}

	if (!forever && !XEventsQueued(display, QueuedAfterFlush)) { 
	    ulong64 now = CurrentTime_md();
	    if (waitUntil < now) {
		return FALSE;
	    } else { 
		int fd = ConnectionNumber(display);
		ulong64 delta = waitUntil - now;
		struct timeval timeout;
		fd_set readfds;
		
		/* Set the seconds and microseconds */
		timeout.tv_sec = delta / 1000;
		timeout.tv_usec = (delta % 1000) * 1000;
		
		/* Set the single bit in the bit set */
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		select(fd + 1, &readfds, NULL, NULL, &timeout);
		if (!XEventsQueued(display, QueuedAfterFlush)) { 
		    return FALSE;
		}
	    }
	}

	XNextEvent(display, &event);
	switch (event.type) { 
			
	case ButtonPress:
	    evt->type = penDownKVMEvent;
	    goto button;
	case ButtonRelease:
	    evt->type = penUpKVMEvent;
	    goto button;
	case MotionNotify: 
	    evt->type = penMoveKVMEvent;
	button: { 
		int x = event.xbutton.x;
		int y = event.xbutton.y;
		if (event.xbutton.window == paintWindow) {
		    evt->screenX = x;
		    evt->screenY = y;
		    return TRUE;
		} else { 
		    /* Did the user tap one of the buttons? */
		    int i;
                    int button;
		    for (i = 0; i < XtNumber(PalmKeys); i++) { 
			const XRectangle *r = &PalmKeys[i].bounds;
			if (x >= r->x && x < r->x + r->width && 
			    y >= r->y && y < r->y + r->height) { 
                            button = PalmKeys[i].button;
                            if (button == PALM_pageUp || button == PALM_pageDown
				|| wantPalmSysKeys) {
				evt->type = keyDownKVMEvent;
				evt->chr  = button;
				return TRUE;
                            }
			    if (button == PALM_power) {
				evt->type = appStopKVMEvent;
				return TRUE;
			    }
			}
		    }
		}
	    }
	    break;
	    
	case KeyPress:
	    /*  This is more complicated than it has any right to be. */
	    /*  There may be multiple characters or no characters. */
	    buff_count = XLookupString(&event.xkey, buff, sizeof(buff), 
				       NULL, &status);
	    buff_index = 0;
	    /*  The next iteration through the loop will find the chars */
	    break;
			
	case ClientMessage:
	    /*  Look for a message saying to quit */
	    if (event.xclient.data.l[0] == deleteAtom) { 
		evt->type = appStopKVMEvent;
		return TRUE;
	    }
	    break;
	    
	case MappingNotify:
	    /*  Every application has to do this! */
	    XRefreshKeyboardMapping(&event.xmapping);
	    break;
			
	    /* The following are experimental.  We may want to stop the machine
	     * if it doesn't have the focus 
	     */
	case MapNotify:
	    isMapped = TRUE;   break;
	case UnmapNotify:
	    isMapped = FALSE;  break;
	case FocusIn:
	    hasFocus = TRUE;   break;
	case FocusOut:
	    hasFocus = FALSE;  break;
	}
    }
}

typedef struct kauaiApplicationData { 
	Position x;
	Position y;
	Pixel foreground;
	Pixel background;
	char *title;
	struct FontStruct *font;
} kauaiApplicationData;


static XtResource resources[] = { 
	{ XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel), 
	      XtOffset(kauaiApplicationData*, foreground), XtRString, "WHITE" },
	{ XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel), 
	      XtOffset(kauaiApplicationData*, background), XtRString, "BLACK" },
	{ XtNx, XtCX, XtRPosition, sizeof(Position), 
	      XtOffset(kauaiApplicationData*, x), XtRImmediate, (void *)100 },
	{ XtNy, XtCY, XtRPosition, sizeof(Position), 
	      XtOffset(kauaiApplicationData*, y), XtRImmediate, (void *)100 },
	{ XtNtitle, XtCTitle, XtRString, sizeof(char *), 
	      XtOffset(kauaiApplicationData*, title), XtRImmediate, 0 },
	{ XtNfont, XtCFont, XtRFont, sizeof(struct FontStruct *), 
	      XtOffset(kauaiApplicationData*, font), XtRImmediate, 0 },
};

void 
InitializeWindowSystem() {
    unsigned int width = 221, height = 337;
    int argc = 0;
    char *argv = "";
    
    Widget shellWidget;
    XtAppContext appContext;
    Window outerWindow;
    char *progName;
    Display *display;
    Pixmap casePixmap;
    kauaiApplicationData appData;

    XSetErrorHandler(kauaiXErrorHandler);
    XSetIOErrorHandler(kauaiXIOErrorHandler);

    shellWidget = XtOpenApplication(&appContext, "kvm", NULL, 0, &argc, &argv,
				    NULL, applicationShellWidgetClass, 
				    NULL, 0);

    kauaiDisplay = display = XtDisplay(shellWidget);
    kauaiScreen = DefaultScreenOfDisplay(display);
	
    XtGetApplicationResources(shellWidget, &appData, 
			      resources, XtNumber(resources), NULL, 0);
	
    kauaiForeground = appData.foreground;
    kauaiBackground = appData.background;
    if (appData.title != NULL) { 
	progName = appData.title;
    } else { 
	progName = "kvm";
    }

    if (getenv("DEBUG_KAUAI_SCREEN")) { 
	XSynchronize(display, 1);
    }

    /*  Create the graphics contexts */
    initImagesAndGCs();

    /* Create the "outer" window */
    outerWindow = XCreateSimpleWindow(display, RootWindowOfScreen(kauaiScreen),
				      appData.x, appData.y, width, height,
									  0, kauaiBackground, kauaiForeground);
    XSelectInput(display, outerWindow, ButtonPressMask | KeyPressMask 
		 | StructureNotifyMask | FocusChangeMask);
    XDefineCursor(display, outerWindow, getCursor());
    casePixmap = getBackgroundPixmap(progName);
    XSetWindowBackgroundPixmap(display, outerWindow, casePixmap);
    XFreePixmap(display, casePixmap);


    /* Create the "paint" window */
    paintWindow = XCreateSimpleWindow(display, outerWindow, 
				      32, 33, PAINT_WIDTH, PAINT_HEIGHT, 
				      0, kauaiBackground, kauaiForeground);
    XSelectInput(display, paintWindow, ButtonPressMask | ButtonReleaseMask | 
		 ButtonMotionMask | KeyPressMask);
    hasBackingStore = (DoesBackingStore(kauaiScreen) == Always);
    if (hasBackingStore) { 
	/* we can scribble directly on the window, without worrying about
	 * expose events or anything like that.  */
	XSetWindowAttributes attributes;
	attributes.backing_store = Always;
	XChangeWindowAttributes (display, paintWindow, CWBackingStore, 
				 &attributes);
	paintDrawable = paintWindow;
	XClearWindow(display, paintWindow);
    } else { 
	/* We need a separate bitmap to serve as the drawable.  We write
	 * to the bitmap, and let it serve as the background for the window.
	 */
	paintDrawable = XCreatePixmap(display, paintWindow, 
				      PAINT_WIDTH, PAINT_HEIGHT,
				      DefaultDepthOfScreen(kauaiScreen));
	XDrawRectangle(display, paintDrawable, gcNormal, 0, 0,
		       PAINT_WIDTH, PAINT_HEIGHT);
	XDrawRectangle(display, paintDrawable, gcErase, 10, 10, 20, 20);
	XSetWindowBackgroundPixmap(display, paintWindow, paintDrawable);
    } 
    
    offScreenDrawable = XCreatePixmap(display, paintWindow, 
				      PAINT_WIDTH, PAINT_HEIGHT,
				      DefaultDepthOfScreen(kauaiScreen));
    
    /*  Let the window manager tell us about close events */
    deleteAtom = XInternAtom(display, "WM_DELETE_WINDOW", FALSE);
    XSetWMProtocols(display, outerWindow, &deleteAtom, 1);

    /*  Indicate that we don't want to be resized. */
    setWindowManagerHints(outerWindow, " ", progName, width, height);

    /*  Make sure that everything is visible     */
    XMapRaised(display, outerWindow);
    XMapRaised(display, paintWindow);
    /*    XMapSubwindows(display, outerWindow); */
    
    /* erase screen */
    XFillRectangle(display, paintDrawable, modeToGC(MODE_ERASE),
		   0, 0, PAINT_WIDTH, PAINT_HEIGHT); 
    XFlush(display);
    windowSystemInitialized = TRUE;
}


void
FinalizeWindowSystem(void) {
    if (windowSystemInitialized) {
	/*  Delete everything that we've allocated */
	Display *display = kauaiDisplay;
	XFreeGC(display, gcNormal);
	XFreeGC(display, gcErase);
	XFreeGC(display, gcInvert);
	XFreeGC(display, gcGray);
	XFreePixmap(display, palmFontBitmap);
	XFreePixmap(display, dukeIcon);
	if (!hasBackingStore) { 
	    XFreePixmap(display, paintDrawable);
	}
	XFreePixmap(display, offScreenDrawable);
	XCloseDisplay(display);
	windowSystemInitialized = FALSE;
    }
}


static void
initImagesAndGCs() 
{
    Display *display = kauaiDisplay;
    Window rootWindow = RootWindowOfScreen(kauaiScreen);
    unsigned long fg = kauaiForeground;
    unsigned long bg = kauaiBackground;

    const int GCbase = 
	GCForeground | GCBackground | GCGraphicsExposures | GCFunction;
    static Pixmap grayBitmap;
    XGCValues vals;

    grayBitmap = XCreateBitmapFromData(display, rootWindow, 
				       (char *)gray_bits, 
				       gray_width, gray_height);
    vals.foreground = fg;
    vals.background = bg;
    vals.graphics_exposures = FALSE;
    vals.function = GXcopy;
    gcNormal = XCreateGC(display, rootWindow, GCbase, &vals);

    vals.fill_style = FillOpaqueStippled;
    vals.stipple = grayBitmap;
    vals.function = GXcopy;
    gcGray = XCreateGC(display, rootWindow, 
		       GCbase | GCStipple | GCFillStyle, &vals);
    
    vals.function = GXxor;
    vals.foreground = fg ^ bg;
    vals.background = bg;
    gcInvert = XCreateGC(display, rootWindow, GCbase, &vals);

    vals.function = GXcopy;
    vals.foreground = bg;
    vals.background = bg;
    gcErase = XCreateGC(display, rootWindow, GCbase, &vals);

    XFreePixmap(display, grayBitmap);

    /* Create a pixmap containing the Palm font image */
    palmFontBitmap = XCreateBitmapFromData(display, rootWindow, 
					   (char *)PalmFont0_bits, 
					   PalmFont0_width, PalmFont0_height);
    /* Create a pixmap containing our favorite little fellow */
    dukeIcon = XCreateBitmapFromData(display, rootWindow, 
				     (char *)Duke_bits, 
				     Duke_width, Duke_height);
}


#ifndef USE_XPM
#  define USE_XPM 0
#endif

static 
Pixmap getBackgroundPixmap(char *name)
{
    Display *display = kauaiDisplay;
    Window rootWindow = RootWindowOfScreen(kauaiScreen);
    int nameLength = strlen(name);
    Pixmap pixmap = 0;

#if USE_XPM
    if (USE_XPM) {
	int status;
	/*  User has supplied a frame.  Let's try and use it. */
	XpmAttributes case_attributes;	
	case_attributes.valuemask = XpmReturnPixels | XpmCloseness | XpmSize;
	case_attributes.closeness = 40000; 
	status = XpmCreatePixmapFromData(display, rootWindow, blank_xpm, 
					 &pixmap, NULL, &case_attributes);
	if (status >= 0) { 
	} else { 
	    pixmap = (Pixmap)NULL;
	}
    }
#endif
    if (!USE_XPM || (pixmap == (Pixmap)NULL)) { 
	/*  Create a gray background with some buttons glued on. */
	int depth = DefaultDepthOfScreen(kauaiScreen);
	unsigned long fg = kauaiForeground;
	Colormap colormap = DefaultColormapOfScreen(kauaiScreen);
	XColor palmColor, junk;
	unsigned int width = 221, height = 337;
	int x_offset = (width - PAINT_WIDTH)/2 + 2;
	int y_offset = 33;
	int i;
	long palmColorPixel;
	
	XAllocNamedColor(display, colormap, PALM_BACKGROUND_COLOR, 
			 &palmColor, &junk);
	palmColorPixel = palmColor.pixel;
	/*  Create a pixmap in the default color */
	pixmap = XCreatePixmap(display, rootWindow, width, height, depth);
	/*  Set the color */
	XSetForeground(display, gcNormal, palmColorPixel);
	XFillRectangle(display, pixmap, gcNormal, 0, 0, width, height);
	XSetForeground(display, gcNormal, fg);
	
	for (i = 0; i < XtNumber(PalmKeys); i++) { 
	    int x = PalmKeys[i].bounds.x;
	    int y = PalmKeys[i].bounds.y;
	    int width = PalmKeys[i].bounds.width;
	    int height = PalmKeys[i].bounds.height;
	    if (i < 3) { 
		XFillRectangle(display, pixmap, gcNormal, x, y, width, height);
		XSetForeground(display, gcNormal, palmColorPixel);
		XFillRectangle(display, pixmap, gcNormal, 
			       x+2, y+2, width-4, height-4);
		XSetForeground(display, gcNormal, fg);
		XDrawRectangle(display, pixmap, gcNormal, 
			       x+4, y+4, width-9, height-9);
	    } else { 
		XFillArc(display, pixmap, gcNormal, x, y, width, height, 
			 0, 360*64);
		XSetForeground(display, gcNormal, palmColorPixel);
		XFillArc(display, pixmap, gcNormal, 
			 x+2, y+2, width-4, height-4, 0, 360*64);
		XSetForeground(display, gcNormal, fg);
		XDrawArc(display, pixmap, gcNormal, 
			 x+4, y+4, width-8, height-8, 0, 360*64);
	    }
	}
	
	XSetForeground(display, gcNormal, fg);
	/* Draw a box around the graphics pixmap */
	XFillRectangle(display, pixmap, gcNormal, x_offset - 2, y_offset - 2, 
		       PAINT_WIDTH + 4, PAINT_HEIGHT + 4);
	
	/* Draw a box around the silked screen area */
	XDrawRectangle(display, pixmap, gcNormal, 25, 197, width-50, 259-197);
	/* Draw a box around the graffiti area */
	XDrawRectangle(display, pixmap, gcNormal, 61, 202, width-122, 254-202);
	
	/* Bad renditions of the silk screen buttons */
	XDrawArc(display, pixmap, gcNormal,  43-10, 212-10, 20, 20, 0, 360*64);
	XDrawArc(display, pixmap, gcNormal, 180-10, 212-10, 20, 20, 0, 360*64);
	XDrawArc(display, pixmap, gcNormal,  43-10, 241-10, 20, 20, 0, 360*64);
	XDrawArc(display, pixmap, gcNormal, 180-10, 241-10, 20, 20, 0, 360*64);
    }

    /* Draw in the title */
    { 
	Font font = 0;
	char **fontList;
	int fontCount;
	
	fontList = XListFonts(display, PALM_TITLE_FONT, 1, &fontCount);
	if (fontList != NULL) { 
	    font = XLoadFont(display, PALM_TITLE_FONT);
	} else { 
	    fontList = XListFonts(display, PALM_ALTERNATIVE_TITLE_FONT, 1, 
				  &fontCount);
	    if (fontList != NULL) { 
		font = XLoadFont(display, PALM_ALTERNATIVE_TITLE_FONT);
	    } 
	}
	if (fontList != NULL) {
	    XFreeFontNames(fontList);
	}
	
	if (font != 0) { 
	    XSetFont(display, gcNormal, font);
	    XSetForeground(display, gcNormal, WhitePixelOfScreen(kauaiScreen));
	    if (strcmp(name + nameLength - 2, "_g") == 0) {
		nameLength -= 2;
	    }
	    XDrawString(display, pixmap, gcNormal, 32, 20, name, nameLength);
	    XSetForeground(display, gcNormal, kauaiForeground);
	    XUnloadFont(display, font);
	}
    }
    return pixmap;
}

static Cursor
    getCursor() 
{ 
    Display *display = kauaiDisplay;
    Window rootWindow = RootWindowOfScreen(kauaiScreen);
    XColor foreground, background;
    Cursor cursor;
    
    Pixmap cursorSource = 
	XCreateBitmapFromData(display, rootWindow, 
			      (char *)cursorSource_bits, 
			      cursorSource_width, cursorSource_height);
    Pixmap cursorMask = 
	XCreateBitmapFromData(display, rootWindow, (char *)cursorMask_bits, 
			      cursorMask_width, cursorMask_height);
    foreground.red = foreground.blue = foreground.green = 0;
    background.red = background.blue = background.green = 65535;
    cursor = XCreatePixmapCursor(display, cursorSource, cursorMask, 
				 &foreground, &background,
				 cursorSource_HotX, cursorSource_HotY);
    XFreePixmap(display, cursorSource);
    XFreePixmap(display, cursorMask);
    return cursor;
}


static void
    setWindowManagerHints(Window window, char *windowName, char *iconName, 
			  int width, int height)
{ 
    Display *display = kauaiDisplay;
    XSizeHints *hints = XAllocSizeHints();
    XWMHints *wmhints = XAllocWMHints();
    
    XSetIconName(display, window,   iconName);
    XStoreName(display, window, windowName);
    
    /* We have an exact width and height.  Do not resize. */
    hints->min_width = hints->max_width = hints->width = width;
    hints->min_height = hints->max_height = hints->height = height;
    hints->flags = PSize | PMaxSize | PMinSize | PPosition;
    XSetWMNormalHints(display, window, hints); 
    
    wmhints->icon_pixmap = dukeIcon;
    wmhints->input = True;
    wmhints->flags = InputHint | IconPixmapHint; 
    
    XSetWMHints(display, window, wmhints); 
    
    XFree(hints);
    XFree(wmhints);
}



static int
    kauaiXErrorHandler(Display *disp, XErrorEvent *err)
{
    char msg[128];
    char buf[128];
    XGetErrorText(disp, err->error_code, msg, sizeof(msg));
    fprintf(stderr, "Xerror %s\n", msg);
    sprintf(buf, "%d", err->request_code);
    XGetErrorDatabaseText(disp, "XRequest", buf, "Unknown", msg, sizeof(msg));
    fprintf(stderr, "Major opcode %d (%s)\n", err->request_code, msg);
    if (err->request_code > 128) {
	fprintf(stderr, "Minor opcode %d\n", err->minor_code);
    }
    return 0;
}

static int
    kauaiXIOErrorHandler(Display *disp)
{
    fprintf(stderr, "X I/O Error\n");
    return 0;
}
