/*
 *  Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 * 
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

package com.sun.kjava;

/**
 * This class contains various methods for drawing on a display. The coordinate
 * system used is such that the points along horizontal axis increase in value
 * from left to right and point along the vertical axis increase in value from
 * top to bottom.
 */

public class Graphics {

/*==========================================================
/* Constants used as parameters for functions in this class.
/*========================================================*/

    /**
     * Initialize the graphics sub system
     */
    private static native void initialize();
    static {
	initialize();
    }

	/**
	 * The single instance of this singleton class.
	 */
	private static Graphics instance = new Graphics(); 

	/** Constant for a plain rectangle border. */ 
	public static final int SIMPLE = 0x0001;

	/** Constant for a slightly raised border. */ 
	public static final int RAISED = 0x0205;

	/** Plain drawing mode. */
	public static final int PLAIN  = 1;
	
	/** Gray drawing mode. */
	public static final int GRAY   = 2;

	/** Erase mode. */
	public static final int ERASE  = 3;

	/** Invert mode. */
	public static final int INVERT = 4;

	/**
	 * Region copy mode: The copied region overwrites the destination.
	 */
	public static final int OVERWRITE = 0;

	/**
	 * Region copy mode: The copied region is AND'ed with the destination.
	 */
	public static final int AND = 1;

	/**
	 * Region copy mode: The copied region is AND'ed with the inverted
	 * destination region.
	 */
	public static final int AND_NOT = 2;

	/**
	 * Region copy mode: The copied region is XOR'ed with the destination.
	 */
	public static final int XOR = 3;

	/**
	 * Region copy mode: The copied region is OR'ed with the destination.
	 */
	public static final int OR = 4;

	/**
	 * Region copy mode: The copied region is inverted and overwrites the
	 * destination.
	 */
	public static final int NOT = 5;

	/**
	 * Create a new (<i>the</i> new) instance.
	 * This constructor is private to prevent others from creating 
	 * new instances.
	 */
	private Graphics() {};

	/**
	 * There is only ever one Graphics object in the system, and this returns it.
	 *
	 * @return the single global Graphics context.
	 */
	public static Graphics getGraphics() { return instance; }

	/**
	 * Draw a line.
	 *
	 * @param srcX the X coordinate of the starting point
	 * @param srcY the Y coordinate of the starting point
	 * @param dstX the X coordinate of the destination point
	 * @param dstY the Y coordinate of the destination point
	 * @param mode the drawing mode to use (one of <code>PLAIN, GRAY, ERASE</code>
	 *        or <code>INSERT</code>.
	 */
	public static native void drawLine(int srcX, int srcY, int dstX, int dstY,
		int mode);

	/**
	 * Draw a solid rectangle.
	 *
	 * @param left the x coordinate of the rectangle's top left corner
	 * @param top the y coordinate of the rectangle's top left corner
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @param mode the drawing mode to use (one of <code>PLAIN, GRAY, ERASE</code>
	 *        or <code>INSERT</code>.
	 * @param cornerDiam the diameter of four imaginary circles used to form the
	 *        rounded corners. An imaginary circle is placed within each corner
	 *        tangent to the rectangle on two sides.
	 */
	public static native void drawRectangle(int left, int top, int width,
		int height, int mode, int cornerDiam);

	/**
	 * Draw a rectangular border. The border is drawn around the rectangle
	 * specified by the given dimensions.
	 *
	 * @param left the x coordinate of the rectangle's top left corner
	 * @param top the y coordinate of the rectangle's top left corner
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @param mode the drawing mode to use (one of <code>PLAIN, GRAY, ERASE</code>
	 *        or <code>INSERT</code>.
	 * @param frameType one of <code>SIMPLE</code>, <code>RAISED</code> or a
	 *        type constructed by a call to <code>borderType</code>.
	 */
	public static native void drawBorder(int left, int top, int width,
		int height, int mode, int frameType);

	/**
	 * Constructs a border type.
	 *
	 * @param cornerDiam the diameter of four imaginary circles used to form
	 *        rounded corners. Must be in the range 0..38.
	 * @param shadow the width of a shadow. Must be in the range 0..3.
	 * @param width width of the border.Must be in the range 0..3.
	 * @return a value representing the specified type
	 */
	public static int borderType(int cornerDiam, int shadow, int width) {
		return ((cornerDiam << 8) | (shadow << 2) | width) & 0xFFFF;
	}

    /*
     * Methods to draw text.
     */

	/**
	 * Draw a string at a given position. Will draw <code>"null"</code> if
	 * <code>text</code> is <code>null</code>.
	 * @param text the String to draw
	 * @param left the x coordinate of the top left bound of first character.
	 * @param top the y coordinate of the top left bound of first character.
	 * @param mode the drawing mode to use (one of <code>PLAIN, RAY, ERASE</code>
	 *        or <code>INVERT</code>.
	 * @return right bound of last character drawn
	 */
	public static native int drawString(String text,int left,int top,int mode);

