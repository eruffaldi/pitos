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

public class Scribble extends Spotlet {

	// handle on the singleton Graphics object
	static Graphics g = Graphics.getGraphics();

	// a bitmap representing an arrow
	static Bitmap arrow = new Bitmap((short)2,new byte[] { 
		(byte)0x00,(byte)0x10,
		(byte)0x00,(byte)0x18,
		(byte)0xFF,(byte)0xFC,
		(byte)0xFF,(byte)0xFE,
		(byte)0xFF,(byte)0xFC,
		(byte)0x00,(byte)0x18,
		(byte)0x00,(byte)0x10});

	// the last point the user touched the screen
	int lastX;
	int lastY;

	// the position and dimensions of the drawing frame
	int frameCnrX;
	int frameCnrY;
	int frameWidth;
	int frameHt;

	// the GUI buttons
	Button exitButton, clearButton, controlButton;

	// the graffiti character entered
	String lastKey = "";

	// a free roaming ball
	Ball ball;

	/**
	 * Clears the screen by pulling everything into the centre and then
	 * exits.
	 */
	public static void exitClear() {

		g.resetDrawRegion();

		g.drawLine(1,1,158,1,g.PLAIN);
		g.drawLine(1,1,1,158,g.PLAIN);
		g.drawLine(158,1,158,158,g.PLAIN);
		g.drawLine(1,158,158,158,g.PLAIN);

		g.drawLine(0,0,159,0,g.ERASE);
		g.drawLine(0,0,0,159,g.ERASE);
		g.drawLine(159,0,159,159,g.ERASE);
		g.drawLine(0,159,159,159,g.ERASE);

		for (int i = 0; i < 80; i++) {

			// delay so we can see what is happening
			try {
				Thread.sleep(10);
			} catch (InterruptedException e) {}

			int dim = 79-i;
			g.copyRegion(i  ,i  ,dim,dim, i+1 ,i+1,g.OVERWRITE);
			g.copyRegion(81 ,i  ,dim,dim, 80  ,i+1,g.OVERWRITE);
			g.copyRegion(i  ,81 ,dim,dim, i+1 ,80 ,g.OVERWRITE);
			g.copyRegion(81 ,81 ,dim,dim, 80  ,80 ,g.OVERWRITE);
		}

		System.exit(0);
	}

	/**
	 * The main method simply creates a Scribble spotlet and
	 * registers its event
	 * handlers.
	 */
	public static void main(String[] args) {

		String helpText = "Scribble lets you scribble on the screen with your stylus while the ball bounces around in the box and erases your picture.  You can start and stop the ball with the corresponding button, and you can change size of the ball with the scroll button.";

		(new HelpDisplay(helpText, "Scribble",
							 NO_EVENT_OPTIONS)).register(NO_EVENT_OPTIONS);

	}

	/**
	 * Default constructor creates the GUI components and draws them.
	 */
	public Scribble() {
		lastX = lastY = 0;

		// create the buttons
		controlButton = new Button("Start",82,145);
		clearButton = new Button("Clear",111,145);
		exitButton = new Button("Exit",139,145);

		g.clearScreen();

		frameCnrX = 5;
		frameCnrY = 0;
		frameWidth = 155;
		frameHt    = 135;

		paint();
		ball = new Ball(frameCnrX,frameCnrY,frameWidth,frameHt,10);
		ball.start();
	}

	/**
	 * Displays the drawing frame (which also contains the ball) and the
	 * controls.
	 */
	private void paint() {

		g.resetDrawRegion();

		// Draw the frame and clear the space below it
		g.drawBorder(frameCnrX+1,frameCnrY+1,frameWidth-2,frameHt-2,
			g.PLAIN,g.SIMPLE);
		g.drawRectangle(0,140,160,20,g.ERASE,0);
			
		// Draw graffiti char
		g.drawString("Graffiti:",7,145);
		g.drawString(lastKey,45,145);
		
		// Draw GUI controls and buttons
		controlButton.paint();
		clearButton.paint();
		exitButton.paint();

		// constrain the drawing region to within the frame
		g.setDrawRegion(frameCnrX+1,frameCnrY+1,frameWidth-2,frameHt-2);
	}

	/**
	 * Handle a pen down event.
	 */
	public void penDown(int x, int y) {

		if (controlButton.pressed(x,y)) {
			ball.running = !ball.running;
			controlButton.setText((ball.running ? "Stop  " : "Start"));
			paint();
			return;
		}
		else if (clearButton.pressed(x,y)) {
			boolean ballState = ball.running;
			ball.running = false;

			// slide the frame contents off to the right
			g.drawLine(frameCnrX+1,frameCnrY+1,frameCnrX+1,
				frameCnrY+frameHt-2,g.ERASE);
			for (int i = frameCnrX+1; i < frameCnrX+frameWidth; i++)
				g.copyRegion(i,frameCnrY+1,frameCnrX+frameWidth-1-i,
					frameCnrY+frameHt-2,i+1,frameCnrY+1,g.OVERWRITE);

			ball.running = ballState;
			return;
		}
		else if (exitButton.pressed(x,y)) {
			ball.stop = true;
			exitClear();
		}
		else
			ball.raise(x,y);
		lastX = x;
		lastY = y;

		if (ball.raised)
			g.resetDrawRegion(); 
	}

	/**
	 * Handle a pen up event.
	 */
	public void penUp(int x, int y) {
		g.drawRectangle(0,lastY-ball.radius,5,ball.radius*2,g.ERASE,0);
		g.drawRectangle(lastX-ball.radius,135,ball.radius*2,5,g.ERASE,0);
		if (ball.raised)
			g.setDrawRegion(frameCnrX+1,frameCnrY+1,frameWidth-2,frameHt-2);
		
		ball.place(x,y);
	}

	/**
	 * Handle a key down event.
	 */
	public void keyDown(int keyCode) {
		switch (keyCode) {
			case PAGEDOWN: ball.resize(-1); break;
			case PAGEUP: ball.resize(1); break;
			default:
				lastKey = (char)keyCode+"  ";
				paint();
				break;
		}
	}

	/**
	 * Handle a pen move event.
	 */
	public void penMove(int x, int y) {
		if (frameCnrX < x && frameCnrX+frameWidth > x &&
			frameCnrY < y && frameCnrY+frameHt > y) {
			if (!ball.raised) {
				g.drawLine(lastX,lastY,x,y,g.PLAIN);
			}
			else {
				g.drawRectangle(0,lastY-ball.radius,5,ball.radius*2,g.ERASE,0);
				g.drawRectangle(lastX-ball.radius,135,ball.radius*2,5,g.ERASE,0);
				g.drawRectangle(0,y-ball.radius,5,ball.radius*2,g.PLAIN,0);
				g.drawRectangle(x-ball.radius,135,ball.radius*2,5,g.PLAIN,0);
			}
			lastX = x;
			lastY = y;
		}
	}
}
