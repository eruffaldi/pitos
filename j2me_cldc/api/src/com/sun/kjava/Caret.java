/*
 * Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 * Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 * Sun Microsystems, Inc. has intellectual property rights relating
 * to the technology embodied in this software.  In particular, and
 * without limitation, these intellectual property rights may include
 * one or more U.S. patents, foreign patents, or pending
 * applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 * and all Sun-based and Java-based marks are trademarks or
 * registered trademarks of Sun Microsystems, Inc.  in the United
 * States and other countries.
 *
 * This software is distributed under licenses restricting its use,
 * copying, distribution, and decompilation.  No part of this
 * software may be reproduced in any form by any means without prior
 * written authorization of Sun and its licensors, if any.
 *
 * FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 * Subject to Standard License Terms and Conditions
 */

package com.sun.kjava;

/** 
 * Class Caret implements a caret ("|") for use as a marker for the
 * current insertion point in a TextField.
 *
 * (Caret should probably be a private class, since it has no use independent 
 * of TextField.)
 */

public class Caret extends Thread {
    Graphics g = Graphics.getGraphics();
    int displayDelay;
    int xPos;
    int yPos;
    int drawMode = g.PLAIN;
    public boolean blinking, stop;
    boolean isVisible = false;
    /**
     * Create a Caret at a position, blinking at a given rate.
     *
     * @param x X coordinate of position
     * @param y Y coordinate of position
     * @param delay delay between blinks, in milliseconds
     */
    public Caret(int delay, int x, int y) {
        super();
	displayDelay = delay;
	xPos = x;
	yPos = y;
    }
    
    /**
     * Set the Caret's position.
     *
     * @param x new X coordinate
     * @param y new Y coordinate
     */
    public void setPosition(int x, int y) {
        if(isVisible) {
            drawCaret(g.ERASE);
            xPos = x;
            yPos = y;
            drawCaret(g.PLAIN);
        }  else {
            xPos = x;
            yPos = y;
        }
  
    }

    public void eraseCaret() {
        drawCaret(g.ERASE);
        isVisible = false;
    }

    /**
     * Draw the Caret at its current position.
     *
     * @param drawMode mode in which to draw
     */
    public void drawCaret(int drawMode) {
	g.drawLine(xPos, yPos, xPos, yPos+10, drawMode);
    }
    
    /**
     * Run: flash the Caret at the prescribed rate.
     */
    public void run() {
	while (!stop) {
	    if(!blinking) {
          if(isVisible) {
              eraseCaret();
          }
          yield();
          continue;
	    }
	    
	    drawCaret(drawMode);
	    if(drawMode == g.PLAIN) {
          drawMode = g.ERASE;
          // it's actually visible now (we just painted it)
          isVisible = true;
	    } else {
          drawMode = g.PLAIN;
          // it's actually not visible now (we just erased it)
          isVisible = false;
	    }
	    try {
	        sleep(displayDelay);
            } catch(InterruptedException ex) {}
	}
  if(isVisible)
      eraseCaret();
    }	
}

