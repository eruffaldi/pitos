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

public class Launcher extends GamePiece {
    
    public static GameBitmap staticBitmap;
    public static int firstPiece;
    public static int count;

	public Launcher() {
	    staticBitmap = bitmap = 
	        new GameBitmap(11, 6, Missiles.x, Missiles.y,
                GameBitmap.HORZ, 4, 	    
            	new Bitmap((short)2, new byte[] {
                // File launcher.bmp;  size: 11x6
                  14,    0,   14,    0,   14,    0,   63, -128,
                  -1,  -32,   -1,  -32
                        } ) );
                
        Missiles.launcher = this;
        GamePiece[] pieces = new GamePiece[count = 1];
        pieces[0] = this;
        firstPiece = Missiles.addPieces(Missiles.LAUNCHER, pieces);
    }
	
	public void draw(int x, int y) {
	    bitmap.draw(x - (bitmap.width >> 1),
	                Missiles.bottom - bitmap.height, this);
	}
	
	public void move(int direction) {
	    int launcherCenter = Missiles.launcherCenter;
	    if (direction < 0) {
	        // move left
	        if ((launcherCenter -= 4) < 0) return;
	    } else {
	        // move right
	        if ((launcherCenter += 4) > 159) return;
	    }
        if (Missiles.doSound) Graphics.playSound(Graphics.SOUND_CLICK);
        bitmap.move(direction, this);
	    Missiles.launcherCenter = launcherCenter;
	}
	
	public void tick() {
    }
    
	public void recycle() {
    }
    
	public void fire(int launcherCenter, int launcherTop) {
	    if (waitTicks <= 0) {
            Missile.fire(launcherCenter, launcherTop);
            waitTicks = 5;
        }
    }
}
