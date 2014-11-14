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
 
package com.sun.kjava;


/**
 * An object that presents a user interface for integer value selection.
 *
 * <p>It contains three Buttons:
 * <ul>
 * <li>A decrement ("-") Button
 * <li>An increment ("+") Button
 * <li>A random value ("?") Button
 * </ul>
 */
public class ValueSelector {
	String label;
	int value, min, max;
	Button plusButton, subButton, randButton;
	int posX;
	int posY;
	int valuePosX;
	int valuePosY;

    /**
     * Create a new ValueSelector.
     *
     * @param label the label for the ValueSelector
     * @param min minimum value to allow
     * @param max maximum value to allow
     * @param init initial value
     * @param x the X coordinate of our position
     * @param y the Y coordinate of our position
     */
	public ValueSelector(String label,int min,int max,int init,int x,int y) {
		this.label = label;
		this.value = init;
		this.min   = min;
		this.max   = max;
		this.posX  = x;
		this.posY  = y;

		int width = Graphics.getWidth(label);
		valuePosX = posX+width+3;
		valuePosY = posY;

		subButton  = new Button(" -  ",x+width+15,y);
		plusButton = new Button(" + ",x+width+31,y);
		randButton = new Button(" ? ",x+width+47,y);
		
	}

    /**
     * Paint the ValueSelector.
     */
	public void paint() {

		Graphics g = Graphics.getGraphics();

		g.drawString(label,posX,posY);
		plusButton.paint();
		subButton.paint();
		randButton.paint();

		String maxStr = String.valueOf(max);
		g.drawRectangle(valuePosX,valuePosY,g.getWidth(maxStr),
			g.getHeight(maxStr),g.ERASE,0);
		g.drawString(String.valueOf(value),valuePosX,valuePosY);
	}	
		
    /**
     * What's the current value?
     *
     * @return the current value
     */
	public int getValue() { 
	    return value; 
	}

    /**
     * Set the current value.
     *
     * @param value the value to set
     */
	public void setValue(int value) { 
	    this.value = value; 
	}

    /**
     * If one of the Buttons was pressed, have it deal with it.
     *
     * @param x the X coordinate of the user's press
     * @param y the Y coordinate of the user's press
     * @return true if the position was handled by one of the Buttons
     */
	public boolean pressed(int x, int y) {
	    if (plusButton.pressed(x,y)) {
		if (value < max) {
		    value++; 
		    paint(); 
		}
		return true;
	    } else if (subButton.pressed(x,y)) {
		if (value > min) {
		    value--;
		    paint();
		}
		return true;
	    } else if (randButton.pressed(x,y)) {
		java.util.Random random = new java.util.Random();
		value = ((random.nextInt()>>>1) % (max - min)) + min;
		paint();
		return true;
	    }
	    return false;
	}
}

