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
 * A vertical scroll bar user interface object.
 */
public class VerticalScrollBar {
    static Bitmap upArrow = new Bitmap((short)1, new byte[] {
	(byte) 0x08,
	(byte) 0x1C,
	(byte) 0x3E,
	(byte) 0x7F,});

    static Bitmap downArrow = new Bitmap((short)1, new byte[] {
	(byte) 0x7F,
	(byte) 0x3E,
	(byte) 0x1C,
	(byte) 0x08,
    });

    public static int SCROLL_BAR_WIDTH = 8;

    int xPos;
    int yPos;
    int height;
    Graphics g = Graphics.getGraphics();
    int arrowHeight;
    int minVal;
    int maxVal;
    int curVal;
    int unit;
    ScrollOwner owner;

    int scrollHandleHeight;


    /**
     * Create a new VerticalScrollBar and associate it with an owner.
     *
     * @param so the ScrollOwner that owns this scroll bar.
     */
  public VerticalScrollBar(ScrollOwner so) {
    owner = so;
  }

    /**
     * Create a new VerticalScrollBar and associate it with an owner.
     *
     * @param so the ScrollOwner that owns this scroll bar.
     * @param x the X coordinate of the scroll bar
     * @param y the Y coordinate of the scroll bar
     * @param h the height of the scroll bar
     * @param min the minimum value allowed
     * @param max the maximum value allowed
     * @param initVal the initial value
     */
    public VerticalScrollBar(ScrollOwner so, int x, int y, int h, int min, int max, int initVal) {
        owner = so;
	init(x, y, h, min, max, initVal);
    }

    /**
     * Set the scroll bar's bounds.
     *
     * @param x the X coordinate of the scroll bar
     * @param y the Y coordinate of the scroll bar
     * @param h the height of the scroll bar
     * @param min the minimum value allowed
     * @param max the maximum value allowed
     * @param initVal the initial value
     */
  public void setBounds(int x, int y, int h, int min, int max, int initVal) {
	init(x, y, h, min, max, initVal);
  }

    /**
     * Initialize the scroll bar.
     *
     * @param x the X coordinate of the scroll bar
     * @param y the Y coordinate of the scroll bar
     * @param h the height of the scroll bar
     * @param min the minimum value allowed
     * @param max the maximum value allowed
     * @param initVal the initial value
     */
  protected void init(int x, int y, int h, int min, int max, int initVal) {
	xPos = x;
	yPos = y;
	height = h;
	arrowHeight = upArrow.getRows();
	minVal = min;
	maxVal = max;
	curVal = initVal;
	if((maxVal == 0) && (minVal == 0)) {
	  unit = height - (2*arrowHeight) -2;
	} else {
	  unit = (height - (2*arrowHeight) -4) / ((maxVal - minVal));
	}
	scrollHandleHeight = unit;
	/*
	if(scrollHandleHeight<8) {
	    scrollHandleHeight = 8;
	}
	if(scrollHandleHeight>30) {
	    scrollHandleHeight = 30;
	}
	*/
  }

    /**
     * Does the scroll bar contain the point in question?
     *
     * @param x the X coordinate to test
     * @param y the Y coordinate to test
     * @return true  if the point is within the scroll bar's bounds
     */
    public boolean contains(int x, int y) {
	if ((x > xPos) && (x < xPos+SCROLL_BAR_WIDTH) &&
	    (y > yPos) && (y < yPos + height)) {
	    return true;
	}
	return false;
    }

    /**
     * Deal with the fact that the pen moved.
     *
     * @param x the X coordinate of the pen's position
     * @param y the Y coordinate of the pen's position
     */
    public void handlePenMove(int x, int y) {
	if(y<yPos+arrowHeight) {
	    if(curVal > minVal) {
		curVal--;
	    } else {
                return;
            }
	} else if (y > yPos + height - (2*arrowHeight)) {
	    if(curVal < maxVal) {
		curVal++;
	    } else {
                return;
            }
	} else {
	  int newCurVal = ((y-(yPos+arrowHeight+1)) / unit) + minVal; 
	  if(newCurVal > maxVal) {
	    newCurVal = maxVal;
	  }
	  if(newCurVal == curVal) {
	    return;
	  } else {
	    curVal = newCurVal;
	  }	  
	} 
        owner.setScrollValue(curVal);
    }

    /**
     * The user pressed a key.  Deal with it.
     *
     * @param keyCode the code of the key the user pressed
     */
    public void handleKeyDown(int keyCode) {
        switch(keyCode) {
        case Spotlet.PAGEUP:
	    if(curVal > minVal) {
		curVal--;
	    } else {
                return;
            }           
            break;
        case Spotlet.PAGEDOWN: 
	    if(curVal < maxVal) {
		curVal++;
	    } else {
                return;
            }
            break;
        }
        owner.setScrollValue(curVal);
    }

    /**
     * The pen went down somewhere.  Deal with it.
     *
     * @param x the X coordinate of the pen's position
     * @param y the Y coordinate of the pen's position
     */
    public void handlePenDown(int x, int y) {
	this.handlePenMove(x, y);
    }


    /**
     * Paint the VerticalScrollBar.
     */
    public void paint() {
	g.drawBitmap(xPos+1, yPos, upArrow);
	g.drawBitmap(xPos+1, yPos+height-arrowHeight, downArrow);
        g.drawLine(xPos+4, yPos+arrowHeight+1, xPos+4, yPos+height-arrowHeight-2, g.GRAY);
        g.drawLine(xPos+5, yPos+arrowHeight+2, xPos+5, yPos+height-arrowHeight-3, g.GRAY);
        g.drawLine(xPos+6, yPos+arrowHeight+1, xPos+6, yPos+height-arrowHeight-2, g.GRAY);

	int handlePos = ((curVal - minVal) * unit) + yPos + arrowHeight+1;
	if((handlePos + scrollHandleHeight) > (yPos+height-(arrowHeight+1))) {
	  handlePos = yPos+height-(arrowHeight+1)-scrollHandleHeight;
	}
	g.drawRectangle(xPos+4, handlePos, 3, scrollHandleHeight, g.PLAIN, g.SIMPLE);
    }

}

