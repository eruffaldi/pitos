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

abstract public class List {
    static final int lineHeight = 11;

    private int topY;			// top of List
    private int height;			// height of List
    private int mainPageSize;	// number of items visible at one time

    private int recordCount;	// total number of records
    private int currentRecord;	// selected record
    private int firstRecord, lastRecord; // records currently visible

    static Graphics g = Graphics.getGraphics();

    // static final int cursorWidth = g.getWidth("-") + 2;

    // Create a new List object, that takes up the indicated vertical space
	// This >>does not<< draw it.
    public List(int topY, int bottomX) {
        this.topY = topY;
        this.height = bottomX - topY;
        this.mainPageSize = height / lineHeight;
    }

    // Paint the list, marking the specified item as the selection.
    // We try to put the currentRecord at the top of the page.  
    public void paint(int currentRecord, int recordCount) {
        // get the info from the user
        this.currentRecord = currentRecord;
        this.recordCount = recordCount;
        if (recordCount == 0) {
            // clear the screen
            g.drawRectangle(0, topY, 159, height, g.ERASE, 0);
            return;
        }
        // Set currentRecord to a reasonable value
        if (currentRecord < 0) { 
            currentRecord = 0;
        } else if (currentRecord >= recordCount) { 
            currentRecord = recordCount - 1;
        }
        // Try to make the current record be the first one on the screen, 
        // but we want at least one screenful.
        int newFirstRecord = currentRecord;
        if (currentRecord + (mainPageSize - 1) >= recordCount) { 
            newFirstRecord = (recordCount - 1) - (mainPageSize - 1);
            if (newFirstRecord < 0) { 
                newFirstRecord = 0;
            }
        }
        paintInternal(newFirstRecord);
    }

    private void 
    paintInternal(int newFirstRecord) { 
        // Set the boundaries of what we're drawing;
        firstRecord = newFirstRecord;
        lastRecord = newFirstRecord + (mainPageSize - 1);
        if (lastRecord >= recordCount) { 
            lastRecord = recordCount - 1;
        }
        // clear the screen
        g.drawRectangle(0, topY, 159, height, g.ERASE, 0);
        // Display each of the records
        for (int i = firstRecord; i <= lastRecord; i++) { 
            displayNoteHeader(i, false);
        }
    }
    
	// Change the "currentRecord" to be the indicated one.  If the new
	// selection isn't visible, it is made visible.
    public void
    setCurrent(int newCurrentRecord) {
        int oldCurrentRecord = currentRecord;
        if (newCurrentRecord < firstRecord || newCurrentRecord > lastRecord) { 
            // This sets currentRecord to the right value
            paint(newCurrentRecord, recordCount);
        } else if (newCurrentRecord == oldCurrentRecord) { 
            // why did we bother?
        } else { 
            currentRecord = newCurrentRecord;
            fixupInversion(oldCurrentRecord, false);
            fixupInversion(newCurrentRecord, true);
        }
    }

    private void
    fixupInversion(int index, boolean isCurrent) {
        if (index < firstRecord || index > lastRecord) 
            return;
        int y = (index - firstRecord) * lineHeight + topY;
		// For now, these two branches of the "if" do the same thing.  
		// But if we ever bring the cursor back, they'll be different.
        if (isCurrent) { 
            // g.drawString("-", 0, y, g.PLAIN);
            // Invert the selection
            g.drawRectangle(0, y, 159, lineHeight, g.INVERT, 0);
        } else { 
            // Invert the selection
            g.drawRectangle(0, y, 159, lineHeight, g.INVERT, 0);
            // Erase the old cursor
            // g.drawRectangle(0, y, cursorWidth, lineHeight, g.ERASE, 0);
        }
    }
    
	// Use this to indicate that a header should be updated.
    public void update(int index) { 
        if (index >= firstRecord && index <= lastRecord) { 
            displayNoteHeader(index, true);
        }
    }


    // display the note record header
    private void 
    displayNoteHeader(int index, boolean erase) {
        // compute the position of the old record on the display
        int y = (index - firstRecord) * lineHeight + topY;
        // Either erase the whole line, or just the cursor part
        if (erase) { 
            g.drawRectangle(0, y, 159, lineHeight, g.ERASE, 0); 
        } else { 
            // Erase the cursor 
            // g.drawRectangle(0, y, cursorWidth, lineHeight, g.ERASE, 0); 
        }

        // Get info about this record from the subclass
        g.drawString(getRecordInfo(index), /* cursorWidth */0, y, g.PLAIN);

        // Draw the cursor, if this item is selected
        if (index == currentRecord) { 
            // g.drawString("-", 0, y, g.PLAIN);
            g.drawRectangle(0, y, 159, lineHeight, g.INVERT, 0);
        }
    }

	// Subclasses need to define this.  This returns a String giving
	// information about the indicated record.
    abstract String getRecordInfo(int index);

    // Scroll up by either a full page, or by 1
    public void 
    prev(boolean fullPage) { 
        if (recordCount <= 0)
            return;
        int newFirstRecord = firstRecord - (fullPage ? (mainPageSize - 1) : 1);
        if (newFirstRecord < 0) { 
            newFirstRecord = 0;
        } 
        if (newFirstRecord != firstRecord) { 
            paintInternal(newFirstRecord);
        }
    }

    // Scroll down by either a full page, or by 1
    public void 
    next(boolean fullPage) { 
        int newFirstRecord = firstRecord + (fullPage ? (mainPageSize - 1) : 1);
        if (newFirstRecord >= recordCount) { 
            newFirstRecord = recordCount - mainPageSize;
            if (newFirstRecord < firstRecord) { 
                newFirstRecord = firstRecord;
            }
        }
        if (newFirstRecord != firstRecord) { 
            paintInternal(newFirstRecord);
        }
    }


    // If the user has selected an item, redraw the selection and return the
	// index of that selection.  Otherwise, return -1.
    public int 
    penDown(int x, int y) { 
        int offset = y - topY;
        if (offset < 0 || offset >= height) { 
            return -1;
        }
        int newCurrentRecord = firstRecord + (offset / lineHeight);
        if (newCurrentRecord <= lastRecord) { 
            setCurrent(newCurrentRecord);
            return newCurrentRecord;
        } else { 
            return -1;
        }
    }

	// For occasional debugging.  This overwrites the buttons at the top.
    private void 
    debug(String x) { 
        g.drawString(x, 5, 5, g.PLAIN);
    }

}

