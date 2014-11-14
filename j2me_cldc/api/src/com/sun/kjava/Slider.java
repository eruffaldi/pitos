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
 * Slider: A graphical valuator object.
 *
 * Allows user to select a value by sliding a marker on a scale. This
 * class isn't very graceful about handling conditions where the width
 * of the slider is less than the interval of the maximum and minimum
 * values. It calculates a "skip" value in these cases to increment the
 * value for each pixel on the screen, e.g.
 *
 * Slider s1 = new Slider(5, 100, 100, 0, 1000, 0) creates a slider
 * 100 pixels wide to handle the interval 0->1000. It then treats each
 * pixel as being 10 units, and the user can only generate values in
 * multiples of 10.
 * 
 */

public class Slider {

  Graphics g;

  int xPos;
  int yPos;
  int width;
  int height = 10;
  int minVal;
  int maxVal;
  int curVal;
  int unit;
  int step;
  static int strWidth = Graphics.getWidth("Value:");
  static int strHeight = Graphics.getHeight("Value:");

    /**
     * Create a Slider object.
     *
     * @param x the X coordinate of the Slider's position
     * @param y the Y coordinate of the Slider's position
     * @param w the width
     * @param mn the minimum value
     * @param mx the maximum value
     * @param initVal the initial value
     */
  public Slider(int x, int y, int w, int mn, int mx, int initVal) {
    xPos = x;
    yPos = y;
    width = w;
    minVal = mn;
    maxVal = mx;
    curVal = initVal;
    g = Graphics.getGraphics();
    unit = width / ((maxVal - minVal)+1);
    step = -1;
    if(unit == 0) {
        unit = 1;
        step = (maxVal - minVal)/width;
    }
  }

    /**
     * Create a new Slider object.
     */
    public Slider() {
	g = Graphics.getGraphics();
    }

    /**
     * Set the position of the Slider.
     *
     * @param x the new X coordinate
     * @param y the new Y coordinate
     */
    public void setLocation(int x, int y) {
	xPos = x;
	yPos = y;
    }

    /**
     * Reset the width, limits, and value of the Slider.
     *
     * @param w the new width
     * @param mn the new minimum value
     * @param mx the new maximum value
     * @param val the new current value
     */
    public void setSizeRange(int w, int mn, int mx, int val) {
	width = w;
	minVal = mn;
	maxVal = mx;
	curVal = val;
	unit = width / ((maxVal - minVal)+1);
  step = -1;
  if(unit == 0) {
      unit = 1;
      step = (maxVal - minVal)/ width;
  }
    }

    /**
     * Draw the Slider.
     */
  public void paint() {
    drawMarker(g.PLAIN);
    g.drawString(""+minVal, xPos, yPos+height+5, g.ERASE);
    g.drawString(""+maxVal, xPos+width-10, yPos+height+5, g.ERASE);
  }

    /**
     * Draw the Slider's marker.
     *
     * @param drawStyle the style in which to draw it.
     */
  public void drawMarker(int drawStyle) {
    // draw the block marker, the border and the value string
    int theVal = curVal;
    if(step != -1) {
        theVal /= step;
    }
    int markerPos = ((theVal-minVal) * unit) + xPos;
    if(unit > 1) {
        g.drawRectangle(markerPos, yPos, unit, height, drawStyle, g.SIMPLE);
    } else {
        g.drawLine(markerPos, yPos, markerPos, yPos + height, drawStyle);
    }
    g.drawBorder(xPos, yPos+3, width, height-6, g.PLAIN, g.SIMPLE);
    if(drawStyle == g.ERASE) {
      g.drawRectangle(xPos+20+strWidth, yPos+height+5, strWidth, strHeight, drawStyle, g.SIMPLE);
    } else {
      g.drawString("Value:" + curVal, xPos+20, yPos+height+5, drawStyle);
    }
  }

    /**
     * Deal with the fact that the pen moved.
     *
     * @param x the X coordinate of the pen's new position
     * @param y the Y coordinate of the pen's new position
     */
    public void handlePenMove(int x, int y) {
	int newCurVal;
  
  if(step == -1) {
      newCurVal = ((x-xPos) / unit) + minVal;
  } else {
      newCurVal = (((x-xPos) / unit) + minVal) * step;
  }
	if(newCurVal != curVal) {
	    drawMarker(g.ERASE);
	}
	curVal = newCurVal;
	paint();
    }

    /**
     * Deal with the fact that the pen went down.
     *
     * @param x the X coordinate of the pen's new position
     * @param y the Y coordinate of the pen's new position
     */
  public void handlePenDown(int x, int y) {
    this.handlePenMove(x, y);
  }


    /**
     * Is this point within the Slider's bounds?
     *
     * @param x the X coordinate to test
     * @param y the Y coordinate to test
     * @return true if the point is in bounds, false otherwise
     */
    public boolean contains(int x, int y) {
	if((x > xPos) && (x < xPos+width) && 
	   (y > yPos) && (y < yPos+height)) {
	    return true;	    
	}
	return false;
    }
}

