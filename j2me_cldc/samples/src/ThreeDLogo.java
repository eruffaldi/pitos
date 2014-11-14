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

public class ThreeDLogo extends Spotlet {

/*==============================================================================
 * Constants
 *============================================================================*/

    /**
     * Size of the data packet to beam
     */
	static final int BEAMPACKETLENGTH = 16;

/*==============================================================================
 * Static variables
 *============================================================================*/

   /**
     * g: handle on the singleton Graphics object.
     */
	static Graphics g = Graphics.getGraphics();

/*==============================================================================
 * Instance variables
 *============================================================================*/

   /**
     * currentX, currentY: used for storing the coordinates of
     * the viewpoint (default: 0, 0, 0).
     */
    int currentX;
    int currentY;
    int currentZ;

    /**
     * object: the graphical object to be displayed.
     */
    ThreeDeeObject object;

    /**
     * tapX, tapY: pen tap recording
     */
    int tapX, tapY;
    int tapDiffX, tapDiffY;

    /**
     * the GUI buttons
     */
	Button exitButton;
	Button beamButton;

/*==============================================================================
 * Methods
 *============================================================================*/

    /**
     * constructor: initialize the demo.
     */
    public ThreeDLogo() {

        // Initialize the object to be shown
        object = new ThreeDeeObject(0,  0, -6000);

        // Initialize view coordinates
        currentX = 0;
        currentY = 0;
        currentZ = 0;

        // Initialize tap variables
		tapX = -1;
		tapDiffX = 0;
		tapDiffY = 0;

		// Create the buttons
		exitButton = new Button("Exit", 138, 146);
		beamButton = new Button("Beam", 1, 1);
		
		paint();
    }

	/**
	 * Repaint the view
	 */
	public void paint() {
		object.project(currentX, currentY, currentZ);
		g.clearScreen();
		exitButton.paint();
		beamButton.paint();
		g.drawString("Drag pen to rotate; tap to reset", 1, 149, g.PLAIN);
		object.display(g);
	}

	/**
	 * rotate the object and repaint
	 */
    public void rotate(int angleX, int angleY, int angleZ) {
		g.drawString("  Rotating object...  ", 50, 2, g.INVERT);
		object.rotate(angleX, angleY, angleZ);
        paint();
	}
	
	/**
	 * Handle a key down event.
	 */
	public void keyDown(int keyCode) {
		switch (keyCode) {
			case PAGEDOWN:
				// Move the object farther
				object.moveBy(0, 0, -250);
				paint();
				break;
			case PAGEUP:
				// Move the object closer; avoid clipping problems
				int oldZ = object.getZ();
				object.moveBy(0, 0, 250);
				// Paint only if location really changed
				if (object.getZ() != oldZ) paint();
				break;
		}
	}

	/**
	 * Handle a beam receive event.
	 */
	public void beamReceive(byte[] data) {

		//**printBeamParams(data);    // for debugging use

		// The length of the data package should
		// be exactly BEAMPACKETLENGTH bytes
		if (data.length != BEAMPACKETLENGTH) return;

		// First four bytes of the data package will
		// contain a magic value to identify that the
		// package is coming from our own application
		if (data[0] != (byte)'3' &&
			data[1] != (byte)'D' &&
			data[2] != (byte)'e' &&
			data[3] != (byte) 0) return;

        // Read the rotation parameters from the beam-received record
        tapDiffX =  getInt(data,  4);
        tapDiffY =  getInt(data,  8);
        object.setZ(getInt(data, 12));
		rotate(tapDiffY, -tapDiffX, 0);
	}

	/**
	 * Handle a pen down event.
	 */
	public void penDown(int x, int y) {
		// Check if user wants to exit
		if (exitButton.pressed(x,y)) {
			System.exit(0);
		}

		// Check if user wants to beam current position info
		if (beamButton.pressed(x,y)) {

		    byte[] data = new byte[BEAMPACKETLENGTH];

 			// First four bytes of the data package will
			// contain a magic value to identify that the
			// package is coming from our own application
			data[0]  = (byte)'3';
			data[1]  = (byte)'D';
			data[2]  = (byte)'m';
			data[3]  = (byte) 0;

            // Store the rotation parameters
            storeInt(tapDiffX,      data, 4);
            storeInt(tapDiffY,      data, 8);
            storeInt(object.getZ(), data, 12);
			
			// Beam the packet
			Spotlet.beamSend(data);
		    //**printBeamParams(data);    // for debugging use
		} else {
		    // Record pen tap location
		    tapX = x;
		    tapY = y;
		}

	}

	/**
	 * penMove: ignore it
	 * /

	/**
	 * Handle a pen up event.
	 */
	public void penUp(int x, int y) {

        if (tapX <= 0 || tapY <= 0) return;  // no penDown yet

		int diffX = x - tapX;
		int diffY = y - tapY;

		// If user tapped the screen without moving the pen,
		// reset the object
		if (diffX < 2 && diffX > -2 &&
			diffY < 2 && diffY > -2) {
			tapDiffX = 0;
			tapDiffY = 0;
		} else {
			tapDiffX += diffX;
			tapDiffY += diffY;
		}

		// Rotate object based on pen drag difference
		rotate(tapDiffY, -tapDiffX, 0);
		
		tapX = -1;  // wait for next penDown
	}

	/**
	 * Main: launch the application
	 */
    public static void main(String args[]) {
		(new ThreeDLogo()).register(NO_EVENT_OPTIONS);
    }

	/**
	 * getInt: get 4 bytes from an array as an int
	 */
    public static int getInt(byte[] data, int offset) {
		return ( data[offset+0]         << 24) |
			   ((data[offset+1] & 0xFF) << 16) | 
			   ((data[offset+2] & 0xFF) <<  8) | 
		       ((data[offset+3] & 0xFF)      ); 
	}
	
	/**
	 * storeInt: store an int as 4 bytes in an array
	 */
    public static void storeInt(int value, byte[] data, int offset) {
        data[offset+0] = (byte)(value >> 24);
        data[offset+1] = (byte)(value >> 16);
        data[offset+2] = (byte)(value >>  8);
        data[offset+3] = (byte) value       ;
	}
	
	/**
	 * printBeamParams: debugging routine
	 * /
    public void printBeamParams(byte[] data) {
		System.out.print((char)data[0]);
		System.out.print((char)data[1]);
		System.out.print((char)data[2]);
		System.out.print((int)data[3]);
		System.out.println(" diffX: " + getInt(data, 4));
		System.out.print  (" diffY: " + getInt(data, 8));
		System.out.println(" Z: "     + getInt(data,12));
    }//*/

}
