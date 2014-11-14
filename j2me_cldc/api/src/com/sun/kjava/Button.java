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
 * Button: a simple button user interface object.
 *
 * Note that this button causes actions to occur when it
 * it pressed, not when it is released. Therefore it is
 * currently impossible for a user to cancel a button selection
 * once it has started!
 *
 * Bitmap buttons do not have a border drawn around them. If you
 * want your bitmap button to have a border, include the border
 * in the bitmap.
 * 
 */

public class Button {
  public static final int minWidth = 4;

  int posX, posY;	// position of the button
  String text;		// text of button
  Bitmap bmp;
  int width, height;	// bounding frame of text
  int strWidth;
  int borderType;
  boolean isEnabled = true;

    /**
     * Create a new Button object with a text label.
     *
     * @param s the button's text label
     * @param x the x coordinate of the button's location
     * @param y the y coordinate of the button's location
     */
  public Button(String s, int x, int y) {
    this.text = s;
    this.posX = x;
    this.posY = y;
    strWidth = Graphics.getWidth(s);
    width = strWidth + 4;
    if(width < minWidth) {
      width = minWidth;
    }
    height = Graphics.getHeight(s) + 2;
    this.borderType = Graphics.borderType(3, 0, 1);
  }

    /**
     * Create a new Button object with graphical label.
     *
     * @param s the button's text label
     * @param x the x coordinate of the button's location
     * @param y the y coordinate of the button's location
     */
  public Button(Bitmap bitmap, int x, int y) {
    this.bmp = bitmap;
    this.posX = x;
    this.posY = y;
    width = bmp.getWidth();
    height = bmp.getRows();
  }

    /**
     * Set the Button's text label.
     *
     * @param s the new label for the button.
     */
  public void setText(String s) {
    this.text = s;
    strWidth = Graphics.getWidth(s);
    width = strWidth+4;
    if(width < minWidth) {
      width = minWidth;
    }
    height = Graphics.getHeight(s)+2;
  }

    /**
     * Set whether the Button allows input (is "enabled").
     *
     * @param state if true, Button allows input.
     */
    public void setEnabled(boolean state) {
	isEnabled = state;
    }

    /**
     * Is the Button enabled?
     *
     * @return true if the Button accepts input, false if not.
     */
    public boolean isEnabled() {
	return isEnabled;
    }

    /**
     * Paint the Button on the global Graphics context.
     * If the Button is not enabled, it draws in a "grayed out" style.
     */
  public void paint() {
    Graphics g = Graphics.getGraphics();

    if (text != null) {
	int drawStyle = isEnabled ? g.SIMPLE : g.GRAY;
      int xOffset = (width - strWidth)/2;
      g.drawString(text,posX+xOffset,posY+1, drawStyle);
      g.drawBorder(posX,posY,width,height,drawStyle,borderType);
    }
    else
      g.drawBitmap(posX,posY,bmp);
    //            g.drawBorder(posX,posY,width,height,g.PLAIN,g.SIMPLE);
  }


    /**
     * Was the button pressed?  If the coordinates are within the Button,
     * give the user some feedback.
     *
     * @return true if the coordinates were within the bounds of the Button.
     */
  public boolean pressed(int x,int y) {
      if(!isEnabled) {
	  return false;
      }
    if (posX <= x && x <= posX+width && posY <= y && y <= posY+height) {
      Graphics g = Graphics.getGraphics();
      g.drawRectangle(posX, posY, width, height, g.INVERT, 0);
      try {
          Thread.sleep(100);
      } catch(InterruptedException ex) {}
      g.drawRectangle(posX, posY, width, height, g.INVERT, 0);
      return true;
    }
    
    return false;
  }
}


