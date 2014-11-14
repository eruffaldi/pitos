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

// This application can be refactored into MVC form.

public class Pong extends Spotlet
{
    // the Graphics object
    static Graphics g = Graphics.getGraphics();

    // the position and dimensions of the drawing frame
    public static int frameMinX = 0;
    public static int frameMinY = 0;
    public static int frameMaxX = 159;
    public static int frameMaxY = 120;

    // paddle size, position (top of paddle), and speed
    public static int paddlesize = 20;
    public static int paddletop = 50; // (frameMaxY - paddlesize) / 2
    public static int paddlespeed = 4;

    // the set of balls
    static PongBall[] balls;
    static int ballcount;
    static int ballactive;

    // the count of hits on the paddle
    static int score;

    // the GUI buttons
    static Button playButton;
    static Button startButton;
    static Button stopButton;
    static Button fasterButton;
    static Button slowerButton;
    static Button moreButton;
    static Button fewerButton;
    static Button largerButton;
    static Button smallerButton;
    static Button exitButton;

    // the state of the game
    static boolean suspended;

    // create a Pong spotlet and register the event handlers
    public static void main(String[] args)
    {

		String helpText = "Welcome to the Pong game!\n\nThe object of Pong is to keep the ball(s) within the box and not let them get past your paddle.  Each time you hit the ball(s) back you get a point (shown in the lower right hand corner of the screen).\n\nTap the option buttons to change the size of the paddle, increase or decrease the number of balls, or to alter the speed at which the balls are bouncing.  Each tap with your stylus or finger increases or decreases the speed slightly.  The scroll button moves your paddle up or down to return the ball back thus scoring a point. Use your stylus to draw out different size boxes in which to play the game.  Good luck!";

		(new HelpDisplay(helpText, "Pong",
							 NO_EVENT_OPTIONS)).register(NO_EVENT_OPTIONS);
    }

    // let the game begin
    //   initialize the ball array and the graphics objects
    public Pong()
    {
        int balltotal = 5;

        // initialize the array of balls
        balls = new PongBall[balltotal];

        // create the buttons
        startButton = new Button("Start",1,128);
        stopButton = new Button("Stop ",1,145);
        fasterButton = new Button("Faster",33,128);
        slowerButton = new Button("Slower",33,145);
        moreButton = new Button("More ",67,128);
        fewerButton = new Button("Fewer",67,145);
        largerButton = new Button("Larger ",98,128);
        smallerButton = new Button("Smaller",98,145);
        exitButton = new Button("Exit",139,145);

        // clear the score
        score = 0;

        // initialize the state
        suspended = true;
        ballactive = 0;

        // clear the screen
        g.clearScreen();
		// display the screen
		displayScreen();

		// create one ball and start it
		balls[0] = new PongBall();
		ballcount = 1;
		balls[0].start();

    }

    // display the frame (which contains the ball) and the controls
    private void displayScreen()
    {
        // clear the screen
        g.clearScreen();

        // draw the frame
        // g.drawBorder(frameMinX,frameMinY,frameMaxX,frameMaxY,
        //              g.PLAIN,g.SIMPLE);
        g.drawLine(frameMinX,frameMinY,frameMaxX,frameMinY,g.PLAIN);
        g.drawLine(frameMaxX,frameMinY,frameMaxX,frameMaxY,g.PLAIN);
        g.drawLine(frameMaxX,frameMaxY,frameMinX,frameMaxY,g.PLAIN);
        g.drawLine(frameMinX,frameMinY,frameMinX,frameMaxY,g.ERASE);

        // display the paddle
        displayPaddle();

        // draw the GUI controls and buttons
        exitButton.paint();
        startButton.paint();
        stopButton.paint();
        fasterButton.paint();
        slowerButton.paint();
        moreButton.paint();
        fewerButton.paint();
        largerButton.paint();
        smallerButton.paint();

        // display the score
        displayScore(0);
    }

    // display the paddle
    private void displayPaddle()
    {
        g.drawLine(frameMinX,frameMinY,frameMinX,frameMaxY,g.ERASE);
        g.drawLine(frameMinX,paddletop,frameMinX,paddletop+paddlesize,g.PLAIN);
    }

    // display the score, possibly incrementing it
    public static void displayScore(int increment)
    {
        char digits[] = new char[4];
        // boost the score
        score = score + increment;
        // set end of string
        digits[3] = (char) 0;
        // compute the ones digit
        digits[2] = (char) ((score % 10) + (int) '0');
        // compute the tens digit
        digits[1] = (char) (((score / 10) % 10) + (int) '0');
        // compute the hundreds digit
        digits[0] = (char) (((score / 100) % 10) + (int) '0');
        // print the new score
        g.drawString(new String(digits, 0, digits.length),140,128);
    }

