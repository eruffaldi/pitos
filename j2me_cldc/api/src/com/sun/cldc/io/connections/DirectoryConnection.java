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


/*
 * WARNING - THIS IS AN EXPERIMENTAL FEATURE OF KVM THAT MAY, OR MAY NOT
 * EXIST IN A FUTURE VERSION OF THIS PRODUCT. IT IS NOT A PART OF THE
 * CLDC SPECIFICATION AND IS PROVIDED FOR ILLUSTRATIVE PURPOSES ONLY
 */


package com.sun.cldc.io.connections;

import java.io.*;
import java.util.*;
import javax.microedition.io.*;

/**
 * Metadata connection
 *
 * @author  Nik Shaylor
 * @version 1.0 2/9/2000
 */
public interface DirectoryConnection extends RandomAccessConnection {

//
// Collection size functions
//

    /**
     * Return the size in bytes that the collection can grow to
     *
     * @return  size
     */
    public long getAvailableSpace() throws IOException;

    /**
     * Return the number of items in the collection
     *
     * @return  size
     */
    public int getItemCount() throws IOException;

//
// Item selection
//

    /**
     * Select the first record int the database
     *
     * @return true if there was a record false if the collection was empty
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean selectFirstItem() throws IOException;

    /**
     * Select the next record int the database
     *
     * @return true if there was another record false otherwise
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean selectNextItem() throws IOException;

    /**
     * Select an item in the collection
     * @param name the name of the item to select
     * @return true if the record exists
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean selectItem(String name) throws IOException;

    /**
     * Select an item in the collection
     * @param i the record number
     * @return true if the record exists
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean selectItem(int i) throws IOException;

    /**
     * Unselect the current item. This is a way to unlock a record without locking another one.
     * @exception  IOException  if an I/O error occurs.
     */
    public void deselectItem() throws IOException;

//
// Metadata management
//

    /**
     * Test to see if the current item is a directory
     * @return true if it is, false if it is not
     */
    public boolean isDirectory() throws IOException;

    /**
     * Create a data item in the collection with a randomly chosen unique name and select it
     * @exception  IOException  if an I/O error occurs.
     */
    public void create() throws IOException;

    /**
     * Create a data item with the supplied name and select it
     * @param name the name of the item to create
     * @exception  IOException  if an I/O error occurs.
     */
    public void create(String name) throws IOException;

    /**
     * Create an item with the supplied number
     * @param i the record number
     * @exception  IOException  if an I/O error occurs.
     */
    public void create(int i) throws IOException;

    /**
     * Create a directory with the supplied name
     * @param name the name of the directory to create
     * @exception  IOException  if an I/O error occurs.
     */
    public void createDirectory(String name) throws IOException;

    /**
     * Delete the current data item from the collection
     * @exception  IOException  if an I/O error occurs.
     */
    public void delete() throws IOException;

    /**
     * Delete a directory
     * @exception  IOException  if an I/O error occurs.
     */
    public void deleteDirectory() throws IOException;

    /**
     * Rename the current data item
     * @param name the new name for the item
     * @exception  IOException  if an I/O error occurs.
     */
    public void rename(String newName) throws IOException;

    /**
     * Rename a directory with the supplied name
     * @param to the new name for the directory
     * @exception  IOException  if an I/O error occurs.
     */
    public void renameDirectory(String to) throws IOException;

    /**
     * Return the number of the current record
     * @return  the name of the item or null if no item is selected
     * @exception  IOException  if an I/O error occurs.
     */
    public String getItemName() throws IOException;

    /**
     * Return the number of the current item
     * @return the current item number
     * @exception  IOException  if an I/O error occurs.
     */
    public int getItemNumber() throws IOException;

    /**
     * Return the date that the item was last modified
     * @return  date or -1 if the record was undated.
     * @exception  IOException  if an I/O error occurs.
     */
    public long getModificationDate() throws IOException;

//
// R/W bits
//

    /**
     * Test to see if the selected item can be read
     * @return  true if the item can be read, else false.
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean canRead() throws IOException;

     /**
     * Set or clear the read bit
     * @param tf the new value for the read bit
     * @exception  IOException  if an I/O error occurs.
     */
    public void setReadable(boolean tf) throws IOException;

    /**
     * Test to see if the selected item can be written
     * @return  true if the item can be written, else false.
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean canWrite() throws IOException;

     /**
     * Set or clear the write bit
     * @param tf the new value for the write bit
     * @exception  IOException  if an I/O error occurs.
     */
    public void setWritable(boolean tf) throws IOException;

}
