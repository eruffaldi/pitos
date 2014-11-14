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
 * A simple expandable vector of integers, similar to <code>java.util.Vector</code>.
 */
public class IntVector {
    private int data[];
    private int count;

    /**
     * Create a new IntVector, and make it small to start.
     */
    public IntVector() {
	this(5);
    }

    /**
     * Create a new IntVector.
     *
     * @param initSize the number of initial elements to allocate
     */
    public IntVector(int initSize) {
	data = new int[initSize];
	count = 0;
    }

    /**
     * What is the value at a given index?  N.B. This does no bounds checking.
     *
     * @param i the index of the entry
     * @return the integer at that index.
     */
    public int valueAt(int i) {
	return data[i];
    }

    /**
     * What is the size of this IntVector?
     *
     * @return the number of integers stored
     */
    public int size() {
	return count;
    }

    /**
     * Append an integer to the end, expanding the vector if necessary.
     *
     * @param i the value of the new datum
     */
    public void append(int i) {
	if (count >= data.length)
	    ensureCapacity(count+5);
	data[count++] = i;
    }

    /**
     * Mark the vector as containing no integers.
     */
    public void removeAllElements() {
	count = 0;
    }

    /**
     * What is the total capacity of this IntVector?
     *
     * @return the number of entries currently allocated space, not all
     * of which may be occupied.
     *
     * @see size()
     */
    public int capacity() {
	return data.length;
    }

    /**
     * Ensure there's room for some number of entries by any means necessary.
     *
     * @param newCap the desired new capacity
     */
    public void ensureCapacity(int newCap) {
	int newData[] = new int[newCap];
	for(int i=0; i<data.length; i++) {
	    newData[i] = data[i];
	}
	data = newData;
    }
    
}
