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
 
package com.sun.kjava;

/**
 * An object of this class represents a black and white bitmap. 
 */

public class Bitmap {

	/**
	 * The only member is an array that will be typecast to a
	 * BitmapType in the Palm OS. Thus, it is tightly coupled to the Palm OS
	 * and relies on this type not changing. This is done for efficiency as well
	 * as to reduce the allocation of a new object each time a bitmap is drawn.
	 */
	private short[] data;

	/**
	 * Constructor to create a bitmap.
	 * The array is the exact representation of a bitmap in the Palm OS
	 * including the headers and flags.
	 * @param data The Palm OS representation of a bitmap.
	 */
    public Bitmap(short[] data) { 
		this.data = data;
	}


	/**
	 * Constructor defines the bitmap. The bits of a bitmap are given as an
	 * array of bytes, each byte defining 8 bits of the bitmap. 
	 * <p>
	 * On the Palm OS, the width (in bytes) must be even. If a bitmap is
	 * constructed with an odd width, padding is automatically added. It is
	 * padded width that is given by a call to getWidth.
	 * The maximum width for a bitmap on this platform is currently 32.
	 * <p>
	 * @param width the width of the bitmap in bytes.
	 * @param pixels the bits of the object. 
	 */
	public Bitmap(short width, byte[] pixels) {

		boolean needPadding = width%2 != 0;
		short rowBytes = (short)(width + width%2);

		// Insert the padding into the pixels array if necessary
		if (needPadding) {
			int pixelsIndex = 0;
			byte[] newPixels = new byte[(pixels.length/width) * rowBytes];
			for (int i = 0; i < newPixels.length; i++) {
				if (i%rowBytes == width)
					newPixels[i] = 0;
				else
					newPixels[i] = pixels[pixelsIndex++];
			}
			pixels = newPixels;
		}

		// Calculate the number of rows in the bitmap
		short height = (short)(pixels.length / rowBytes);

		// Allocate the memory that will be typecast to a BitmapType
		data = new short[8 + (pixels.length+1)/2];

		// Fill the width and height fields
		data[0] = (short)(width*8); // must be in pixels
		data[1] = height;

		// Fill the rowBytes and flags fields
		data[2] = rowBytes;
		data[3] = 0;

		// Fill the reserved fields
		data[4] = 0;
		data[5] = 0;
		data[6] = 0;
		data[7] = 0;

		// Copy the pixel data into the remainder of the structure
		int pixelIndex;
		int dataIndex;
		for (dataIndex = 8; dataIndex < data.length; dataIndex++) {
			pixelIndex = (dataIndex-8)*2;
			data[dataIndex] = (short)
				(((pixels[pixelIndex] << 8) & 0xFF00) |
				 (pixels[pixelIndex+1] & 0xFF)); 
		}
	}

	/**
	 * Return the width of the space in pixels used to display the bitmap. This
	 * will be a multiple of 16 and so may not correspond with the width
	 * specified when constructing the bitmap.
	 * @return the width of the space in pixels used to display the bitmap.
	 */
	public int getWidth() { return (int)data[0]; }

	/**
	 * Return the number of rows in the bitmap.
	 * @return the number of rows in the bitmap
	 */
	public int getRows() { return (int)data[1]; }
}
