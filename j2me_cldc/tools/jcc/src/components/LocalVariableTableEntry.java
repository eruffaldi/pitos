/*
 *	LocalVariableTableEntry.java	1.2	98/02/19
 *
 * Copyright (c) 1997 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 */

package components;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/*
 * Classes compiled with -g have line number attributes on their
 * code. These give the correspondence between Java bytecode relative PC
 * and source lines (when used with the filename, too!)
 */
public
class LocalVariableTableEntry {
    public int pc0;
    public int length;
    public UnicodeConstant name;
    public UnicodeConstant sig;
    public int slot;

    public static final int size = 10; // bytes in class files

    LocalVariableTableEntry( int p, int l, UnicodeConstant n,
			     UnicodeConstant si, int sl ){
	pc0 = p;
	length = l;
	name = n;
	sig = si;
	slot = sl;
    }

    public void write( DataOutput o ) throws IOException {
	o.writeShort( pc0 );
	o.writeShort( length );
	o.writeShort( name.index );
	o.writeShort( sig.index );
	o.writeShort( slot );
    }
}

