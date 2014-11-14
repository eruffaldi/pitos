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
 
package dots;

import com.sun.kjava.*;

/**
 * Create a Dots game-playing class by subclassing this class.
 * Be sure to put your class in this same package (dots). Otherwise,
 * the Dots main application on the Palmtop will not be able to find it.
 * Name your class something that is unlikely to collide with other people's classes.
 * Otherwise, you can't run both classes against each other on the same device. 
 *
 * <p>The constants and fields inherited from DotGame (and listed below) provide information on the
 * game state. You are on your honor to NOT write into these fields! Implement your
 * algorithm by overriding the <code>myTurn</code> method. You may add additional 
 * fields and methods.
 * For beaming ease, try to avoid adding additional classes.
 *
 * <p>Each game starts with a new instance of your class. No state is saved
 * between games.
 *
 * <p>For example code, see dots.TopDown.java
 */
public class DotGame extends Spotlet {
    
    // protected stuff 
    
    /**
     * The Host player (= 1)
     */
    protected final static byte HOST  = 0x1;
    
    /**
     * The Guest player (= 2)
     */
    protected final static byte GUEST = 0x2;
    
    /**
     * The Top side of a box (= 0)
     */
    protected final static byte TOP    = 0;
    
    /**
     * The Left side of a box (= 1)
     */
    protected final static byte LEFT   = 1;
    
    /**
     * The Bottom side of a box (= 2)
     */
    protected final static byte BOTTOM = 2;
    
    /**
     * The Right side of a box (= 3)
     */
    protected final static byte RIGHT  = 3;
    
    /**
     * The number of rows of boxes in this game
     */
    protected static int rows;
    
    /**
     * The number of columns of boxes in this game
     */
    protected static int cols;
    
    /**
     * The current number of empty boxes in this game
     */
    protected static int emptyBoxes;
    
    /**
     * The current state of each box. 
     *
     * <p>Although the game is called Dots, the
     * data structures represent the boxes formed by drawing lines
     * between dots. The box array is the primary data structure and is indexed as
     * <code>box[row][col]</code>.
     *
     * <p>The lower 4 bits <3:0> of a boxCode represent each side of the box and
     * indicate which sides
     * are open (0) or have been drawn (1). The bit positions
     * are <code>(1 << side)</code>, where <code>side</code> is one of TOP, LEFT, etc.
     *
     * <p>The uppermost bit <7> indicates if the box is empty (0) or has all 4
     * sides drawn (1) and thus contains a player's icon.
     *
     * <p>If the box is empty, the middle bits <6:4> indicate how many sides of the
     * box are open: 1-4. Otherwise, these bits indicate
     * the "owner" of the box: HOST or GUEST.
     */
    protected static byte[][] box;       // filled:1, who/num:3, sideCode:4
    
    /**
     * The list of plays made in the game.
     *
     * <p>The first entry [0] is special and contains the number of rows (upper byte) and
     * columns (lower byte) in the game.
     *
     * <p>Subsequent entries are filled in as the game progresses. Each entry indicates
     * a play made (that is, a box side drawn) in the format:
     * <code>(row << 12) | (col << 8) | (who << 4) | (side)</code>. <code>who</code> indicates which player made the
     * move: (HOST or GUEST).  <code>side</code> is one of TOP, LEFT, etc. 
     * Note that only a single entry is made for each line segment, although most
     * line segments are the sides of two adjacent boxes.
     * 
     * <p><code>playNum</code> contains the index of the current play; that is,
     * the play that is about to be made by you.
     *
     * @see dots.DotGame#playNum
     */
	protected static short[] playList;   // row:4, col:4, who:4, side:4 (starts with row/col)

    /**
     * The number of the current "play" in the game. This value is 1 when the 
     * starting player's <code>myTurn</code> method is first called. It is incremented after
     * each "play" (line drawn). This is the number of the play that you will be
     * making on your turn. It is one greater than the number of plays that
     * have already been made (which are listed in in the playList).
     */
	protected static int playNum;
	
