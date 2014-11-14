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
 
import com.sun.kjava.*;

public class ThreeDeeObject {

/*==============================================================================
 * Static variables
 *============================================================================*/

    /**
     * centerX, centerY: used for storing the center coordinates 
     * of our physical drawing plane. Must be 80 on the PalmPilot
     * (physical screen size is 160x160).
     */
    static final int centerX = 80;
    static final int centerY = 80;

    /**
     * planeDist: the 2D projection plane distance from origo.
     */
    static final int planeDist = -100;

    /**
     * clipPlane: object move limit (to avoid clipping problems)
     */
	static final int clipPlane = -5750;

    /**
     * length, p: define the object to show
     */
    static final int length = 130;

    static int p[][] = {
  { 750, 200, 0 },
  { 553, 234, 0 },
  { 380, 334, 0 },
  { 252, 487, 0 },
  { 183, 675, 0 },
  { 175, 2700, 0 },
  { 650, 2700, 0 },
  { 650, 775, 0 },
  { 850, 775, 0 },
  { 850, 2700, 0 },
  { 1325, 2700, 0 },
  { 1316, 675, 0 },
  { 1248, 487, 0 },
  { 1119, 334, 0 },
  { 946, 234, 0 },
  { 750, 200, 0 },

  { 2150, 2700, 0 },
  { 1953, 2665, 0 },
  { 1780, 2565, 0 },
  { 1652, 2412, 0 },
  { 1583, 2225, 0 },
  { 1575, 200, 0 },
  { 2050, 200, 0 },
  { 2050, 2125, 0 },
  { 2250, 2125, 0 },
  { 2250, 200, 0 },
  { 2725, 200, 0 },
  { 2716, 2225, 0 },
  { 2648, 2412, 0 },
  { 2519, 2565, 0 },
  { 2346, 2665, 0 },
  { 2150, 2700, 0 },

  { -200, 750, 0 },
  { -234, 553, 0 },
  { -334, 380, 0 },
  { -487, 252, 0 },
  { -675, 184, 0 },
  { -2700, 175, 0 },
  { -2700, 650, 0 },
  { -775, 650, 0 },
  { -775, 850, 0 },
  { -2700, 850, 0 },
  { -2700, 1325, 0 },
  { -675, 1316, 0 },
  { -487, 1248, 0 },
  { -334, 1119, 0 },
  { -234, 946, 0 },
  { -200, 750, 0 },

  { -2700, 2150, 0 },
  { -2665, 1953, 0 },
  { -2565, 1780, 0 },
  { -2412, 1652, 0 },
  { -2225, 1583, 0 },
  { -200, 1575, 0 },
  { -200, 2050, 0 },
  { -2125, 2050, 0 },
  { -2125, 2250, 0 },
  { -200, 2250, 0 },
  { -200, 2725, 0 },
  { -2225, 2716, 0 },
  { -2412, 2648, 0 },
  { -2565, 2519, 0 },
  { -2665, 2346, 0 },
  { -2700, 2150, 0 },

  { -2150, -2700, 0 },
  { -2346, -2665, 0 },
  { -2519, -2565, 0 },
  { -2648, -2412, 0 },
  { -2716, -2225, 0 },
  { -2725, -200, 0 },
  { -2250, -200, 0 },
  { -2250, -2125, 0 },
  { -2050, -2125, 0 },
  { -2050, -200, 0 },
  { -1575, -200, 0 },
  { -1583, -2225, 0 },
  { -1652, -2412, 0 },
  { -1780, -2565, 0 },
  { -1953, -2665, 0 },
  { -2150, -2700, 0 },

  { -750, -200, 0 },
  { -946, -235, 0 },
  { -1119, -335, 0 },
  { -1248, -488, 0 },
  { -1316, -675, 0 },
  { -1325, -2700, 0 },
  { -850, -2700, 0 },
  { -850, -775, 0 },
  { -650, -775, 0 },
  { -650, -2700, 0 },
  { -175, -2700, 0 },
  { -183, -675, 0 },
  { -252, -488, 0 },
  { -380, -335, 0 },
  { -553, -235, 0 },
  { -750, -200, 0 },

  { 2700, -2150, 0 },
  { 2665, -2346, 0 },
  { 2565, -2519, 0 },
  { 2412, -2648, 0 },
  { 2225, -2716, 0 },
  { 200, -2725, 0 },
  { 200, -2250, 0 },
  { 2125, -2250, 0 },
  { 2125, -2050, 0 },
  { 200, -2050, 0 },
  { 200, -1575, 0 },
  { 2225, -1583, 0 },
  { 2412, -1652, 0 },
  { 2565, -1780, 0 },
  { 2665, -1953, 0 },
  { 2700, -2150, 0 },

  { 200, -750, 0 },
  { 235, -946, 0 },
  { 335, -1119, 0 },
  { 488, -1248, 0 },
  { 675, -1316, 0 },
  { 2700, -1325, 0 },
  { 2700, -850, 0 },
  { 775, -850, 0 },
  { 775, -650, 0 },
  { 2700, -650, 0 },
  { 2700, -175, 0 },
  { 675, -184, 0 },
  { 488, -252, 0 },
  { 335, -380, 0 },
  { 235, -553, 0 },
  { 200, -750, 0 },

  {   0,   0, 0 },
  {   0,   0, 2000 },

};

/*==============================================================================
 * Instance variables
 *============================================================================*/

    // The location of the object with respect to origo
    int x, y, z;          

    // The arrays for storing the coordinates 
    // (with respect to the location of the object)
    int px[];
    int py[];
    int pz[];

    // Arrays for calculating the current 2D projection for the object
    int vx[];
    int vy[];
    
/*==============================================================================
 * Constructors
 *============================================================================*/

