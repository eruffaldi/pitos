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

public class GameBitmap {
    
    public static final boolean VERT = true;
    public static final boolean HORZ = false;
    
    public static int nextLeft;
    public static int nextTop = 10;
    

	Graphics ui = Graphics.getGraphics();
	
	public int width;    // width
	public int height;   // height
	
	public int leftSave;   // offscreen draw left
	public int topSave;   // offscreen draw top
	
	public int leftMove;   // offscreen move left
	public int topMove;   // offscreen move top
	public int widthMove;   // offscreen move width
	public int heightMove;   // offscreen move height
	
	public boolean vertical;
	public int offset;
	
    public GameBitmap(int width, int height, int x, int y,
                boolean vertical, int offset, Bitmap bitmap) {
                    
        leftSave = nextLeft;
        topSave = nextTop;
        this.width = width;
        this.height = height;
        this.offset = offset;
        this.vertical = vertical;
        
        nextLeft += 80;
        if (nextLeft >= 160) {
            nextLeft = 0;
            nextTop += 20;
        }
                    
        x += 5;
        y += Missiles.charH - height - 2;
        
	    ui.drawBitmap(x, y, bitmap);
	    ui.copyOffScreenRegion(x, y, width, height, leftSave, topSave,
	            ui.OVERWRITE, ui.ONSCREEN_WINDOW, ui.OFFSCREEN_WINDOW);
	            
        leftMove = leftSave + width + 5;
        topMove = topSave;
        widthMove = width;
        heightMove = height;
	    ui.copyOffScreenRegion(x, y, width, height, leftMove, topMove, 
	            ui.OVERWRITE, ui.ONSCREEN_WINDOW, ui.OFFSCREEN_WINDOW);
	            
        if (this.vertical = vertical) {
            heightMove += offset;
        } else {
            widthMove += offset;
        }
	    ui.copyOffScreenRegion(x, y, width, height,
	            leftMove + widthMove - width, topMove + heightMove - height,
	            ui.XOR, ui.ONSCREEN_WINDOW, ui.OFFSCREEN_WINDOW);
    }
	
	public void display(int x, int y, GamePiece piece) {
	    draw(x + 5, y + Missiles.charH - height - 2, piece);
	}
	
	public void draw(int left, int top, GamePiece piece) {
	    piece.left = left;
	    piece.top = top;
	    ui.copyOffScreenRegion(leftSave, topSave, width, height, left, top,
	            ui.XOR, ui.OFFSCREEN_WINDOW, ui.ONSCREEN_WINDOW);
	}
	
	public void move(int direction, GamePiece piece) {
	    int left = piece.left;
	    int top = piece.top;
	    if (direction < 0) {
	        if (vertical) top -= offset; else left -= offset;
	    }
	    ui.copyOffScreenRegion(leftMove, topMove, widthMove, heightMove,
	            left, top, 
	            ui.XOR, ui.OFFSCREEN_WINDOW, ui.ONSCREEN_WINDOW);
	    if (direction > 0) {
	        if (vertical) top += offset; else left += offset;
	    }
	    piece.left = left;
	    piece.top = top;
	}
}