    /**
     * Your role in the current game: HOST or GUEST.
     */
	protected byte me = HOST;
    
    // from here on is private  
    
    private final static byte SQUARE = 12;
    private final static byte OFFSET = 2;
    
    private final static byte INITSTATE = 0;
    private final static byte HELPSTATE = 1;
    private final static byte PENSTATE  = 2;
    private final static byte RUNSTATE  = 3;
    private final static byte BEAMSTATE = 4;
    private final static byte PAUSESTATE = 5;
    private final static byte BUSYSTATE = 6;
    private final static byte DONESTATE = 7;
    private final static byte RETRYSTATE = 8;
	private static byte state;
	
	private static Graphics ui = Graphics.getGraphics();
	private static Button hostFirst;
	private static Button guestFirst;
	private static Button exitButton;
	private static Button backButton;
	private static Button hostStartButton;
	private static Button guestStartButton;
	private static Button helpButton;
	private static ValueSelector rowBox;
	private static ValueSelector colBox;
    private static com.sun.kjava.SelectScrollTextBox classListBox;
    private static ScrollTextBox helpBox; 
	
	private static DotGame host;
	private static DotGame guest;
	private static DotGame player;
	private static boolean isBeaming;
	
	private static int hiliteX1;
	private static int hiliteY1;
	private static int hiliteX2;
	private static int hiliteY2;
    
	private int score = -1;
	private int scoreCol;
	private DotGame opponent;
	
	private static int x;
	private static int y;
	
	private static int lastSentPlayNum;
	private static byte[] data;
	private static int dataIndex;
	private static int playerIndex;
	
	private static String classNames;
    private static String prefix = "dots.";
    private static int prefixLen = prefix.length();
    private static String PEN = "PenTaps";
    private static String DOTGAME = "DotGame";
    private static String AVERAGE = "Average";
    private static String RANDOM  = "Random";
    private static String TOPDOWN = "TopDown";
    
    private static String gameNames = 
        "PenTaps\nAverage\nRandom\nTopDown";
    
	private static String[] playerClass = { PEN, PEN };
    
    private static int charHeight = Graphics.getHeight("E");
    private static int beamSeq;
	
    static final java.util.Random random = new java.util.Random();
	
	/**
	 * Create a DotGame spotlet.
	 */
	public static void main(String[] args) {
		DotGame game = new DotGame();
	    exitButton = new Button("Exit", 139, 146);
	    backButton = new Button("Back", 136, 146);
	    hostStartButton = new Button("Host moves first", 1, 113);
	    guestStartButton = new Button("Guest moves first", 82, 113);
	    helpButton = new Button("Help", 110, 146);
	    rowBox = new ValueSelector("       Rows:", 1, 11, 9, 0, 130);
	    colBox = new ValueSelector("Columns:", 1, 13, 9, 0, 146);;
        classListBox = new SelectScrollTextBox("", 3, 40, 151, 66);
		initScreen();
		game.register(NO_EVENT_OPTIONS);
        for (;;) {
            if (state == RUNSTATE){
	            takeTurns();
	        }
	    }
	}

	/**
	 * Perform your initializations (if any) by providing a constructor for your class.
	 */
	public DotGame() {
    }
	
	/**
	 * Display help
	 */
	private static void helpScreen() {
	    ui.clearScreen();
	    ui.drawString("Loading help text...", 10,10,ui.PLAIN);
	    if (helpBox == null) {
	        helpBox = new ScrollTextBox(helpText, 5, 5, 150, 130);
	    }
        helpBox.paint();
        backButton.paint();
        state = HELPSTATE;
	}
	
