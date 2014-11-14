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
 * An object representing a group of RadioButtons.  At most one
 * RadioButton in a RadioGroup can be selected at one time.
 *
 * @see RadioButton
 */
public class RadioGroup {
    
    RadioButton[] buttons;
    int curElement;
    int numElements;

    /**
     * Create a new RadioGroup.
     *
     * @param numButtons the number of RadioButtons it will contain
     */
    public RadioGroup(int numButtons) {
	buttons = new RadioButton[numButtons];
	curElement = 0;
    }
    
    /**
     * Add a RadioButton to the RadioGroup.
     *
     * @param theButton the RadioButton to add
     */
    public void add(RadioButton theButton) {
	if(curElement < buttons.length) {
	    buttons[curElement++] = theButton;
	    theButton.setParent(this);
	}
    }

    /**
     * Get the RadioButton at an index.
     *
     * @param i the index of the RadioButton to return
     * @return the requested RadioButton
     */
    public RadioButton buttonAt(int i) {
	return buttons[i];
    }

    /**
     * Set the currently-selected RadioButton.  Clear the old selection.
     *
     * @param theButton the RadioButton to select
     */
    public void setSelected(RadioButton theButton) {
	RadioButton tmpButton;
	// deselect other ones
	for(int i=0; i<curElement; i++) {
	    tmpButton = buttons[i];
	    tmpButton.setState(tmpButton == theButton);
	} 
    }

    /**
     * Get the currently selected RadioButton.
     *
     * @return the currently selected RadioButton
     */
    public RadioButton getSelected() {
	for(int i=0; i<curElement; i++) {
	    if(buttons[i].isSelected()) {
		return buttons[i];
	    }
	} 
	return null;
    }

    /**
     * Is any one of the RadioButtons in the group selected?
     *
     * @return true if one of the RadioButtons in the group is selected.
     */
    public boolean hasSelection() {
	for(int i=0; i<curElement; i++) {
	    if(buttons[i].isSelected()) {
		return true;
	    }
	} 
	return false;
    }

    /**
     * How many RadioButtons in this group?
     *
     * @return the number of RadioButtons in the group
     */
    public int size() {
	return curElement;
    }

}

