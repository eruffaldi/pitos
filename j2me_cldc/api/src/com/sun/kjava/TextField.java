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
 * This class provides a simple TextField. It creates a thread for
 * the caret to blink, accepts key input (including delete and 
 * backspace) and allows for only upper case entry.
 *
 * At present there is no support for Pen selection at all.
 *
 * It needs to be used in conjunction with a Spotlet, as this
 * class does not extend Spotlet and therefore has no event
 * handling itself. You need to get the Spotlet keyDown()
 * method to call this class's handleKeyDown() method.
 *
 * After construction, to get the field "working" call setFocus()
 * this will start the caret. Call loseFocus() to stop the caret
 * when it's all over.
 * *
 */
public class TextField {
    Graphics g = Graphics.getGraphics();
    String title;
    int xPos;
    int yPos;
    int width;
    int height;
    int caretPos;
    int titleWidth;
    int titleHeight;
    int startText;
    int textWidth;
    Caret caret;
    String text;
    boolean hasFocus;
    int maxLen;
    boolean onlyUpperCase = false;

    /**
     * Create a new TextField
     *
     * @param ttext The title (label) for the text field
     * @param x x position (upper left)
     * @param y y position (upper left)
     * @param w width (including label)
     * @param h height
     */
    public TextField(String ttext, int x, int y, int w, int h) {
        xPos = x;
        yPos = y;
        width = w;
        height = h;
        title = ttext + ":";
        titleWidth = g.getWidth(title) + 1;  // extra space
        titleHeight = g.getHeight(title);
        startText = xPos + titleWidth;
        textWidth = width - titleWidth;        
        caretPos = startText;
        caret = new Caret(150, caretPos, yPos);
        hasFocus = false;
        text = new String("");
    }

    /**
     * Set whether or not the textfield should convert 
     * everything to upper case
     *
     * @param flag if true then convert chars to upper case
     */
    public void setUpperCase(boolean flag) {
        onlyUpperCase = flag;
    }

    /*
     * Paint the textfield which includes the label, 
     * and a gray (dotted) line for the text entry area.
     *
     */
    public void paint() {
        // erase previous title & text
        g.drawRectangle(xPos, yPos, width, height, g.ERASE, g.SIMPLE);
        // draw the title
        g.drawString(title, xPos, yPos);
        // draw the "...."	
        g.drawLine(startText, yPos+titleHeight+2, startText+textWidth, yPos+titleHeight+2, g.GRAY); 
        // and the text
        g.setDrawRegion(startText, yPos, textWidth, height);
        g.drawString(text, startText, yPos);
        g.resetDrawRegion();
    }

    /**
     * Stops the caret thread.
     */
    public void killCaret() {
        caret.stop = true;
    }

    /**
     * Give the textfield "focus". The registered Spotlet actually
     * has focus. This method kicks off the caret thread to get
     * the caret to blink.
     *
     */
    public void setFocus() {
        hasFocus = true;
        caret.setPosition(caretPos, yPos);
        caret.blinking = true;
        if(!caret.isAlive()) {
            caret.start();
        }
    }

    /**
     * Stops the caret blinking.
     *
     * @see TextField#setFocus
     */
    public void loseFocus() {
        hasFocus = false;
        caret.blinking = false;
    }

    /**
     * Sets the text in the textfield. Use this to pre-set
     * (or clear) the value displayed in the textfield.
     * 
     * Note: Does not convert the string to upper case,
     * even if the textfield has been set to upper case only.
     *
     */
    public void setText(String txt) {
        text = txt;
        int strWidth = g.getWidth(text);        
        if(strWidth > textWidth) {
            strWidth = textWidth;
        }
        caretPos = startText + strWidth;
        caret.setPosition(caretPos, yPos);
        paint();
    }

    /**
     * Gets the text entered into the textfield
     *
     * @return String containing the user's entry
     */
    public String getText() {
        return text;
    }

    /**
     * Returns whether or not the textfield has focus
     *
     * @see TextField#setFocus
     * @see TextField#loseFocus
     */
    public boolean hasFocus() {
        return hasFocus;
    }

    /**
     * Returns whether or not the x,y position is inside the
     * textfield
     *
     * @see TextField#setFocus
     * @see TextField#loseFocus
     */
    public boolean pressed(int x, int y) {
        if((x>startText) && (x<startText+textWidth) && (y>yPos) && (y<(yPos+height))) {
            return true;
        }
        return false;
    }

    /**
     * Should be called by Spotlet.keyDown(). 
     * Currently this handles backspace (0x08) and delete
     * (0x7f) as backwards delete. Does upper case conversion
     * if necessary.
     *
     */
    public void handleKeyDown(int key) {
        if((key == 0x08) || (key == 0x7f)) {
            if(text.length() > 0) {
                // delete
                char ch = text.charAt(text.length()-1);
                text = text.substring(0, text.length()-1);
                caretPos = startText + g.getWidth(text);
            }
        } else if((key >=0x20) && (key < 0x7f)) {
            // printable chars
            if(onlyUpperCase) {
                if((key >= 0x61) && (key <= 0x7a)) {
                    key -= 0x20;
                }
            }
            int textW = g.getWidth(text + ((char)key));
            if(textW < textWidth) {
                text += (char)key;            
                caretPos = startText + textW;
            } else {
                // PENDING add an error beep here!
            }
        }
        caret.setPosition(caretPos, yPos);
        paint();
    }    
}