	/**
	 * Initializer
	 */
	private static void initScreen() {
	    ui.clearScreen();
	    ui.drawString("Searching class database...", 10, 50, ui.PLAIN);
	    if (classNames == null) classNames = gameNames;
	    if (classListBox == null) {
            classListBox = new SelectScrollTextBox(
                "\n   Searching class database...", 3, 26, 155, 66);
        }
	    classListBox.setText(classNames);
	    classListBox.paint();
	    ui.drawBorder(1, 40, 155, 67, ui.PLAIN, ui.SIMPLE);
	    drawO(0, 0);
	    ui.drawString("  Host player:", SQUARE+OFFSET+4, OFFSET+1);
	    setClass(playerClass[0]);
	    drawX(1, 0);
	    ui.drawString("Guest player:", SQUARE+OFFSET+4, SQUARE+OFFSET+1);
	    setClass(playerClass[1]);
	    hostStartButton.paint();
	    guestStartButton.paint();
	    rowBox.paint();
	    colBox.paint();
	    helpButton.paint();
	    exitButton.paint();
	    beamSeq = -1;
	    state = INITSTATE;
	}

	/**
	 * Set the Host or Guest className
	 */
	private static void setClass(String name) {
	    int x = 75;
	    int y = (playerIndex * SQUARE) + OFFSET+1;
	    ui.drawRectangle(x, y, 160-x, charHeight, ui.ERASE, 0);
	    ui.drawString(name, x, y);
	    playerClass[playerIndex] = name;
	    playerIndex ^= 1;
	    String type = (playerIndex == 0) ? "HOST" : "GUEST";
	    ui.drawString("Tap below to select new " + type + " class:   ", 0, 26, ui.INVERT);
	    if (classNames != null) {
	        String names = classNames;
	        classListBox.setText(names);
	        classListBox.paint();
	    }
	}

	
	/**
	 * Handle a pen down event.
	 */
	public void penDown(int x, int y) {
		switch (state) {
		    case DONESTATE:
		    case RUNSTATE:
		    case BEAMSTATE:
		        if (backButton.pressed(x,y)) {
		            state = BUSYSTATE;
		            ui.clearScreen();
		            sleep(20);
		            initScreen();
		        }
	            return;
		    case HELPSTATE:   
		        helpStatePenDown(x, y);
		        return;
		    case INITSTATE:   
		        initStatePenDown(x, y);
		        return;
		    case PENSTATE:
		        penStatePenDown(x, y);
		        return;
		    default:
		        return;		    
		}
	}
		
	/**
	 * Handle a pen move event.
	 */
	public void penMove(int x, int y) {
	    if (state == INITSTATE && classListBox.contains(x,y)) {
	        classListBox.handlePenMove(x,y);
	        return;
	    }
	    else if (state == HELPSTATE && helpBox.contains(x,y)) {
            helpBox.handlePenMove(x, y);
        }
	}
    
	/**
	 * Handle a key down event.
	 */
	public void keyDown(int keyCode) {
	    if (state == INITSTATE) classListBox.handleKeyDown(keyCode);
	    else if (state == HELPSTATE) helpBox.handleKeyDown(keyCode);
	    else if (state == PAUSESTATE) state = RUNSTATE;
	    else if (state == BEAMSTATE) state = RETRYSTATE;
	}
    
	/**
	 * Handle a pen down event in the HELPSTATE.
	 */
	private void helpStatePenDown(int x, int y) {
        if (backButton.pressed(x,y)) {
            state = BUSYSTATE;
            initScreen();
            state = INITSTATE;
            return;
        }
        if (helpBox.contains(x,y)) {
    	    helpBox.handlePenDown(x, y);
    	    return;
        }	
	}
	
	/**
	 * Handle a pen down event in the INITSTATE.
	 */
	private void initStatePenDown(int x, int y) {
		if (exitButton.pressed(x,y)) {
	        System.exit(0);
		}
		if (helpButton.pressed(x,y)) {
            state = BUSYSTATE;
		    helpScreen();
		}
	    if (hostStartButton.pressed(x,y)) {
	        start(HOST);
	        return;
	    }
	    if (guestStartButton.pressed(x,y)) {
	        start(GUEST);
	        return;
	    }
		if (rowBox.pressed(x,y)) return;
		if (colBox.pressed(x,y)) return;
		String selection = classListBox.getSelection(x, y);
        if (selection != null) {
	        setClass(selection);
	        return;
	    }
		if (classListBox.contains(x,y)) {
		    classListBox.handlePenDown(x,y);
	        return;
	    }
	}
	
