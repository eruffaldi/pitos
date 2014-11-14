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
 * OVERVIEW:  This file allows the Windows version of the KVM
 *            to emulate a "Palm lookalike" graphical user interface
 *            on Windows.  This feature is provided simply as a 
 *            convenience for the user and to enable the debugging
 *            of the VM on a platform where good debugging tools
 *            are available.
 * AUTHOR:    Frank Yellin
 *            Ioi Lam and Rich Berlin converted to Windows
 * NOTE:      The functions in this file do the stack manipulation etc.
 *            but depend on a separate set of machine-specific functions
 *            to do the actual work.  See winGraphics.c for those functions;
 *            they don't want to be in this file because windows.h has
 *            definitions that conflict with those in global.h
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/* #define TRACE */
#include "global.h"


/*=========================================================================
 * Definitions and function prototypes
 *=======================================================================*/

#define FONT_HEIGHT 11

int winGetNextKVMEvent(KVMEventType *event, int forever);

/*=========================================================================
 * Functions
 *=======================================================================*/

bool_t
GetNextKVMEvent(KVMEventType *event, bool_t forever, ulong64 waitUntil)
{
    return winGetNextKVMEvent(event, forever);
}

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
 * FUNCTION:      getWidth(Ljava/lang/String;)I (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Return the width of a String in pixels.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a String object
 *   returns:     the width of the string in pixels
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_getWidth(void)
{
    INSTANCE string = (INSTANCE)topStack;
    SHORTARRAY thisArray = (SHORTARRAY)string->data[0].cellp;
    int        offset    = string->data[1].cell;
    int        length    = string->data[2].cell;
    int        width = 0;
    int        i;

    for (i = 0; i < length; i++) { 
        char c = (char)thisArray->sdata[offset + i];
	width += winCharWidth(c);
    }
    topStack = (cell)width;
}

/*=========================================================================
 * FUNCTION:      getHeight(Ljava/lang/String;)I (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Return the height of a String in pixels.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a String object
 *   returns:     the height of the string in pixels
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_getHeight(void)
{
    topStack = FONT_HEIGHT;
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
void Java_com_sun_kjava_Graphics_drawLine(void)
{
    int mode = popStack();
    int dstY = popStack();
    int dstX = popStack();
    int srcY = popStack();
    int srcX = popStack();

#ifdef TRACE
    fprintf(stderr, "drawLine(%d, %d, %d, %d, %d)\n",
	    srcX, srcY, dstX, dstY, mode);
#endif

    winDrawLine(srcX, srcY, dstX, dstY, mode);
}

/*=========================================================================
 * FUNCTION:      drawRectangle(IIIIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Draw or erase a rectangle. The action performed will depend
 *                on
 *                the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the upper left corner coordinates
 *                width,height - the width and height of the rectangle
 *                mode - an integer describing the type of drawing action
 *                cornerDiam - diameter or circle used to give rounded corners
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawRectangle(void) {
    int cornerDiam   = popStack();
    int mode         = popStack();
    int height       = popStack();
    int width        = popStack();
    int y            = popStack();
    int x            = popStack();

#ifdef TRACE
    fprintf(stderr, "drawRectangle(%d, %d, %d, %d, %d, %d)\n",
	    x, y, width, height, mode, cornerDiam);
#endif
    winFillRectangle(x, y, width, height, cornerDiam, mode);
}

/*=========================================================================
 * FUNCTION:      drawBorder(IIIIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Draw or erase a border around a rectangle. The action
 *                performed will depend on the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the upper left corner coordinates
 *                width,height - the width and height of the rectangle
 *                mode - an integer describing the type of drawing action
 *                borderType - the type of the border
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawBorder(void)
{
    int frameType = popStack();	/* SIMPLE = 01, RAISED = 0x0205 */
    int mode      = popStack();
    int height    = popStack();
    int width     = popStack();
    int y         = popStack();
    int x         = popStack();

    frameType = frameType;
#ifdef TRACE
    fprintf(stderr, "drawBorder(%d, %d, %d, %d, %d, %d)\n",
	    x, y, width, height, mode, frameType);
#endif
    winDrawRectangle(x-1, y-1, width+1, height+1, mode);
}

