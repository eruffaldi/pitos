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
 * A box displaying text on the screen. This class flows the
 * text in the box. It doesn't break words, and therefore isn't
 * graceful handling words larger than the width of the box.
 */
public class TextBox {
    protected String text;
    protected IntVector lineStarts = new IntVector();
    protected IntVector lineEnds = new IntVector();
    protected int xPos, yPos, width, height;
    protected Graphics g = Graphics.getGraphics();
    protected static int widthM = Graphics.getWidth("e");
    protected static int heightM = Graphics.getHeight("E");

    /**
     * Create a new TextBox object.
     */
    public TextBox() {	
    }

    /**
     * Create a new TextBox object.
     *
     * @param t the initial text
     * @param x the X coordinate of the ScrollTextBox's position
     * @param y the Y coordinate of the ScrollTextBox's position
     * @param w the width
     * @param h the height
     */
    public TextBox(String t, int x, int y, int w, int h) {
	xPos = x;
	yPos = y;
	width = w;
	height = h;
	text = t;
	if (text != null) {
	    computeLineBreaks2();
	}
    }

    /**
     * How many lines of text does the TextBox currently hold?
     *
     * @return the number of lines of text contained
     */
    public int getNumLines() {
	return lineStarts.size();
    }

    /**
     * Set the text. You need to call paint() on the TextBox to
     * get the new text displayed.
     *
     * @param t a String representing the new text.
     */
    public void setText(String t) {
	text = t;
	lineStarts.removeAllElements();
	lineEnds.removeAllElements();
	if(text!=null) {
	    computeLineBreaks2();
	}
    }

    /**
     * Reset the display bounds of the TextBox.
     *
     * @param x the new X coordinate of the ScrollTextBox's position
     * @param y the new Y coordinate of the ScrollTextBox's position
     * @param w the new width
     * @param h the new height
     */
    public void setBounds(int x, int y, int w, int h) {
	xPos = x;
	yPos = y;
	width = w;
	height = h;
	setText(text);
    }

    private void computeLineBreaks2() {
	int start = 0;
	int lastSpace = 0;
	int pos = 0;
	char ch;
	int strWidth;

        // This is (hopefully) a faster strategy than computeLineBreaks.
        // Instead of computing character widths for every character
        // we just do it on each ' ' or '\n' in the string. 
        // NOTE: this still isn't perfect - we don't handle a word wider
        // than our width very well (in fact, not at all).


	while(pos < text.length()) {
	    ch = text.charAt(pos);
	    if(ch == ' ' || ch == '\n') {
		strWidth = g.getWidth(text.substring(start, pos));
		if( strWidth > width) {
		    // break the line
		    lineStarts.append(start);
		    lineEnds.append(lastSpace);
		    start = lastSpace+1;
		    lastSpace = lastSpace;
		} else if(strWidth == width) {
		    // break here
		    lineStarts.append(start);
		    lineEnds.append(pos);
		    start = pos;
		    lastSpace = pos;
		} else if(ch == ' ') {
		    lastSpace = pos;
		}
                if(ch == '\n') {
                    // break the line
                    lineStarts.append(start);
                    lineEnds.append(pos);
                    start = pos+1;
                    lastSpace = pos+1;
                }
            }
            pos++;
	}
        // there may not have been a space at the end of the string.
        // we need to check if we have to break the last line.
        strWidth = g.getWidth(text.substring(start, pos));
        if( strWidth > width) {
            // break the line
            lineStarts.append(start);
            lineEnds.append(lastSpace);
            start = lastSpace+1;
            lastSpace = lastSpace;
        }
	lineStarts.append(start);
	lineEnds.append(pos);
	//	g.drawString("numbreaks " + lineStarts.size(), 0,0);
    }

    private void computeLineBreaks() {
	int start = 0;
	int lastBreak = 0;
	int pos = 0;
	int strWidth = 0;
	int charWidth;
	int lastSpace = 0;
	char ch;
	while(pos < text.length()) {
	    ch = text.charAt(pos);
	    // keep track of the last space to back out to
	    if(ch == ' ') lastSpace = pos;
	    // check for '\n' in the text
	    if(ch == '\n')  {
		// forced line break.
		lineStarts.append(start);
		lineEnds.append(pos);
		lastSpace = pos+1;
		start = pos+1;
		strWidth = 0;
	    } else {	 
		charWidth = Graphics.getWidth(ch+"");
		// check for a break due to width
		if(strWidth+charWidth > width) {
		    // insert linebreak
		    lineStarts.append(start);
		    lineEnds.append(lastSpace);
		    start = lastSpace+1;
		    pos = lastSpace;
		    strWidth = 0;
		} else {
		    strWidth += charWidth;
		}
	    }
	    pos++;
	}
	lineStarts.append(start);
	lineEnds.append(pos);
    }

    /**
     * Paint the TextBox on the screen.
     */
    public void paint() {
	g.setDrawRegion(xPos, yPos, width, height);
	
	int x = xPos;
	int y = yPos;
	int first = 0;
	int last = 0;
	for(int i=0; i<lineStarts.size(); i++) {
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
}