	/**
	 * Return true if class name == DotGame
	 */
	private static boolean isDotGame(byte[] rec, int index) {
	    int len = DOTGAME.length();
	    for (int i = 0; i < len; i++) {
	        if (rec[index+i] != DOTGAME.charAt(i)) return false;    
	    }
        return true;
    }
    
	/**
	 * 
	 */
	private static void start(byte startWho) {
	    ui.clearScreen();
	    backButton.paint();
	    
	    rows = rowBox.getValue();
	    cols = colBox.getValue();
        emptyBoxes = rows * cols;
        int totalPlays = (2 * emptyBoxes) + rows + cols + 1;
    	playList = new short[totalPlays];
    	playList[0] = (short)((rows << 8) | cols);
    	playNum = 1;
    	lastSentPlayNum = 0;
    	box = new byte[rows][cols];
	    for (int row = 0; row < rows; row++) {
	        for (int col = 0; col < cols; col++) {
	            box[row][col] = 4 << 4;
	        }	        
	    }	    
	    for (int col = 0; col < (cols+1); col++) {
	        for (int row = 0; row < (rows+1); row++) {
	            setXY(row, col);
	            ui.drawLine(x-1, y, x+1, y, ui.PLAIN);
	            ui.drawLine(x, y-1, x, y+1, ui.PLAIN);
	        }	        
	    }
	    
	    host = getPlayerDotGame(0);
	    host.me = HOST;
	    
	    guest = getPlayerDotGame(1);
	    guest.me = GUEST;
	    
	    host.opponent = guest;
	    guest.opponent = host;
        
        host.scoreCol = (startWho == HOST) ? 0 : 4;
        drawO(12, host.scoreCol);
        drawX(12, (guest.scoreCol = host.scoreCol ^ 4));
        
	    setXY(12, 1);
        ui.drawString("=", x, y+1, ui.PLAIN);
	    setXY(12, 5);
        ui.drawString("=", x, y+1, ui.PLAIN);
        
        host.drawScore();
        guest.drawScore();
        ui.drawString(playNum+" ", 95, 160-13);
        
        
        player = (startWho == HOST) ? host : guest;    	
	    state = RUNSTATE;
	}

	/**
	 *
	 */
	private static DotGame getPlayerDotGame(int index) {
	    String className = playerClass[index];
	    DotGame game = null;
	    try {
	        game = (DotGame)(Class.forName(prefix+className)).newInstance();
	    } catch (Exception ex) {
	        System.exit(-1);
	    }
	    return game;
	}
	
	/**
	 *
	 */
	private static void takeTurns() {
    	player.drawTurnBorder(ui.PLAIN);
    	short playCode;
    	if (emptyBoxes == 0) {
        	if (host.score >= guest.score) {
                host.drawTurnBorder(-1);
        	} 
        	if (host.score <= guest.score) {
                guest.drawTurnBorder(-1);
        	}
        	if (isBeaming && lastSentPlayNum != playNum) {
        	    switchPlayer();
        	    player.myTurn();
        	}
        	state = DONESTATE;
    	    return;
    	}
        playCode = player.myTurn();
        if (play(playCode) == 0) switchPlayer();
    }

	/**
	 * 
	 */
	private static void switchPlayer() {
	    player = player.opponent;
	}

	/**
	 * 
	 */
	private static void setXY(int row, int col) {
        x = (col * SQUARE) + OFFSET;
        y = (row * SQUARE) + OFFSET;
	}

	/**
	 * 
	 */
	private void drawScore() {
	    setXY(12, scoreCol+1);
        ui.drawString(String.valueOf(++score), x+8, y+1, ui.PLAIN);
	}

	/**
	 * 
	 */
	private void drawTurnBorder(int mode) {
	    if (mode != ui.ERASE) {
	        opponent.drawTurnBorder(ui.ERASE);
	    }
	    setXY(12, scoreCol);
        if (mode == -1) {
            ui.drawRectangle(x, y, 36, 12, ui.INVERT, 0);
	    }
        ui.drawBorder(x, y, 36, 12, mode, ui.SIMPLE);
	}
	
