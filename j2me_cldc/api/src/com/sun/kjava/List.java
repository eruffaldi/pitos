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
 * A class representing a list of Objects.
 *
 * Resembles <code>java.util.Vector</code>.
 */
public class List {
    private Object data[];
    private int count;

    /**
     * Create a new List, and make it small to start.
     */
    public List() {
	this(5);
    }

    /**
     * Create a new List.
     *
     * @param initSize the number of initial elements to allocate
     */
    public List(int initSize) {
	data = new Object[initSize];
	count = 0;
    }

    /**
     * What is the Object at a given index?  N.B. This does no bounds checking.
     *
     * @param i the index of the entry
     * @return the Object at that index.
     */
    public Object elementAt(int i) {
	return data[i];
    }

    /**
     * What is the size of this List?
     *
     * @return the number of Objects stored
     */
    public int size() {
	return count;
    }

    /**
     * Append an Object to the end, expanding the vector if necessary.
     *
     * @param i the value of the new datum
     */
    public void append(Object obj) {
	if (count >= data.length)
	    ensureCapacity(count+5);
	data[count++] = obj;
    }

    /**
     * Mark the vector as containing no Objects, and drop all references to
     * the Objects previously contained.
     */
    public void removeAllElements() {
	for(int i=0; i<count; i++) {
	    data[i] = null;
	}
	count = 0;
    }

    /**

    /**
     * What is the total capacity of this List?
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
	Object newData[] = new Object[newCap];
	for(int i=0; i<data.length; i++) {
	    newData[i] = data[i];
	    data[i] = null;
	}
	data = newData;
    }

  /**
   * Set the indexed element to an Object.
   * <p>Note: this is a replacement operation - it is not
   * an insertion into the list!
   *
   * @param o the Object to place in the List
   * @param pos the index at which to place it.
   */
  public boolean setElementAt(Object o, int pos) {
    if(pos > count) {
      return false;
    }
    data[pos] = o;
    return true;
  }
    
}
