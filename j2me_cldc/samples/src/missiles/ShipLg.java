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

public class ShipLg extends GamePiece {

    public static GameBitmap staticBitmap;
    public static int firstPiece;
    public static int count;
	
	public ShipLg() {
	    points = 2;
	    if (staticBitmap != null) {
	        bitmap = staticBitmap;
	        return;
	    }
	    
	    staticBitmap = bitmap = 
	        new GameBitmap(13, 4, Missiles.x, Missiles.y,
                GameBitmap.HORZ, 4, 	
                new Bitmap((short)2, new byte[] {
                    // File shiplg.bmp;  size: 13x4
                     7,    0,    63,  -32,  -1,   -8,    15,  -128 }));

        GamePiece[] pieces = new GamePiece[count = NUMSHIPS];
        pieces[0] = this;
        recycleShip();
        for (int i = 1; i < count; i++) {
            pieces[i] = new ShipLg();
	        pieces[i].recycleShip();            
        }
        firstPiece = Missiles.addPieces(Missiles.SHIPLG, pieces);
    }
    
	public void recycle() {
	    recycleShip();
	}

	public void tick() {
	    bitmap.move(direction, this);
	    dropBomb();
        waitTicks = speed;
        checkOffScreen();
    }
}