	/**
	 * 
	 */
	private static void drawX(int row, int col) {
        setXY(row, col);
        ui.drawLine(x+3, y+3, x+(3+SQUARE-6), y+(3+SQUARE-6), ui.PLAIN);
        ui.drawLine(x+3, y+(3+SQUARE-6), x+(3+SQUARE-6), y+3, ui.PLAIN);
	}
	
	/**
	 * 
	 */
	private static void drawO(int row, int col) {
        setXY(row, col);
        ui.drawRectangle(x+4, y+4, SQUARE-7, SQUARE-7, ui.PLAIN, 3);
	}
	
	/**
	 * 
	 */
	private static int play(short playCode) {
        int row = (playCode >> 12) & 0xF;
        int col = (playCode >> 8) & 0xF;
	    if (row < 0 || row >= rows || col < 0 || col >= cols) {
	        return 0;
	    }
        int side = playCode & 0x3;
        playList[host.playNum++] = playCode;
        drawSide(row, col, side);
        int complete = setSide(row, col, side);
        if (side == BOTTOM) {
            row++;
            side = TOP;
        } else if (side == RIGHT) {
            col++;
            side = LEFT;
        } else if (side == TOP) {
            row--;
            side = BOTTOM;
        } else if (side == LEFT) {
            col--;
            side = RIGHT;
        }
        complete += setSide(row, col, side);
        ui.drawString(playNum+" ", 95, 160-13);
        return complete;
	}

	/**
	 * 
	 */
	private static void drawSide(int row, int col, int side) {
	   ui.drawLine(hiliteX1, hiliteY1, hiliteX2, hiliteY2, ui.ERASE);
       if (side == BOTTOM) {
            row++;
            side = TOP;
        } else if (side == RIGHT) {
            col++;
            side = LEFT;
        }
	    setXY(row, col);
	    if (side == TOP) {
	        ui.drawLine(x, y, x + (SQUARE-1), y, ui.PLAIN);
	        hiliteX1 = x+1;
	        hiliteY1 = y-1;
	        hiliteX2 = x + (SQUARE-1);
	        hiliteY2 = y-1;
	    } else {
	        ui.drawLine(x, y, x, y + (SQUARE-1), ui.PLAIN);
	        hiliteX1 = x-1;
	        hiliteY1 = y+1;
	        hiliteX2 = x-1;
	        hiliteY2 = y + (SQUARE-1);
	    }
	    ui.drawLine(hiliteX1, hiliteY1, hiliteX2, hiliteY2, ui.PLAIN);
	}

	/**
	 * 
	 */
	private static int setSide(int row, int col, int side) {
	    if (row < 0 || row >= rows || col < 0 || col >= cols) {
	        return 0;
	    }
	    int returnVal = 0;
        int boxCode = box[row][col];
        int sideCode = 1 << side;
        if ((boxCode & sideCode) != 0) {
            ui.drawString("Bad Side", 80, 140, ui.PLAIN);
            return 0;
        }
        boxCode |= sideCode;
        boxCode -= (1 << 4);
        if ((boxCode >> 4) == 0) {
            boxCode |= (0x8 | player.me) << 4;
            setXY(row, col);
            returnVal = 1;
            if (player == host) {
                drawO(row, col);
            } else {
                drawX(row, col);
            }
            player.drawScore();
            emptyBoxes--;
        }
        box[row][col] = (byte)boxCode;
        return returnVal;
    }

	/**
	 * Call this method (multiple times if you wish) to get the playCode for a line segment that you
	 * are interested in. The method returns -1 if the play is illegal (not a valid
	 * box/side, or the side is already drawn). Otherwise, the method returns the
	 * playCode that should be returned from your <code>myTurn</code> method if you wish to make that
	 * play.
	 *
	 * @param row the row number of the box: 0 to (rows-1)
	 * @param col the column number of the box: 0 to (cols-1)
	 * @param side the side of the box: TOP, LEFT, etc.
	 * @return -1 if the play is illegal, or the play encoded in the format
	 * used in the playList (with who=0).
	 * @see dots.DotGame#playList
	 */
	protected static short getPlayCode(int row, int col, int side) {
        if ( row < 0 || row >= rows ||
             col < 0 || col >= cols ||
             side < 0 || side > 3) {
            return -1;
        }
        byte boxCode = box[row][col];
        byte sideCode = (byte)(1 << side);
        if ((boxCode & sideCode) != 0) return -1;
	    return (short)((row << 12) | (col << 8) | (player.me << 4) | side);
	}