/*=========================================================================
 * FUNCTION:      drawString(Ljava/lang/String;III)I (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Draw or erase a string at a given position. The action
 *                performed will depend on the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  text - the String to draw
 *                left,top - the top left bound of the first character
 *                mode - an integer describing the type of drawing action
 *   returns:     the left parameter plus the width of the string in pixels
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawString(void)
{
    int mode = popStack();
    int y  = popStack();
    int x  = popStack();
    INSTANCE string = (INSTANCE)topStack;
    SHORTARRAY thisArray = (SHORTARRAY)string->data[0].cellp;
    int        offset    = string->data[1].cell;
    int        length    = string->data[2].cell;
    int        i;

    int startX = x;

#ifdef TRACE
    fprintf(stderr, "drawString(%d, %d, %d)\n",
	    x, y, mode);
#endif

    for (i = 0; i < length; i++) { 
        char c = (char)thisArray->sdata[offset + i];
        int width = winCharWidth(c);
        if (width > 0) {
            winDrawChar(c, x, y, mode);
            x += width;
        }
    }

    winRefreshScreen(startX, y, x - startX, FONT_HEIGHT);
    topStack = (cell)x;
}


/*=========================================================================
 * FUNCTION:      setDrawRegion(IIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Set the clipping rectangle.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the top left corner of the clipping rectangle
 *                width,height - the dimensions of the clipping rectangle
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_setDrawRegion(void)
{
    int height = popStack();
    int width =  popStack();
    int y =      popStack();
    int x =      popStack();

#ifdef TRACE
    fprintf(stderr, "setDrawRegion(%d, %d, %d, %d)\n",
	    x, y, width, height);
#endif
    winSetClip(x, y, width, height);
}

/*=========================================================================
 * FUNCTION:      resetDrawRegion()V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Reset the clipping rectangle to be the whole window.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_resetDrawRegion(void)
{
#ifdef TRACE
    fprintf(stderr, "resetDrawRegion()\n");
#endif
    winResetClip();
}

/*=========================================================================
 * FUNCTION:      copyRegion(IIIIIII)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Copy a region to another region (possibly the same location)
 * INTERFACE (operand stack manipulation):
 *   parameters:  srcX,srcY - the origin of the copy
 *                width,height - the dimensions of the region to be copied
 *                dstX,dstY - the destination of the copy
 *                mode - the copy mode to be used
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_copyRegion(void)
{
    int mode       = popStack(); 
    int dstY       = popStack();
    int dstX       = popStack();
    int height     = popStack();
    int width      = popStack();
    int y          = popStack();
    int x          = popStack();

#ifdef TRACE
    fprintf(stderr, "copyRegion(%d, %d, %d, %d, %d, %d, %d)\n",
	    x, y, width, height, dstX, dstY, mode);
#endif
    winCopyRegion(x, y, width, height, dstX, dstY, mode);
}

/*=========================================================================
 * FUNCTION:      drawBitmap(IILspotless/Bitmap;)V (STATIC)
 * CLASS:         spotless/Graphics
 * TYPE:          native function
 * OVERVIEW:      Draw a bitmap
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - top left corner of bitmap
 *                bitmap - the bitmap to be drawn
 *                pixels- the actual pixels in the bitmap
 *   returns:     <nothing>
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_drawBitmap(void)
{
    INSTANCE object  = (INSTANCE)popStack();
    int y            = popStack();
    int x            = popStack();

#ifdef TRACE
    fprintf(stderr, "drawBitmap(%d, %d)\n",
	    x, y);
#endif

    SHORTARRAY sarray = (SHORTARRAY)object->data[0].cellp;
    short *data  = sarray->sdata;
    int width    = data[0];
    int height   = data[1];
    int rowBytes = data[2];
    char *bitmap = (char*)&data[8];

	/* Use data[7] as an internal flag to indicate if byteswap and
	 * inversion have already been performed
	 */
	if (data[7] == 0) {
		/* convert bitmap to Windows format: byteswap and invert.
		 * silly Windows treats 1=white and 0=black.
		 */
		int totalBytes = rowBytes * height;
		int i;
		for (i = 0; i < totalBytes; i += 2) {
			int temp = bitmap[i];
			bitmap[i] = bitmap[i+1] ^ 0xFF;
			bitmap[i+1] = temp ^ 0xFF;
		}
		data[7] = 1;
	}

    winDrawMonoBitmap(x, y, width, height, rowBytes, bitmap);
}

/*=========================================================================
 * FUNCTION:      playSound(I)V (STATIC)
 * CLASS:         spotless.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Play a sound
 * INTERFACE (operand stack manipulation):
 *   parameters:  the code number of the Palm sound to play
 *   returns:     <nothing>
 *
 * NOTE: This method doesn't really belong in the Graphics class, but we
 *  don't want to create a new KVM class just for this one method.
 *=======================================================================*/
void Java_com_sun_kjava_Graphics_playSound(void) {
    popStack(); /* ignore this for now */
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
void 
Java_com_sun_kjava_Graphics_copyOffScreenRegion(void)
{
    int dstWin = popStack();
    int srcWin = popStack();
    int mode   = popStack();
    int dstY   = popStack();
    int dstX   = popStack();
    int height = popStack();
    int width  = popStack();
    int y      = popStack();
    int x      = popStack();
    
    winCopyRectangle(srcWin, dstWin, x, y, width, height, dstX, dstY, mode);
}