    // handle a pen down event
    //   exit
    //   start
    //   stop
    //   increase speed
    //   decrease speed
    //   add a ball
    //   remove a ball
    //   increase paddle size
    //   decrease paddle size
    // other ideas
    //   make ball larger or smaller
    //   change speed of paddle alone
    //   speed up ball with finer granularity than tenths of a second
    public void penDown(int x, int y)
    {

        // handle game events (the game form)
		// exit
		if (exitButton.pressed(x,y))
		{
			// stop all the balls
			for (int i = 0;
				 i < balls.length && balls[i] != null;
				 i++)
			{
				balls[i].missed = true;
				balls[i].suspended = false;
				balls[i].stopped = true;

				// enable the balls to be garbage collected
				balls[i] = null;
			}
			System.exit(0);
		}

		// start the game
		if (startButton.pressed(x,y))
		{
			boolean reset = true;

			// start all the balls
			for (int i = 0;
				 i < balls.length && balls[i] != null;
				 i++)
			{
				if (balls[i].missed == false)
					reset = false;
				balls[i].missed = false;
				balls[i].suspended = false;
			}

			// set the state to running
			suspended = false;

			// set the active ball count
			ballactive = ballcount;

			// if all balls are out of play then reset the score
			if (reset)
			{
				score = 0;
				displayScore(0);
			}

			// done
			return;
		}

		// suspend the game
		if (stopButton.pressed(x,y))
		{
			// suspend all the balls
			for (int i = 0;
				 i < balls.length && balls[i] != null;
				 i++)
				balls[i].suspended = true;

			// set the state to suspended
			suspended = true;

			// set the active ball count
			ballactive = 0;

			// done
			return;
		}

		// increase ball and paddle speed
		if (fasterButton.pressed(x,y))
		{
			PongBall.faster();
			paddlespeed = paddlespeed + paddlespeed;
			if (paddlespeed > 8)
				paddlespeed = 8;

			return;
		}

		// decrease ball and paddle speed
		if (slowerButton.pressed(x,y))
		{
			PongBall.slower();
			paddlespeed = paddlespeed - paddlespeed / 2;
			if (paddlespeed < 2)
				paddlespeed = 2;

			return;
		}

		// add a ball
		if (moreButton.pressed(x,y) && ballcount < balls.length)
		{
			// create a new ball and start it if appropriate
			balls[ballcount] = new PongBall();
			balls[ballcount].start();
			if (suspended)
				balls[ballcount].suspended = true;
			else
			{
				balls[ballcount].suspended = false;
				ballactive = ballactive + 1;
			}

			// increment the number of balls
			ballcount = ballcount + 1;

			// done
			return;
		}

		// remove a ball (always keep one)
		if (fewerButton.pressed(x,y) && ballcount > 1)
		{
			// decrement the number of balls
			ballcount = ballcount - 1;

			// stop the thread and remove the reference to it
			balls[ballcount].missed = true;
			balls[ballcount].suspended = false;
			balls[ballcount].stopped = true;
			balls[ballcount] = null;

			// done
			return;
		}

		// increase paddle size
		if (largerButton.pressed(x,y)
				 && paddlesize < frameMaxY - frameMinY - 4)
		{
			paddlesize = paddlesize + 5;
			if (paddletop > frameMaxY - paddlesize)
				paddletop = frameMaxY - paddlesize;
			displayPaddle();

			return;
		}

		// decrease paddle size
		if (smallerButton.pressed(x,y) && paddlesize > 5)
		{
			paddlesize = paddlesize - 5;
			displayPaddle();

			return;
		}
    }

    // handle a key down event
    //   move the paddle up
    //   move the paddle down
    public void keyDown(int keyCode)
    {
		switch (keyCode)
        {
            case PAGEUP:
                // move the paddle
                if (paddletop > frameMinY)
                    paddletop = paddletop - paddlespeed;
                // fix out of bounds problem
                if (paddletop < frameMinY)
                    paddletop = frameMinY;
                // draw the paddle
                displayPaddle();
                break;

            case PAGEDOWN:
                // move the paddle
                if (paddletop < frameMaxY - paddlesize)
                    paddletop = paddletop + paddlespeed;
                // fix out of bounds problem
                if (paddletop > frameMaxY - paddlesize)
                    paddletop = frameMaxY - paddlesize;
                // draw the paddle
                displayPaddle();
                break;

            default:
                break;
        }
    }

    // handle a pen up event
    //   redefine the game rectangle
    //   the user specifies the lower right corner
    public void penUp(int x, int y)
    {
		// the game must be suspended or have no balls visible
        if ((!suspended) && ballactive != 0)
            return;

        // the pen must not be in the button area
        if (y > 120)
            return;

        // set the lower right corner x, enforcing minimum
        if (x < 100)
            frameMaxX = 120;
        else
            frameMaxX = x;

        // set the lower right corner y, enforcing minimum
        if (y < 100)
            frameMaxY = 80;
        else
            frameMaxY = y;

        // set the upper left corner x symmetrically
        frameMinX = 159 - frameMaxX;

        // set the upper left corner y symmetrically
        frameMinY = 120 - frameMaxY;

        // move the paddle
        paddletop = frameMinY;

        // resize the paddle if necessary
        if (paddlesize > frameMaxY - frameMinY)
            paddlesize = frameMaxY - frameMinY;

        // reset all the balls (so they will reappear within the frame)
        for (int i = 0;
             i < balls.length && balls[i] != null;
             i++)
        {
            balls[i].missed = true;
            balls[i].suspended = false;
        }

        // display the screen
        displayScreen();
    }
}