	/**
	 * Handle a pen down event in the HUMANSTATE.
	 */
	private static void penStatePenDown(int x, int y) {
		if (backButton.pressed(x,y)) {
		    state = BUSYSTATE;
		    initScreen();
		    return;
		}
	    DotGame.x = x;
	    DotGame.y = y;
	    state = RUNSTATE;
	}
	
	protected short penTurn() {
	    for (;;) {
            state = PENSTATE;
            do sleep(2);
            while (state == PENSTATE);
    	    if (state != RUNSTATE) return -1;
    	    
            boolean firstX = (x % SQUARE) < 6;
            boolean firstY = (y % SQUARE) < 6;
            int col = x / SQUARE;
            int row = y / SQUARE;
            int side = 0;
            
            if (firstX) {
                if (!firstY) {
                    side = LEFT;
                    if (col == cols) {
                        side = RIGHT;
                        col--;
                    }
                } else {
                    col = -1;
                }
            } else {
                if (firstY) {
                    side = TOP;
                    if (row == rows) {
                        side = BOTTOM;
                        row--;
                    }
                } else {
                    row = -1;
                }
            }
            short playCode = getPlayCode(row, col, side);
            if (playCode != -1) {
    	        ui.playSound(ui.SOUND_CLICK);
    	        return playCode;
            }
        }
    }
	
	/**
	 * Override this method with the code for your Dots game-playing algorithm.
	 * This method is called each time it is your turn to draw a line segment. If
	 * <code>playNum == 1</code> then you are the starting player.
	 * <p>Return the playCode of the segment you wish to draw. If you return an
	 * illegal playCode, you lose this turn!
	 * @return the playCode of the line segment you wish to draw
	 * @see dots.DotGame#getPlayCode
	 */
	protected short myTurn() {
	    System.exit(-2);
	    return 0;
	}
    
	/**
	 * Go to sleep.
	 */
	private void sleep(int delay) {
		try {
			Thread.sleep(delay);
		} catch (InterruptedException e) {}	
	}
	
    
	static String helpText =
"In the Dots game, 2 players take turns drawing lines (horizontal or vertical) between 2 "+
"adjacent dots of a grid. When a player completes a box, the player's icon is placed inside the box "+
"AND the player gets another turn. When all boxes are filled, the player with the most icons "+
"wins. The bottom of the screen shows the starting player on left, the score, "+
"the player whose turn it is (outlined), and the number of plays.\n"+
"In this game, two Java(tm) classes battle each other. One class is "+
"the HOST (icon = circle). The other is the GUEST (icon = X). You select the "+
"algorithms for both players from a list of classes on your device. You may also "+
"select the grid size and starting player.\nThe pre-loaded classes are:\n"+
"> PenTaps - This is you. Tap the screen on your turn.\n"+
"> TopDown - Draws lines left-to-right, top-to-bottom.\n"+
"> Random - Draws lines in random places.\n"+
"> Average - This algorithm tries to be smart. Try to beat it!\n"+
"> Beam - Choose this to have your GUEST be someone else on another device. Both of you must be running the "+
"Dots game. Each of you choose your HOST algorithm, and then choose Beam for the GUEST. One of you chooses the grid "+
"size and starts the game.\n\n"+
"If you are up to the challenge, come to the KJava Playground (Hackers Lounge in the Pavillion) "+
"and write your own algorithm! Then play your code against "+
"others. Because beaming is slow, the best way to play another person's algorithm is to beam one "+
"class over and play them against each other on the same device.";

}
