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
 * A Ball is an animated ball that runs in it's own thread. It moves within a
 * rectangular region, bouncing off the walls.
 */
class Ball extends Thread {

	// random number generator
	static java.util.Random random = new java.util.Random(); 

	// the matrix to transform the direction based on the
	// current direction and which wall was hit
	static int[][] matrix = {
		{  1,-1,  -1, 1,   1, 1 },
		{ -1,-1,   1, 1,  -1, 1 },
		null,
		{  1, 1,  -1,-1,   1,-1 },
		{ -1, 1,   1,-1,  -1,-1 }
	};

	// the region in which the ball moves
	int top, left, width, height;

	// the position and radius of the ball
	int posX, posY, radius;

	// controls the speed of the ball; number of milliseconds to delay each loop
	int delay = 20;

	// the direction of the ball is controlled by these two variables
	int deltaX = 1;
	int deltaY = 1;

	// a handle onto the singleton Graphics object
	Graphics g;

	// public variables to control the behaviour of the thread
	public boolean running, stop;
	public boolean raised;

	/**
	 * Constructor defines the region in which the ball moves as well
	 * as the size of the ball.
	 */
	Ball(int left, int top, int width, int height, int radius) {

		this.radius = radius;
		this.left = left - radius;		
		this.top  = top - radius;		
		this.width = width;
		this.height = height;

		this.g = Graphics.getGraphics();
    }

	/**
	 * Raise the ball from the screen if the given coordinates match the
	 * current position of the ball.
	 */
	public void raise(int x, int y) {
		int ballSize = radius*2;
		if (posX < x && x < posX+ballSize && posY < y && y < posY+ballSize){

			// stop the ball running
			raised = true;

			// erase the ball from its current position
			g.drawRectangle(posX,posY,ballSize,ballSize,g.ERASE,radius);
		}
	}

	/**
	 * Place the ball back on the screen. Depending on where it was raised from,
	 * its direction and speed will be modified.
	 */
	public void place(int x, int y) {
		x -= radius/2;
		y -= radius/2;
		if (raised && left < x && x < left+width && top < y && y < top+height) {
			int ballSize = radius*2;

			// calculate the new direction
			if (x < posX) deltaX = 1; else deltaX = -1;
			if (y < posY) deltaY = 1; else deltaY = -1;

			// calculate the new speed (would be more accurate with sqrt)
			if (running) {
				int xdiff = x - posX;
				if (xdiff < 0) xdiff = -xdiff;
				int ydiff = y - posY; 
				if (ydiff < 0) ydiff = -ydiff;
				delay = (5 - (xdiff+ydiff)/3);
				if (delay < 1) delay = 1;
			}

			posX = x;
			posY = y;
			g.drawRectangle(posX,posY,ballSize,ballSize,g.PLAIN,radius);
			raised = false;
		}
	}

	/**
	 * Resize the ball (within certain limits) by the given amount.
	 */
	public void resize(int diff) {
		int prev = radius;
		radius += diff;
		if (radius < 2) radius = 2;
		else if (radius > 20) radius = 20;
		else {
			// size actually changed -> redraw
			int ballSize = prev*2;
			g.drawRectangle(posX,posY,ballSize,ballSize,g.ERASE,0);
			ballSize = radius*2;
			g.drawRectangle(posX,posY,ballSize,ballSize,g.PLAIN,radius);
		}
	}

	/**
	 * Starts the ball running. The 'running' and 'raised' variables control
	 * whether or not the ball is actually moving.
	 */
	public void run() {
		int right = left + width;
		int bottom = top + height;

		posX = (random.nextInt()>>>1)%width + left; // positive random #
		posY = (random.nextInt()>>>1)%height + top; // positive random #

		int ballSize = radius*2;

		// draw the ball at its initial position
		g.drawRectangle(posX,posY,ballSize,ballSize,g.PLAIN,radius);

		while (!stop) {

			if (!running || raised) {
				yield();
				continue;
			}
			ballSize = radius * 2;

			// erase the ball from its current position
			g.drawRectangle(posX,posY,ballSize,ballSize,g.ERASE,radius);

			// calculate a direction of the ball as an integer in the range
			// -2 .. 2 (excluding 0)
			int direction = deltaX + deltaY;
			if (direction == 0) direction = deltaX + 2*deltaY;

			// is the current position colliding with any wall
			int collision = 0;
			if (posX <= left || posX >= right) collision++;
			if (posY <= top || posY >= bottom) collision += 2;

			// change the direction appropriately if there was a collision
			if (collision != 0) {
				collision = (collision - 1) * 2;

				deltaX = matrix[direction+2][collision];
				deltaY = matrix[direction+2][collision+1];
			}

			// calculate the new position and draw the ball there
			posX = posX + deltaX;
			posY = posY + deltaY;
			g.drawRectangle(posX,posY,ballSize,ballSize,g.PLAIN,radius);

			// use the delay to control the speed of the ball
			try {
				sleep(delay);
			} catch (InterruptedException e) {}
		}
	}
}