	/** 
	 * Draw a string at a given position. This method is equivalent to
	 * <code>drawString(text, left, top, PLAIN)</code>.
	 *
	 * @param text the String to draw
	 * @param left the x coordinate of the top left bound of first character.
	 * @param top the y coordinate of the top left bound of first character.
	 * @return the x coordinate of the right bound of last character drawn
	 */
	public static int drawString(String text, int left, int top) {
		return drawString(text, left, top, PLAIN);
	}

	/**
	 * Returns the width of a string in pixels.
	 *
	 * @param s the String to measure
	 * @return the width of the given String in pixels
	 */
	public static native int getWidth(String s);

	/**
	 * Returns the height of a string in pixels.
	 *
	 * @param s the String to measure
	 * @return the height of the given String in pixels
	 */
	public static native int getHeight(String s);


    /*
     * Methods to operate on clipping rectangles or the whole screen
     */

	/**
	 * Set the region in which drawing can be performed. If the specified region
	 * is <code>null</code> then the region is set to be the entire window.
	 *
	 * @param left the x coordinate of the top left position of the region
	 * @param left the y coordinate of the top left position of the region
	 * @param width the width of the region
	 * @param height the height of the region
	 */
	public native static void setDrawRegion(int left, int top, int width,
		int height);

	/**
	 * Reset the region in which drawing can be performed to be the whole
	 * screen.
	 */
	public native static void resetDrawRegion();

	public static final int ONSCREEN_WINDOW  = 0;
	public static final int OFFSCREEN_WINDOW = 1;
	
	/**
	 * Copy a rectangular region from one place to another.
	 *
	 * @param left the x coordinate of the region's top left corner
	 * @param top the y coordinate of the region's top left corner
	 * @param width the width of the region
	 * @param height the height of the region
	 * @param dstX the x coordinate of the point to which the region should
	 *        be copied
	 * @param dstY the y coordinate of the point to which the region should
	 *        be copied
	 * @param mode the copy mode (one of
	 * <code>OVERWRITE, AND, AND_NOT, XOR, OR, INVERT</code>)
	 */
	public native static void copyRegion(int left, int top, int width,
		int height, int dstX, int dstY, int mode);

	/**
	 * Copy a rectangular region from one place to another, possibly in
	 * different windows. There is the usual ONSCREEN_WINDOW and a hidden
	 * OFFSCREEN_WINDOW of the same size. The OFFSCREEN_WINDOW is handy for
	 * storing bitmaps in game programs.
	 *
	 * @param left the x coordinate of the source region's top left corner
	 * @param top the y coordinate of the source region's top left corner
	 * @param width the width of the source region
	 * @param height the height of the source region
	 * @param dstX the x coordinate of the point to which the region should
	 *        be copied in the destination
	 * @param dstY the y coordinate of the point to which the region should
	 *        be copied in the destination
	 * @param mode the copy mode (one of
	 * <code>OVERWRITE, AND, AND_NOT, XOR, OR, INVERT</code>)
	 * @param srcWind either ONSCREEN_WINDOW or OFFSCREEN_WINDOW
	 * @param dstWind either ONSCREEN_WINDOW or OFFSCREEN_WINDOW
	 */
	public native static void copyOffScreenRegion(int left, int top, int width,
		int height, int dstX, int dstY, int mode, int srcWind, int dstWind);

	/**
	 * Clear the screen.
	 */
	public static void clearScreen() {
		drawRectangle(0,0,160,160,ERASE,0);
	}

	/**
	 * Draw a bitmap.
	 *
	 * @param left the x coordinate of the bitmap's top left corner
	 * @param top the y coordinate of the bitmap's top left corner
	 * @param bitmap the bitmap to be drawn
	 */
	public native static void drawBitmap(int left,int top,Bitmap bitmap);

	/*
	 * System sounds (the Palm code numbers).
	 */

    /**
     * System sound for info.
     */
	public static final int SOUND_INFO = 1;
    /**
     * System sound for warning.
     */
	public static final int SOUND_WARNING = 2;
    /**
     * System sound for error.
     */
	public static final int SOUND_ERROR = 3;
    /**
     * System sound for startup.
     */
	public static final int SOUND_STARTUP = 4;
    /**
     * System sound for the alarm.
     */
	public static final int SOUND_ALARM = 5;
    /**
     * System sound for confirmation.
     */
	public static final int SOUND_CONFIRMATION = 6;
    /**
     * System sound for a click.
     */
	public static final int SOUND_CLICK = 7;
	
	/**
	 * Play a system sound.
	 *
	 * @param sound one of the SOUND_xxx constants
	 */
	public native static void playSound(int sound);
}
