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

package missiles;

import com.sun.kjava.*;

public class Missiles extends Spotlet {

    public final static byte WAITTIME = 100;

    public final static byte INITSTATE = 0;
    public final static byte HELPSTATE = 1;
    public final static byte PENSTATE  = 2;
    public final static byte PLAYSTATE  = 3;
    public final static byte BEAMSTATE = 4;
    public final static byte PAUSESTATE = 5;
    public final static byte BUSYSTATE = 6;
    public final static byte DONESTATE = 7;
    
    // game static fields
	static byte state;
	
	static boolean isSecret;

    static final java.util.Random random = new java.util.Random();
	
	static Graphics ui = Graphics.getGraphics();

	public static Launcher launcher;
	public static Missile missile;
	
    public static int launcherCenter = 80;
    public static int bottom = 145;
    public static int launcherTop = bottom;

	public static int charH;
	public static int y;
	public static int x;
	public static int offT;
	
	static Button helpButton;
	static Button exitButton;
	static Button backButton;
	static ScrollTextBox helpBox;
	
	static boolean classesAreLoaded;
	
	static int lives;
	static int livesX;
	static int score;
	static int scoreX;
	public static boolean doSound = false;

	// game instance fields
	int numSpaceships;
	public GamePiece next;


	/**
	 * The main method simply creates a MissileGame spotlet and registers
	 * its event handlers.
	 */
	public static void main(String[] args) {
		new Missiles(null);
	}
	
	public Missiles(Object obj) {
	    introScreen();
		register(WANT_SYSTEM_KEYS);
	    loop();
	}
	    
	public static void introScreen() {
        state = BUSYSTATE;
        ui.clearScreen();
        charH = ui.getHeight("A");
        y = 0;
        String message = (classesAreLoaded) ? "Classes:" :
                        "Finding classes & preparing graphics...";
        ui.drawString(message, 0, y, ui.PLAIN);
        int indexes[] = new int[numClasses];
        for (int i = numClasses-1; i >= 0; i--) indexes[i] = i;
        for (int i = numClasses-1; i >= 1; i--) {
            int j = (random.nextInt() >>> 1) % i;
            int temp = indexes[i];
            indexes[i] = indexes[j];
            indexes[j] = temp;
        }
        for (int i = numClasses-1; i >= 0; i--) {
            load(indexes[i]);
        }
        classesAreLoaded = true;
        
		helpButton = new Button("Help", 110, 146);
		helpButton.paint();
		exitButton = new Button("Exit", 139, 146);
		exitButton.paint();
		backButton = new Button("Back", 136, 146);
		ui.drawString("Press key to start game", 0, 148);
		state = INITSTATE;
	}

	/**
	 * load a class and initialize it
	 */
	static void load(int i) {
	    String className = classNames[i << 1];
	    String hint = classNames[(i << 1) + 1];
	    int numPieces = classNumPieces[i];
	    
	    if (numPieces != 0 || hint != null) {
            y += charH + 1;
            x = ui.drawString(className, 5, y, ui.PLAIN);
        }
        
        if (numPieces != 0) {
            gamePieces[classFirstPiece[i]].display(x, y);
            return;
        }
        
        String fullname = "missiles." + className;
	        try {
            (Class.forName(fullname)).newInstance();
            } catch (Exception ex) {
		if (hint == null) return;
                ui.drawString(" ?? -> " + hint, x, y, ui.PLAIN);
                return;
            }
	}

	/**
	 * Handle a pen down event.
	 */
	public void penDown(int x, int y) {
	    if (state == INITSTATE) {
	        if (exitButton.pressed(x, y)) exit();
	        if (helpButton.pressed(x, y)) {
	            state = BUSYSTATE;
	            ui.clearScreen();
	            if (helpBox == null) {
    	            ui.drawString("Reading help text...", 10, 20, ui.PLAIN);
                    helpBox = new ScrollTextBox(helpText(), 0, 0, 160, 145);
                }
                helpBox.paint();
                backButton.paint();
	            state = HELPSTATE;
	            return;
	        }
	    }
	    if (state == HELPSTATE) {
	        if (backButton.pressed(x, y)) {
	            state = BUSYSTATE;
	            introScreen();
	            state = INITSTATE;
	        }
            if (helpBox.contains(x,y)) {
        	    helpBox.handlePenDown(x, y);
        	    return;
            }	
	    }
	    if (state == PAUSESTATE || state == PLAYSTATE || state == DONESTATE) {
	        if (backButton.pressed(x, y)) {
	            state = BUSYSTATE;
	            setClip(false);
	            introScreen();
	            state = INITSTATE;
	        }
	    }
	}

	public static void setClip(boolean doClip) {
	    if (doClip) {
	        ui.setDrawRegion(0, 0, 160, bottom);
	    } else {
	        ui.resetDrawRegion();
	    }
	}
	
	public static void incrementScore(int value) {
	    setClip(false);
	    score += value;
	    ui.drawString(score+" ", scoreX, 148, ui.PLAIN);
	    setClip(true);
	}
	
	public static void decrementLives() {
	    setClip(false);
	    lives--;
	    if (lives == 0) state = DONESTATE;
	    ui.drawString(lives+" ", livesX, 148, ui.PLAIN);
	    setClip(true);
	}
	
	public void setSound(boolean on) {
	    setClip(false);
	    String msg;
	    if (doSound = on) {
	        msg = "sound  ";
	    } else {
	        msg = "quiet  ";
	    }
	    ui.drawString(msg, 100, 148, ui.PLAIN);
	    setClip(true);
	}
	
