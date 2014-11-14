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
 * A scrolling TextBox object.
 *
 * You need to control this class from a registered Spotlet. In the
 * Spotlet class, implement penDown(), penMove() and keyDown() to 
 * call the handlePenDown(), handlePenMove() and handleKeyDown() 
 * methods of this class.
 *
 */

public class ScrollTextBox extends TextBox implements ScrollOwner {
    VerticalScrollBar vsb;

  int minVal;
  int maxVal;
  int curVal;
  int visibleLines;
  int numLines;

    protected ScrollTextBox() {
    }

    /**
     * Create a new ScrollTextBox object.
     *
     * @param t the initial text
     * @param x the X coordinate of the ScrollTextBox's position
     * @param y the Y coordinate of the ScrollTextBox's position
     * @param w the width
     * @param h the height
     */
    public ScrollTextBox(String t, int x, int y, int w, int h) {
        super(t, x, y, w - VerticalScrollBar.SCROLL_BAR_WIDTH, h);
	init();
        vsb = new VerticalScrollBar(this);
	vsb.setBounds(x+w-VerticalScrollBar.SCROLL_BAR_WIDTH,
                      y, h, 0, maxVal, curVal);
  }

    /**
     * Reset the display bounds of the ScrollTextBox.
     *
     * @param x the new X coordinate of the ScrollTextBox's position
     * @param y the new Y coordinate of the ScrollTextBox's position
     * @param w the new width
     * @param h the new height
     */
    public void setBounds(int x, int y, int w, int h) {
      if((x == xPos) && (y == yPos) && (w == width) && (h == height)) {
          // setting the bounds the same is a no-op.
          return;
      }
      super.setBounds(x, y, w-VerticalScrollBar.SCROLL_BAR_WIDTH, h);
      init();
      vsb.setBounds(x+w-VerticalScrollBar.SCROLL_BAR_WIDTH,
		    y, h, 0, maxVal, curVal);
    }

    /**
     * Set the text. You need to call paint() on the ScrollTextBox
     * to get the new text/scrollbar to display.
     *
     * @param t a String representing the new text.
     */
  public void setText(String t) {
    super.setText(t);
    init();
    // reset the scrollbar to the new size
    vsb.setBounds(xPos+width-VerticalScrollBar.SCROLL_BAR_WIDTH,
                  yPos, height, 0, maxVal, curVal);
  }

    /**
     * Initialize the object.
     */
  protected void init() {
	numLines = getNumLines();
	visibleLines = height/heightM;

	maxVal = numLines - visibleLines;
	minVal = 0;
	curVal = 0;

	if(maxVal < 0) {
	  maxVal = 0;
	}
  }

    /**
     * Is this point inside the bounds of the object?
     *
     * @param x the X coordinate of the position to test
     * @param y the Y coordinate of the position to test
     * @return true of the point is inside our bounds
     */
    public boolean contains(int x, int y) {
        return vsb.contains(x, y);
    }

    /**
     * The pen has gone down at (x, y).  Do the right thing.
     *
     * @param x the X coordinate of the pen position
     * @param y the Y coordinate of the pen position
     */
  public void handlePenDown(int x, int y) {
    vsb.handlePenDown(x,y);
  }
  
    /**
     * The pen has moved at (x, y).  Do the right thing.
     *
     * @param x the X coordinate of the pen position
     * @param y the Y coordinate of the pen position
     */
  public void handlePenMove(int x, int y) {
    vsb.handlePenMove(x, y);
  }

    /**
     * The user pressed a key.  Do the right thing.
     *
     * @param keyCode a code representing the key the user pressed
     */
    public void handleKeyDown(int keyCode) {
        vsb.handleKeyDown(keyCode);
    }

    /**
     * Paint the ScrollTextBox.
     */
  public void paint() {
        vsb.paint();
	
	g.setDrawRegion(xPos, yPos, width, height);
	
	g.drawRectangle(xPos, yPos, width, height, g.ERASE, g.SIMPLE);
	int x = xPos;
	int y = yPos;
	int first = 0;
	int last = 0;
	int numLines = (lineStarts.size() > curVal+visibleLines) ? curVal+visibleLines : lineStarts.size();
	for(int i=curVal; i<numLines; i++) {
	    first = lineStarts.valueAt(i);
	    last = lineEnds.valueAt(i);
	    if(first != last) {
		if(text.charAt(first) == ' ') {
		    first++;
		}
		g.drawString(text.substring(first, last), x, y);
	    }
	    y += heightM;
	}
	
	g.resetDrawRegion();
    
  }

    /**
     * Set the current scroll value and repaint.
     *
     * @param val the new scroll value.
     */
  public void setScrollValue(int val) {
    curVal = val;
    paint();
  }
}

