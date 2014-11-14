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

import com.sun.kjava.*;


public class UITest extends Spotlet implements DialogOwner {

    public static Button exitButton;
    public static Button clearButton;
    public static Graphics g = Graphics.getGraphics();
    public TextField tf;
    public TextField tf2;
    Slider slider;
    int currDialog;
    protected static String DIALOG1_TITLE = "OPENING_DIALOG";
    protected static String DIALOG2_TITLE = "SECOND_DIALOG";
    String dialog1Text = "This is a demonstration program for the bug fixes in this release of the KVM. In particular this page shows the changes to the Dialog class, which now uses a ScrollTextBox if necessary to display the contents of the dialog. The next screen shows the Dialog class without the extra long text to force a scroll bar.\n\n When you press the OK button, this screen will be replaced with the next dialog. Pressing OK on that screen will bring up a test page for the TextField class fixes. \n\nPress OK to continue...";
    String dialog2Text = "A simple dialog. \n\n It uses embedded line breaks to show extra whitespace";

    public static void main(String args[]) {
        new UITest();
    }

    public UITest() {
        currDialog = 1;
        Dialog d = new Dialog(this, DIALOG1_TITLE, dialog1Text, "OK");
        unregister();
        d.showDialog();
    }


    public void dialogDismissed(String title) {
        if(title.equals(DIALOG1_TITLE)) {            
            register(NO_EVENT_OPTIONS);

            currDialog = 2;
            Dialog d = new Dialog(this, DIALOG2_TITLE, dialog2Text, "OK");
            unregister();
            d.showDialog();
        } else {

            // we need to re-register ourselves for events
            register(NO_EVENT_OPTIONS);
         
            currDialog = 3;
   
            g.clearScreen();
            tf = new TextField("Solution", 5, 10, 100, 20);            
            tf.setUpperCase(true);
            
            tf2 = new TextField("Solution2", 5, 50, 100, 20);
            tf2.setUpperCase(true);

            slider = new Slider(5, 100, 100, 0, 1000, 0);
                    
            tf.setFocus();
            exitButton = new Button("Exit",115,145);        
            clearButton = new Button("Clear Text Fields",5,145);        

            paint();

            tf.setText("Answer");
            tf2.setText("Answer2");
        }
    }

    public void paint() {
        if(currDialog == 3) {
            exitButton.paint();
            clearButton.paint();
            g.drawRectangle(0, 0, 160, 80, 0, g.PLAIN);
            tf.paint();
            tf2.paint();
            slider.paint();
        }
    }

    public void keyDown(int key) {
        if(currDialog == 3) {
            if(tf.hasFocus()) {
                tf.handleKeyDown(key);
            }
            
            if(tf2.hasFocus()) {
                tf2.handleKeyDown(key);
            }
        }
    }

    public void penMove(int x, int y) {
        if(currDialog == 3) {
            if(slider.contains(x, y)) {
                slider.handlePenMove(x, y);
            }
        }
    }

    public void penDown(int x, int y) {
        if(currDialog == 3) {
            if(exitButton.pressed(x, y)) {
                System.exit(0);
            }
            
            if(clearButton.pressed(x, y)) {
                tf.setText("");
                tf.paint();
                tf2.setText("");
                tf2.paint();
            }
            
            if(slider.contains(x,y)) {
                slider.handlePenDown(x, y);
            }
            
            if(tf.pressed(x,y) && (!tf.hasFocus())) {
                tf2.loseFocus();
                tf.setFocus();
            }
            
            if(tf2.pressed(x,y) && (!tf2.hasFocus())) {
                tf.loseFocus();
                tf2.setFocus();
            }
        }
    }



}
