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
 * A simple, prepackaged "help" text user interface object.
 */

public class HelpDisplay extends Spotlet {

  ScrollTextBox stb;
  Button doneButton;
  Spotlet application;
  int appEventOptions;
  String appName;

    /**
     * Create a new HelpDisplay.
     *
     * @param hText the text that's going to help the user
     * @param className the exact name of the class to create and run
     * @param eventOptions the event options we're interested in
     */
  public HelpDisplay(String hText, String className, int eventOptions) {
    appName = className;
    appEventOptions = eventOptions;
    stb = new ScrollTextBox(hText, 5, 5, 150, 130);
    doneButton = new Button("Done",60,145);
    stb.paint();
    doneButton.paint();

  }

    /**
     * The pen has gone down.  If the user pressed the "done" button,
     * create and register the application named by <code>className</code>.
     */
  public void penDown(int x, int y) {
	  if (doneButton.pressed(x,y)) {
		  try { 
			  Class appClass = Class.forName(appName);
			  Spotlet application = (Spotlet)appClass.newInstance();
			  application.register(appEventOptions);
		  } catch (IllegalAccessException e) { 
		  } catch (ClassNotFoundException e) { 
		  } catch (InstantiationException e) {
		  }
		  if(stb.contains(x,y)) {
			  stb.handlePenDown(x, y);
		  }	
	  }
  }

    /**
     * The pen moved.
     */
public void penMove(int x, int y) {
    if(stb.contains(x,y)) {
      stb.handlePenMove(x, y);
    }
  }

    /**
     * The user has pressed a key.
     */
    public void keyDown(int keyCode) {
        stb.handleKeyDown(keyCode);
    }
}