	public void setPause() {
	    setClip(false);
	    ui.drawString("pause  ", 100, 148, ui.PLAIN);
	    setClip(true);
	    state = PAUSESTATE;
	}
	
	/**
	 *
	 */
	public void startGame() {
	    ui.clearScreen();
	    backButton.paint();
	    scoreX = ui.drawString("Score: ", 0, 148, ui.PLAIN);
	    score = 0;
	    livesX = ui.drawString("Lives: ", 53, 148, ui.PLAIN);
	    lives = 6;
	    
	    incrementScore(0);
	    decrementLives();
	    
	    setSound(doSound);
	    setClip(true);
	    
	    if (launcher != null) {
	        launcher.draw(launcherCenter, 0);
	        launcherTop = launcher.top;
	    }
    	for (int i = 0; i < gameNumPieces; i++) {
    	    gamePieces[i].recycle();
    	}
	    state = PLAYSTATE;
	}
	    
	public static int waitTime = WAITTIME;
	
	public void loop() {
	    long wakeTime = 0;
	    long time;
	    for(;;) {
	        while (state != PLAYSTATE ||
	               (time = System.currentTimeMillis()) < wakeTime) {
				try {
					Thread.sleep(10);
				} catch (InterruptedException e) {}	
			}
	        while ((time = System.currentTimeMillis()) < wakeTime);
	        wakeTime = time + waitTime;
	        for (int i = 0; i < gameNumPieces; i++) {
	            GamePiece piece = gamePieces[i];
	            if ((--piece.waitTicks) <= 0) piece.tick();
	        }
	    }
	}

	public static void score(int points) {
	}
	
	/**
	 * Handle a pen up event.
	 */
	public static void exit() {
	    System.exit(0);
	}

	/**
	 * Handle a key down event.
	 */
	public void keyDown(int keyCode) {
	    if (keyCode == KEY_POWER || keyCode == 276) {
            exit();
		}

        if (state == PLAYSTATE) {
    		switch (keyCode) {
                case Spotlet.KEY_HARD2:
                    if (launcher != null) launcher.move(-1);
                    return;
                case Spotlet.KEY_HARD3:
                    if (launcher != null) launcher.move(+1);
                    return;
                case Spotlet.KEY_HARD1:
                    setPause();
                    return;
                case Spotlet.KEY_HARD4:
                    setSound(!doSound);
                    return;
                case Spotlet.PAGEUP:
                    if (classNumPieces[MISSILE] != 0) {
                        if (launcher != null) {
                            launcher.fire(launcherCenter, launcherTop);
                        } else {
                            Missile.fire(launcherCenter, launcherTop);
                        }
                    }
                    return;
            }
        }
        
	    if (state == INITSTATE) {
            state = BUSYSTATE;
            startGame();
            return;
		}

	    if (state == PAUSESTATE & keyCode == Spotlet.KEY_HARD1) {
            state = PLAYSTATE;
            setSound(doSound);
            return;
		}
		
	    if (state == HELPSTATE) {
	        helpBox.handleKeyDown(keyCode);
	        return;
	    }
	}

    public static int addPieces(int classIndex, GamePiece[] pieces) {
        int numPieces = pieces.length;
        int firstPiece = gameNumPieces;
        for (int i = 0; i < numPieces; i++) {
            gamePieces[gameNumPieces++] = pieces[i];
        }
        classNumPieces[classIndex] = numPieces;       
        return (classFirstPiece[classIndex] = firstPiece);
    }
    
    
    public static final int LAUNCHER = 0;
    public static final int SHIPSM = 1;
    public static final int SHIPMD = 2;
    public static final int SHIPLG = 3;
    public static final int SHIPXL = 4;
    public static final int MISSILE = 5;
    public static final int BOMB = 6;
    public static final int MINE = 7;
    public static final int CHAFF = 8;
    
    static String[] INIT_CLASSNAMES = {
        "Launcher", "Look around",
        "ShipSm",   "Look around",
        "ShipMd",   "Look around",
        "ShipLg",   "Look around",
        "ShipXL",   "Look around",
        "Missile",  "Look around",
        "Bomb",     "Look around",
    };
    
    public static final int MAXCLASSES = 20;
    public static String[] classNames = new String[MAXCLASSES];
    public static int[] classFirstPiece = new int[MAXCLASSES];
    public static int[] classNumPieces = new int[MAXCLASSES];
    public static int numClasses;
    
    public static final int MAXPIECES = 50;
    public static GamePiece[] gamePieces = new GamePiece[MAXPIECES];
    public static int gameNumPieces;
    
    static {
        int len = INIT_CLASSNAMES.length;
        numClasses = len >> 1;
        for (int i = 0; i < len; i++) {
            classNames[i] = INIT_CLASSNAMES[i];
        }
        len = GameBitmap.nextLeft;    // force class loading of GameBitmap??
    }
    

    static String helpText() {
        return
"The object of the Missile game is to shoot down spaceships before they blow you up with bombs. " +
"But first you have to gather all the classes to run the game. The intro screen gives you a clue of where to find them. "+
"Good Luck!\n\n"+
"Buttons:\n"+
"  Scroll up = shoot a missile\n"+
"  Phone = move launcher left\n"+
"  Todo = move launcher right\n"+
"  Calendar = pause/resume\n"+
"  Memo = sound on/off\n";


    }
}
