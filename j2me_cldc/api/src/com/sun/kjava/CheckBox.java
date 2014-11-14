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
 * A checkbox user interface object.
 *
 * A CheckBox object displays a check box next to a text label.
 * It has two states, checked and unchecked.
 */

public class CheckBox {
    static int outerWidth = 16;
    static int outerHeight = 10;

  static Bitmap notSelectedBitmap = new Bitmap((short)2, new byte[] {
    (byte)0x00, (byte)0x00,
    (byte)0xff, (byte)0x00,
    (byte)0x81, (byte)0x00,
    (byte)0x81, (byte)0x00,
    (byte)0x81, (byte)0x00,
    (byte)0x81, (byte)0x00,
    (byte)0x81, (byte)0x00,
    (byte)0x81, (byte)0x00,
    (byte)0xff, (byte)0x00,
  });

  static Bitmap selectedBitmap = new Bitmap((short)2, new byte[]  {
    (byte)0x00, (byte)0xc0,
    (byte)0xfd, (byte)0xc0,
    (byte)0x83, (byte)0x80,
    (byte)0xb7, (byte)0x00,
    (byte)0xbe, (byte)0x00,
    (byte)0x9d, (byte)0x00,
    (byte)0x89, (byte)0x00,
    (byte)0x81, (byte)0x00,
    (byte)0xff, (byte)0x00,
  });
  
    static Graphics g = Graphics.getGraphics();

    boolean isSelected;
    int xPos;
    int yPos;
    String title;

    /**
     * Create a new checkbox at an undefined position with no text
     * label.
     */
    public CheckBox() {
      this(-1, -1, null);
    }

    /**
     * Create a new checkbox at a given position with a text label.
     *
     * @param x the X coordinate of position.
     * @param y the Y coordinate of position.
     * @param text label of the CheckBox
     */
    public CheckBox(int x, int y, String text) {
	xPos = x;
	yPos = y;
	isSelected = false;
	title = text;
    }

    /**
     * Set the CheckBox's position.
     *
     * @param x the X coordinate of position.
     * @param y the Y coordinate of position.
     */
    public void setLocation(int x, int y) {
	xPos = x;
	yPos = y;
    }

    /**
     * Set the CheckBox's label.
     */
    public void setText(String text) {
	title = text;
    }

    /**
     * Paint the CheckBox.
     */
    public void paint() {
	if(isSelected) {
	  g.drawBitmap(xPos, yPos, selectedBitmap);
	} else {
	  g.drawBitmap(xPos, yPos, notSelectedBitmap);
	}
	g.drawString(title, xPos+outerWidth, yPos-1);
    }

    /**
     * The user selected the CheckBox; invert its state.
     * If it was checked, set the state to unchecked, and <i>vice-versa.</i>
     * This will cause the CheckBox to redraw itself.
     */
    public void handlePenDown(int x, int y) {
      this.setState(!isSelected);
    }

    /**
     * Did the user's "press" fall within the CheckBox?
     *
     * @param x the X coordinate of the user's press
     * @param y the Y coordinate of the user's press
     * @return true if <code>(x, y)</code> fall within bounds
     */
    public boolean pressed(int x, int y) {
	if((x >= xPos) && (x <= (xPos+outerWidth)) &&
	   (y >= yPos) && (y <= (yPos+outerHeight))) {
	    return true;
	}
	return false;
    }

    /**
     * Set the state and redraw to reflect it.
     *
     * @param state the new state
     */
    public void setState(boolean state) {
	isSelected = state;
	paint();
    }

}

