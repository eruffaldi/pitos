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
 * A two-state button meant as part of a group, only one of which can be
 * "on" at one time.
 *
 * @see RadioGroup
 */
public class RadioButton {
    static int outerFrame = Graphics.borderType(4, 0, 1);
    static int innerFrame = Graphics.SIMPLE;
    static int outerWidth = 8;
    static int outerHeight = 8;
    static int innerWidth = 4;
    static int innerHeight = 4;

    static Graphics g = Graphics.getGraphics();

    boolean isSelected;
    int xPos;
    int yPos;
    RadioGroup parent;
    String title;

    /**
     * Create a new RadioButton.
     */
    public RadioButton() {
	isSelected = false;
	xPos = -1;
	yPos = -1;
	title = null;
	parent = null;
    }

    /**
     * Create a new RadioButton.
     *
     * @param x the X coordinate of the RadioButton's position
     * @param y the Y coordinate of the RadioButton's position
     * @param text the label for the button
     */
    public RadioButton(int x, int y, String text) {
	xPos = x;
	yPos = y;
	isSelected = false;
	title = text;
    }

    /**
     * Set the position of the RadioButton.
     *
     * @param x the X coordinate of the RadioButton's position
     * @param y the Y coordinate of the RadioButton's position
     */
    public void setLocation(int x, int y) {
	xPos = x;
	yPos = y;
    }

    /**
     * Set the label of the button.
     *
     * @param text the new text of the label
     */
    public void setText(String text) {
	title = text;
    }

    /**
     * Get the label of the button.
     *
     * @return the text of the label
     */
    public String getText() {
	return title;
    }

    /**
     * Paint the RadioButton on the screen.
     */
    public void paint() {
	int drawFill = g.ERASE;
	if(isSelected) {
	    drawFill = g.PLAIN;
	}
	g.drawRectangle(xPos+2, yPos+2, innerWidth, innerHeight, drawFill, innerFrame);
	g.drawBorder(xPos, yPos, outerWidth, outerHeight, g.PLAIN, outerFrame);
	g.drawString(title, xPos+outerWidth+3, yPos-1);
    }

    /**
     * The pen has gone down in the button.  Handle making or removing the
     * selection.
     *
     * @param x the X coordinate of the RadioButton's position
     * @param y the Y coordinate of the RadioButton's position
     */
    public void handlePenDown(int x, int y) {
	if(!isSelected) {
	    if(parent != null) {
		parent.setSelected(this);
	    } else {
		this.setState(!isSelected);
	    }
	}
	paint();
    }

    /**
     * Did the user press inside the RadioButton?
     *
     * @param x the X coordinate of the RadioButton's position
     * @param y the Y coordinate of the RadioButton's position
     * @return true if the coordinates are within the area, false otherwise.
     */
    public boolean pressed(int x, int y) {
	if((x >= xPos) && (x <= (xPos+outerWidth)) &&
	   (y >= yPos) && (y <= (yPos+outerHeight))) {
	    return true;
	}
	return false;
    }

    /**
     * Set the parent RadioGroup of this button.
     *
     * @param rg the parental RadioGroup
     */
    public void setParent(RadioGroup rg) {
	parent = rg;
    }

    /**
     * Set the state of the button.
     *
     * @param state the new state; <code>true</code> means "selected"
     */
    public void setState(boolean state) {
	isSelected = state;
	paint();
    }

    /**
     * Is this RadioButton currently selected?
     *
     * @return true if selected, false if not
     */
    public boolean isSelected() {
	return isSelected;
    }
}

