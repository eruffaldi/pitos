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

public abstract class GamePiece {
    
    public static final int MAXTICKS = -1 >>> 1;
    public static final int NUMSHIPS = 1;

    public GameBitmap bitmap;
	
	public int left;
	public int top;
	public int waitTicks;
	public int direction;
	public int bombType;
	public int speed;
	public int points;
	
	
	public void display(int x, int y) {
	    bitmap.display(x, y, this);
	}

	public abstract void tick();
	
	public void start(int x, int y) {
	}
	
	public void erase() {
	    bitmap.draw(left, top, this);
	    recycle();
	}
	
	public void recycle() {
	    waitTicks = MAXTICKS;
	}
	
	public void recycleShip() {
	    int random = Missiles.random.nextInt() >>> 1;
	    top = (random >> 8) & 0x7F;
	    speed = 0;//(random >> 1) & 0x3;
	    waitTicks = 0;
	    bombType = (random >> 4) & 0xF;
	    if ((random & 1) == 0) {
	        direction = +1;
	        left = 0 - bitmap.width;
	    } else {
	        direction = -1;
	        left = 160;
	    }
    }
    
	public void dropBomb() {
	    int center;
	    if (bombType == -1) return;
        if (direction > 0) {
            center = left;
            if (center < Missiles.launcherCenter) return;
        } else {
            center = left+bitmap.width-1;
            if (center > Missiles.launcherCenter) return;
	    }
	    if (Bomb.drop(center, top)) bombType = -1;
    }
    
	public boolean overlaps(GamePiece other) {
	    int right = left + bitmap.width - 1;   // right
	    int bottom = top + bitmap.height - 1;  // bottom
	    
	    int otherRight = other.left + other.bitmap.width - 1;   // right
	    int otherBottom = other.top + other.bitmap.height - 1;  // bottom
	    
	    if (other.left <= right && otherRight >= left &&
	        other.top <= bottom && otherBottom >= top) return true;
	    return false;
	}
	
	public boolean wentOffScreen() {
	    if (bitmap.vertical) {
	        int bottom = top + bitmap.height - 1;  // bottom
	        if (bottom < 0 || top > 159/*Missiles.bottom*/) return true;
	    } else {
	        int right = left + bitmap.width - 1;  // right
	        if (right < 0 || left > 159) return true;
	    }
	    return false;
	}	
	
	public void checkOffScreen() {
	    if (wentOffScreen()) recycle();
	}
}
