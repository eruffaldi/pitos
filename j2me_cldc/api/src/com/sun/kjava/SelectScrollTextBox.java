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

public class SelectScrollTextBox extends ScrollTextBox {
    
    public static final int LEADING = 2;

    public SelectScrollTextBox(String t, int x, int y, int w, int h) {
        super(t, x, y, w, h);
    }
    
    public String getSelection(int x, int y) {
        int yOffset = y - yPos;
        int row = yOffset / heightM;
        int rem = yOffset % heightM;
        int lineWidth = width-VerticalScrollBar.SCROLL_BAR_WIDTH;
        
        if ( x < xPos ||
             x >= (xPos+lineWidth) ||
             row < 0 ||
             row >= visibleLines ||
             rem < LEADING || 
             rem >= (heightM - LEADING)) {
            return null;
        }
        y = row * heightM;
	    Graphics.drawRectangle(xPos, yPos + y, xPos + lineWidth,
	                    heightM, Graphics.INVERT, 0);
        row += curVal;
    	return text.substring(lineStarts.valueAt(row),
    	                      lineEnds.valueAt(row));
    }
    
    public void setText(String t) {
        super.setText(t);
        vsb.setBounds(xPos+width-VerticalScrollBar.SCROLL_BAR_WIDTH,
		    yPos, height, 0, maxVal, curVal = 0);
    }
}


