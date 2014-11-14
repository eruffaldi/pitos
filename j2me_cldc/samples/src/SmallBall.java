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
 * A SmallBall is a lightweight animated ball that runs in it's own thread.
 * It moves within a rectangular region, bouncing off the walls.
 */
class SmallBall extends Thread {

	// random number generator
	static java.util.Random random = new java.util.Random(); 

	// controls the speed of all balls; delay in centiseconds
	static int delay = 20;
	static void slower() {
		delay += 10;
		if (delay > 100) delay = 100;
	}

	static void faster() {
		delay -= 10;
		if (delay < 0) delay = 0;
	}

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
	int posX, posY, radius = 5;

	// the direction of the ball is controlled by these two variables
	int deltaX;
	int deltaY;

	// a handle onto the singleton Graphics object
	Graphics g;

	// public variables to control the behaviour of the thread
	public boolean stop;

	/**
	 * Constructor defines the region in which the ball moves as well
	 * as its starting position.
	 */
	SmallBall(int left, int top, int width, int height) {
		super();

		this.left = left + 1;		
		this.top  = top + 1;		
		this.width = width - (2 * radius + 2);
		this.height = height - (2 * radius + 2);
		
		// use positive random #s
		this.posX = (random.nextInt()>>>1) % (this.width-20) + 10;
		this.posY = (random.nextInt()>>>1) % (this.height-20) + 10;

		deltaX = random.nextInt() & 1;
		deltaY = random.nextInt() & 1;
		
		if (deltaX == 0) deltaX = -1;
		if (deltaY == 0) deltaY = -1;

		this.g = Graphics.getGraphics();
    }

	/**
	 * Starts the ball running.
	 */
	public void run() {
		int right = left + width;
		int bottom = top + height;

		int ballSize = radius*2;

		// draw the ball at its initial position
		g.drawRectangle(posX,posY,ballSize,ballSize,g.PLAIN,radius);


		while (!stop) {

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

		// erase the ball from its current position
		g.drawRectangle(posX,posY,ballSize,ballSize,g.ERASE,radius);

	}
}
