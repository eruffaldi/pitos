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

/**
 * This class implements that array version of the OneDragon program which draws
 * a dragon like fractal on the screen.
 */

class OneDragon extends Thread {

	static int dragons = 0;
	public static boolean noDragons() { return dragons == 0; }

	// instance variables -- screen variables
	// .. current (x,y) position on screen
	private int posX;
	private int posY;

	// .. length -- length of each segment
	private int length;

	// .. iterations -- number of iterations
	private int iterations;

	// the height of draw region (measured from the top of the screen
	private int bottom = 130;

	// move up, down, left, right, updating position
	// direction: up = 0, right = 1, down = 2, left = 3
	// rotation: 90 degrees clockwise (add 1 to segment, mod 4)
	public void move(int direction)
	{
		int x = posX;
		int y = posY;
		switch (direction)
		{
			case 0:
				y = y - length;
				break;
			case 1:
				x = x + length;
				break;
			case 2:
				y = y + length;
				break;
			case 3:
				x = x - length;
				break;
			default:
				return;
		}
		Graphics.drawLine(posX,posY,x,y,Graphics.PLAIN);
		posX = x;
		posY = y;
	}

	// rotate direction
	public int rotate(int direction)
	{
		int rotated;
		direction = (++direction) % 4;
		move(direction);
		return direction;
	}

	// arguments: number of iterations, segment length, origin x and y
	public OneDragon(int n, int length, int startX, int startY) {
		this.posX   = startX;
		this.posY   = startY;
		this.length     = length;
		this.iterations = n;
	}

	public void run() {
	
		int p;	// scanning index into segment array
		int r;	// root index into segment array
		int s = 2 << (iterations - 1);	// size of segment array

		// do a gc here and then test the available memory as this is
		// potentially a big allocation
		System.gc();

		int d[] = new int[s];	// segment array;

		synchronized(this) {
			if (dragons == 0)
				Graphics.setDrawRegion(0,0,160,bottom);
			dragons++;
		}

		// initial step -- first segment -- move right
		move(1);
		d[0] = 1;
		r = 0;

		// iterate n times over the segment array
		for (int i = 1; i <= iterations; i++) {

			// initialize the scan pointer into the segment array
			p = r;

			// iterate down the segment array from the root
			while (p >= 0) {

				// increment the root pointer
				r = r + 1;

				// initialize the new segment
				d[r] = rotate(d[p]);

				// move the scan pointer to the next (previous) segment
				p = p - 1;
			}
		}

		synchronized(this) {
			if (dragons == 1)
				Graphics.resetDrawRegion();
			dragons--;
		}
	}

}
