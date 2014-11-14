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
 
import com.sun.kjava.*;

class MovingStar {

/*==============================================================================
 * Constants
 *============================================================================*/

	// Width of the star bitmap in pixels
	static final int BITMAPWIDTH = 8;

/*==============================================================================
 * Static variables
 *============================================================================*/

	// Handle on the singleton Graphics object
	static Graphics g = Graphics.getGraphics();

	// Star bitmap
	static Bitmap starBitmap = new Bitmap((short)1, new byte[] {
		(byte)0x18,
		(byte)0x42,
		(byte)0x24,
		(byte)0x99,
		(byte)0x99,
		(byte)0x24,
		(byte)0x42,
		(byte)0x18 });

	// Initial speed of stars
	static int starSpeed = 4;
	
	// random number generator
	static java.util.Random random = new java.util.Random(); 

/*==============================================================================
 * Instance variables
 *============================================================================*/

	// Current coordinates
	int x, y;

/*==============================================================================
 * Static methods
 *============================================================================*/

	/**
	 * Boost star speed.
	 */
	public static void boostSpeed() {
		if (starSpeed == 4) starSpeed = 6;
	}

/*==============================================================================
 * Constructors
 *============================================================================*/

	/**
	 * Constructor.
	 */
	public MovingStar() {
		x = 159;
		y = (random.nextInt()>>>1) % 138; // positive random #
    }

/*==============================================================================
 * Instance methods
 *============================================================================*/

	/**
	 * Scroll the star left on the display.
	 */
	public void scrollLeft() {
		undraw();
		x -= starSpeed;
		g.drawBitmap(x, y, starBitmap);
	}

	/**
	 * Undraw the star.
	 */
	public void undraw() {
		g.drawRectangle(x, y, BITMAPWIDTH, BITMAPWIDTH, g.ERASE, 0);
	}

	/**
	 * Check if the star went out of the visible display area.
	 */
	public boolean wentBy() {
		if (x < -5) 
			 return true;
		else return false;
	}

	/**
	 * Check if the star hit the ship at given height.
	 */
	public boolean hitShip(int height) {
		if (x < 10 && y >= height-7 && y <= height+7)
			 return true;
		else return false;
	}

	/**
	 * (Re)initialize the position of the star.
	 * Allows "recycling" of star objects.
	 */
	public void initialize() {
		x = 155;
		y = (random.nextInt()>>>1) % 138; // positive random #
	}
}
