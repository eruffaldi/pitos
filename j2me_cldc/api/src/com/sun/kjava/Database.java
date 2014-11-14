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
import java.lang.*;

/**
 * This class serves as an interface to the PalmOS database
 * manager. It allows the user to create and access PalmOS
 * databases from KJava.
 */
public class Database {

/*=========================================================================
 * Constants and definitions
 *=======================================================================*/

        /**
         * Read-only mode.
         */
        public static final int READONLY = 1;

        /**
         * Write-only mode.
         */
        public static final int WRITEONLY = 2;

        /**
         * Read and write mode.
         */
        public static final int READWRITE = 3;

        /**
         * End of database (last record indicator).
         */
        public static final int ENDOFDATABASE = 0xFFFF;


/*=========================================================================
 * Static variables
 *=======================================================================*/

/*=========================================================================
 * Instance variables
 *=======================================================================*/

        /**
         * dbRef: stores the PalmOS-level database reference
         * when the database is open.
         */
        private int dbRef;

/*=========================================================================
 * Static methods
 *=======================================================================*/

        /**
         * Create a new database.
         */
        public static native boolean create(int cardNo, String name, int creatorID,
                                                int typeID, boolean resDB);

        /**
         * Open a database. Private function. Use the constructor for
         * opening a database instead.
         */
        private static native int open(int typeID, int creatorID, int mode);

/*=========================================================================
 * Constructors
 *=======================================================================*/

       /**
        * Open a database.
        */
        public Database(int typeID, int creatorID, int mode) {
            dbRef = open(typeID, creatorID, mode);
        }

/*=========================================================================
 * Instance methods
 *=======================================================================*/

        /**
         * Check if the database is open.
         */
        public boolean isOpen() {
            return dbRef != 0;
        }

        /**
         * Get the number of records in the database.
         */
        public native int getNumberOfRecords();

        /**
         * Read a database record into a Java byte array object.
         * Remember that PalmOS database record numbers start
         * from 0.
         */
        public native byte[] getRecord(int recordNumber);

        /**
         * Set the contents of a PalmOS database record.
         */
        public native boolean setRecord(int recordNumber, byte[] data);

        /**
         * Add a new record to the end of the database.
         */
        public boolean addRecord(byte[] data) {
                // Add to the end of database
                return setRecord(ENDOFDATABASE, data);
        }

        /**
         * Delete an existing record.
         */
        public native boolean deleteRecord(int recordNumber);

        /**
         * Read record to a pre-allocated buffer instead
         * of allocating a new bytearray each time.  Also
         * allow a record to be read partially if necessary.
         * Currently unimplemented.
         */
        public native int readRecordToBuffer(int recordNumber,
                int readOffset, int length,
                byte[] buffer, int writeOffset);

        /**
         * Set the contents of a database record.  Allows more
         * complex data manipulation than setRecord.
         * Currently unimplemented.
         */
        public native int writeRecordFromBuffer(int recordNumber,
                int writeOffset, int length,
                byte[] buffer, int readOffset);

        /**
         * Close the current database.
         */
        public native void close();

}

