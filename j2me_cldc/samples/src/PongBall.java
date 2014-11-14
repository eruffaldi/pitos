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

// PongBall is based on SmallBall
// A PongBall is a lightweight animated ball that runs in its own thread.
// It moves within a rectangular region, bouncing off the walls.
class PongBall extends Thread
{
	// random number generator
	static java.util.Random random = new java.util.Random(); 

    // control the speed of all balls
    //   delay is in milliseconds
    static int delay = 20;
    static void slower()
    {
        delay += 10;
        if (delay > 100) delay = 100;
    }
    static void faster()
    {
        delay -= 10;
        if (delay < 0) delay = 0;
    }

    // this matrix is used to change the direction of a ball when it bounces,
    //   based on the current direction and which wall was hit
    static int[][] matrix =
    {
        {  1,-1,  -1, 1,   1, 1 },
        { -1,-1,   1, 1,  -1, 1 },
        null,
        {  1, 1,  -1,-1,   1,-1 },
        { -1, 1,   1,-1,  -1,-1 }
    };

    // the region in which the ball moves
    int minX;
    int minY;
    int maxX;
    int maxY;

    // the position of the ball
    int posX;
    int posY;

    // the radius of the ball
    int radius = 5;

    // the velocity (direction and speed) of the ball
    int deltaX;
    int deltaY;

    // the graphics object
    Graphics g;

    // public variable to indicate that a ball missed the paddle
    //   (up a creek without a paddle)
    public boolean missed;

    // public variable to suspend a ball
    public boolean suspended;

    // public variable to stop a ball (stop its thread)
    public boolean stopped;

    // construct a new PongBall
    //   the arguments define the active region
    PongBall()
    {
        super();

        // set the graphics object
        this.g = Graphics.getGraphics();

        // initialize state
        missed = false;
        suspended = true;
        stopped = false;
    }

    // move the ball
    public void run()
    {
        // the ball diameter
        int diameter = radius*2;

        // loop while the thread is active
        while (!stopped)
        {
            // set the ball's region
            minX = Pong.frameMinX + 1;
            minY = Pong.frameMinY + 1;
            maxX = Pong.frameMaxX - (diameter + 2);
            maxY = Pong.frameMaxY - (diameter + 2);

            // set the ball's position (use positive random #s)
            posX = ((random.nextInt()>>>1) % (maxX - minX)) + minX;
            posY = ((random.nextInt()>>>1) % (maxY - minY)) + minY;

            // set the ball's direction
            deltaX = random.nextInt() & 1;
            deltaY = random.nextInt() & 1;
            if (deltaX == 0) deltaX = -1;
            if (deltaY == 0) deltaY = -1;

            // draw the ball at its initial position
            g.drawRectangle(posX,posY,diameter,diameter,g.PLAIN,radius);

            // loop while the ball is in play
            while (!missed)
            {
                // loop while the ball is inactive
                while (suspended)
					try {
                        sleep(delay);
					} catch (InterruptedException e) {}
                if (missed)
                    break;

                // erase the current image of the ball
                g.drawRectangle(posX,posY,diameter,diameter,g.ERASE,radius);

                // calculate the direction of the ball as an integer
                //   in the range -2 .. 2 (excluding 0)
                int direction = deltaX + deltaY;
                if (direction == 0) direction = deltaX + 2*deltaY;

                // is the ball in its old position colliding with any wall?
                int collision = 0;
                if (posX <= minX || posX >= maxX) collision++;
                if (posY <= minY || posY >= maxY) collision += 2;

                // check for ball-paddle interaction
                if (posX <= minX)
                {
                    // test for paddle hit at minY and 
                    if ((posY+diameter < Pong.paddletop)
                        || (posY > Pong.paddletop+Pong.paddlesize))
                    {
                        Pong.ballactive = Pong.ballactive - 1;
                        missed = true;
                        break;
                    }

                    // the ball hit the paddle
                    Pong.displayScore(1);
                }

                // if the ball hit a wall then change direction
                if (collision != 0)
                {
                    collision = (collision - 1) * 2;

                    deltaX = matrix[direction+2][collision];
                    deltaY = matrix[direction+2][collision+1];
                }

                // calculate the new position and draw the ball there
                posX = posX + deltaX;
                posY = posY + deltaY;
                g.drawRectangle(posX,posY,diameter,diameter,g.PLAIN,radius);

                // use the delay to control the speed of the ball
                try {
					sleep(delay);
				} catch (InterruptedException e) {}
            }

            // wait to start another game
            while (!stopped && missed)
				try {
					sleep(delay);
				} catch (InterruptedException e) {}
        }

        // erase the final image of the ball
        g.drawRectangle(posX,posY,diameter,diameter,g.ERASE,radius);
    }
}
