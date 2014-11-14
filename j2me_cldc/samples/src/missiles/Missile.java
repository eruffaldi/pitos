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

public class Missile extends GamePiece {

    public static int interval = 0;

    public static GameBitmap staticBitmap;
    public static int firstPiece;
    public static int count;
	public static int leftOffset;

    public static GameBitmap hitBitmap;
	public static int hitLeftOffset;
	public static int hitTopOffset;

	public boolean isaHit;

	public Missile() {
	    waitTicks = MAXTICKS;
	    if (staticBitmap != null) {
	        bitmap = staticBitmap;
	        return;
	    }

	    staticBitmap = bitmap =
	        new GameBitmap(7, 8, Missiles.x, Missiles.y,
                GameBitmap.VERT, 4,
                new Bitmap((short)2, new byte[] {
                // File missile.bmp;  size: 7x8
                  16,    0,   16,    0,   56,    0,   56,    0,
                 124,    0,  124,    0,   -2,    0,   -2,    0
                     } ) );
        leftOffset = bitmap.width >> 1;

	    hitBitmap =
	        new GameBitmap(14, 9, Missiles.x+10, Missiles.y,
                GameBitmap.HORZ, 1,
                new Bitmap((short)2, new byte[] {
                // File boom.bmp;  size: 14x9
                  49,   32,   72,   64,    5,    0, -123,   48,
                  50,   72,   74, -128,   10,  -92,   65,   16,
                   1,    0 } ) );
        hitLeftOffset = (bitmap.width - hitBitmap.width) >> 1;
        hitTopOffset = (bitmap.height - hitBitmap.height) >> 1;

        GamePiece[] pieces = new GamePiece[count = 4];
        pieces[0] = this;
        for (int i = 1; i < count; i++) {
            pieces[i] = new Missile();
        }
        firstPiece = Missiles.addPieces(Missiles.MISSILE, pieces);
    }

	public static void fire(int launcherCenter, int bottom) {
	    for (int i = firstPiece; i < firstPiece+count; i++) {
	        GamePiece piece = Missiles.gamePieces[i];
	        if (piece.waitTicks > 100) {
	            piece.start(launcherCenter, bottom);
	            break;
	        }
	    }
	}

	public void start(int launcherCenter, int bottom) {
	    bitmap.draw(launcherCenter - leftOffset,
	                bottom - bitmap.height, this);
	    waitTicks = interval;
    	isaHit = false;
	}

	public boolean hitShip() {
	    for (int shipIndex = Missiles.SHIPSM; shipIndex <= Missiles.SHIPXL; shipIndex++) {
	        int count = Missiles.classNumPieces[shipIndex];
	        int firstPiece = Missiles.classFirstPiece[shipIndex];
	        GamePiece ship = Missiles.gamePieces[firstPiece];
	        if (count != 0 && this.overlaps(ship)) {
                ship.erase();
                Missiles.incrementScore(ship.points);
                return true;
	        }
	    }
        return false;
    }

	public void tick() {
	    if (isaHit) {
	        hitBitmap.draw(left, top, this);
	        recycle();
	    } else {
    	    bitmap.move(-1, this);
	        waitTicks = interval;
    	    if (hitShip()) {
                if (Missiles.doSound) {
                    Graphics.playSound(Graphics.SOUND_STARTUP);
                }
    	        erase();
    	        hitBitmap.draw(left + hitLeftOffset,
    	                top + hitTopOffset, this);
    	        isaHit = true;
	            waitTicks = 5;
    	    } else {
                checkOffScreen();
            }
	    }
    }
}