    /**
     * Constructor: create the default object
     */
    public ThreeDeeObject(int hereX, int hereY, int hereZ) {

        // Set the location of the object
        moveTo(hereX, hereY, hereZ);
        
        // Allocate arrays for storing the individual point
        // coordinates of the object
        px = new int[length];
        py = new int[length];
        pz = new int[length];

        vx = new int[length];
        vy = new int[length];    

        for (int i = 0; i < length; i++) {
            px[i] = p[i][0];
            py[i] = p[i][1];
            pz[i] = p[i][2];
        }

    }

/*==============================================================================
 * Methods
 *============================================================================*/

    /**
     * size: return the number of points in the object.
     */
    public int size() {
        return length;
    }

    /**
     * getX: get the X coordinate of the n'th point of the object
     */
    public int getX(int offset) {
        return x + px[offset];
    }

    /**
     * getY: get the Y coordinate of the n'th point of the object
     */
    public int getY(int offset) {
        return y + py[offset];
    }

    /**
     * getZ: get the Z coordinate of the n'th point of the object
     */
    public int getZ(int offset) {
        return z + pz[offset];
    }

    /**
     * getZ, setZ: get and set the absolute Z coordinate of the object
     */
	public int getZ() {
		return z;
    }

	public void setZ(int newZ) {
		z = newZ;
    }

    /**
     * moveTo: move the object to given location.
     */
    public void moveTo(int hereX, int hereY, int hereZ) {
        x = hereX;
        y = hereY;
        z = hereZ;
    }

    /**
     * moveBy: move the object by given offsets.
     */
    public void moveBy(int moreX, int moreY, int moreZ) {
        x += moreX;
        y += moreY;
        z += moreZ;

		// Avoid clipping problems
		z = (z < clipPlane) ? z : clipPlane;
    }

    /**
     * rotate: rotate the object by given angles (angles are
     * defined in decimal degrees, i.e., full circle = 360).
     */
    public void rotate(int angleX, int angleY, int angleZ) {

        for (int i = 0; i < length; i++) {
            int rx = p[i][0];
            int ry = p[i][1];
            int rz = p[i][2];

            // Rotate around X axis
            if (angleX != 0) {
                int nry = ry;
                ry = Trigonometric.multiCos(ry, angleX) - 
                     Trigonometric.multiSin(rz, angleX);
    	        rz = Trigonometric.multiSin(nry, angleX) + 
                     Trigonometric.multiCos(rz, angleX);
            }

            // Rotate around Y axis
            if (angleY != 0) {
                int nrx = rx;
                rx = Trigonometric.multiCos(rx, angleY) - 
                     Trigonometric.multiSin(rz, angleY);
                rz = Trigonometric.multiSin(nrx, angleY) + 
                     Trigonometric.multiCos(rz, angleY);
            }
            
            // Rotate around Z axis
            if (angleZ != 0) {
                int nrx = rx;
                rx = Trigonometric.multiCos(rx, angleZ) - 
                     Trigonometric.multiSin(ry, angleZ);
                ry = Trigonometric.multiSin(nrx, angleZ) + 
                     Trigonometric.multiCos(ry, angleZ);
            }

            px[i] = rx;
            py[i] = ry;
            pz[i] = rz;
        }
    }

    /**
     * turn: turn the object with respect to the origo.
     */
    public void turn(int angleX, int angleY, int angleZ) {

        for (int i = 0; i < length; i++) {
            int rx = x;
            int ry = y;
            int rz = z;

            // Turn around X axis
            if (angleX != 0) {
                int nry = y;
                ry = Trigonometric.multiCos( ry, angleX) - 
                     Trigonometric.multiSin(rz, angleX);
    	        rz = Trigonometric.multiSin(nry, angleX) + 
                     Trigonometric.multiCos(rz, angleX);
            }

            // Turn around Y axis
            if (angleY != 0) {
                int nrx = rx;
                rx = Trigonometric.multiCos( rx, angleY) - 
                     Trigonometric.multiSin(rz, angleY);
                rz = Trigonometric.multiSin(nrx, angleY) + 
                     Trigonometric.multiCos(rz, angleY);
            }
            
            // Turn around Z axis
            if (angleZ != 0) {
                int nrx = rx;
                rx = Trigonometric.multiCos( rx, angleZ) - 
                     Trigonometric.multiSin(ry, angleZ);
                ry = Trigonometric.multiSin(nrx, angleZ) + 
                     Trigonometric.multiCos(ry, angleZ);
            }

            x = rx;
            y = ry;
            z = rz;
        }
    }

    /**
     * project: calculate a 2D projection for the object
     * based on the given viewpoint coordinates.
     */
    public void project(int currentX, int currentY, int currentZ)
    {
        int sx, sy, sz;
        int cx, cy;

        for (int i = 0; i < length; i++) {
            sx = getX(i) + currentX;
    	    sy = getY(i) + currentY;
    	    sz = getZ(i) + currentZ;

            cx = sx * planeDist / sz;
            cy = sy * planeDist / sz;

    	    // Note: for parallel (non-perspective) projection,
    	    // replace 'cx' and 'cy' below with 'sx' and 'sy'
    	    vx[i] = centerX + cx;
    	    vy[i] = centerY - cy;
        }
    }

    /**
     * display: display the current 2D project of the object.
     */
    public void display(Graphics g)
    {
        for (int i = 1; i < length; i++) {
			// Hardcoded numbers to avoid drawing 
			// when moving from one "U" to another
			// (there are better ways to do this,
			// but this will do for now...)
			if (i % 16 != 0)
				g.drawLine(vx[i-1], vy[i-1], vx[i], vy[i], g.PLAIN);
        }
    }

}

