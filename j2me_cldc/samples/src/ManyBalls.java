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

public class ManyBalls extends Spotlet {

	// handle on the singleton Graphics object
	static Graphics g = Graphics.getGraphics();

	// The more and less button bitmaps
	static Bitmap moreBitmap = new Bitmap((short)2,new byte[] {
		(byte)0x01, (byte)0x80,
		(byte)0x01, (byte)0x80,
		(byte)0x01, (byte)0x80,
		(byte)0x0F, (byte)0xF0,
		(byte)0x0F, (byte)0xF0,
		(byte)0x01, (byte)0x80,
		(byte)0x01, (byte)0x80,
		(byte)0x01, (byte)0x80 });
	static Bitmap lessBitmap = new Bitmap((short)2,new byte[] {
		(byte)0x00, (byte)0x00,
		(byte)0x00, (byte)0x00,
		(byte)0x00, (byte)0x00,
		(byte)0x0F, (byte)0xF0,
		(byte)0x0F, (byte)0xF0,
		(byte)0x00, (byte)0x00,
		(byte)0x00, (byte)0x00,
		(byte)0x00, (byte)0x00 });
	

	// the last point the user touched the screen
	int lastX;
	int lastY;

	// the position and dimensions of the drawing frame
	int frameCnrX;
	int frameCnrY;
	int frameWidth;
	int frameHt;

	// the GUI buttons
	Button exitButton, clearButton, moreButton, lessButton;

	// a set of free roaming balls
	SmallBall[] balls;
	int numBalls;

	/**
	 * The main method simply creates a ManyBalls spotlet and registers
	 * its event handlers.
	 */
	public static void main(String[] args) {
		int numBalls = 30;
		if (args.length >= 1) { 
			try { 
				numBalls = Integer.parseInt(args[0]);
			} catch (NumberFormatException e) {
				/* Ignore it if it's not an integer */
			}
		}
		(new ManyBalls(numBalls)).register(NO_EVENT_OPTIONS);
	}

	public ManyBalls(int maxBalls) {

		// initialize the array of balls
		balls = new SmallBall[maxBalls];

		// create the buttons
		clearButton = new Button("Clear",108,145);
		exitButton = new Button("Exit",136,145);
		lessButton = new Button(lessBitmap,2,146);
		moreButton = new Button(moreBitmap,20,146);

		g.clearScreen();

		frameCnrX = 0;
		frameCnrY = 0;
		frameWidth = 160;
		frameHt    = 140;

		// Start with one ball
		balls[0] = new SmallBall(frameCnrX, frameCnrY,frameWidth,frameHt);
		balls[0].start();
		numBalls = 1;

		paint();
		refreshCounter();
	}

	/**
	 * Refresh the counter shown on the screen.
	 */
	private void refreshCounter() {
		
		// Draw the ball number indicator
		g.drawString(numBalls+" threads  ",40,145);
	}

	/**
	 * Draws the drawing frame (which also contains the ball) and the
	 * controls.
	 */
	private void paint() {

		// Draw the frame and clear the space below it
		g.drawBorder(frameCnrX+1,frameCnrY+1,frameWidth-2,frameHt-2,
			g.PLAIN,g.SIMPLE);
		g.drawRectangle(0,140,160,20,g.ERASE,0);
			
		// Draw GUI controls and buttons
		clearButton.paint();
		exitButton.paint();
		moreButton.paint();
		lessButton.paint();
	}

	/**
	 * Handle a pen down event.
	 */
	public void penDown(int x, int y) {

		if (lessButton.pressed(x,y)) {
			if (numBalls > 0) {

				// decrement the counter
				numBalls = numBalls - 1;

				// stop the thread and remove the reference to it
				balls[numBalls].stop = true;
				balls[numBalls] = null;
			}
			
			// Redraw the counter
			refreshCounter();
		}
		else if (moreButton.pressed(x,y)) {
			if (numBalls < balls.length) {

				// create a new ball and start it moving
				balls[numBalls] = new SmallBall(frameCnrX, frameCnrY,
					frameWidth,frameHt);
				balls[numBalls].start();

				// increment the counter
				numBalls = numBalls + 1;

			}
			
			// Redraw the counter
			refreshCounter();
		}
		else if (clearButton.pressed(x,y)) {

			// kill all the balls
			for (int i = 0; i < balls.length && balls[i] != null; i++){
				balls[i].stop = true;

				// Remove reference to each ball
				balls[i] = null;
			}

			// clear the frame 
			g.drawLine(frameCnrX+1,frameCnrY+1,frameCnrX+1,
				frameCnrY+frameHt-2,g.ERASE);
			for (int i = frameCnrX+1; i < frameCnrX+frameWidth; i++)
				g.copyRegion(i,frameCnrY+1,frameCnrX+frameWidth-2-i,
					frameCnrY+frameHt-2,i+1,frameCnrY+1,g.OVERWRITE);

			// reset the number of balls counter
			numBalls = 0;

			// Redraw the counter
			refreshCounter();
		}
		else if (exitButton.pressed(x,y)) {
			// kill all the balls
			for (int i = 0; i < balls.length && balls[i] != null; i++) {
				balls[i].stop = true;

				// enable the balls to be garbage collected
				balls[i] = null;
			}
			System.exit(0);
		}
	}

	/**
	 * Handle a key down event.
	 */
	public void keyDown(int keyCode) {
		switch (keyCode) {
			case PAGEDOWN: SmallBall.slower(); break;
			case PAGEUP: SmallBall.faster(); break;
		}
	}
}
