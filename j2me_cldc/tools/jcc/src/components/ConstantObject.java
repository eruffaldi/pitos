/*
 *	ConstantObject.java	1.4	99/04/06 SMI
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
import java.io.IOException;
import Const;
import util.DataFormatException;

/*
 * An abstract class for representing objects in the constant pools
 * All constants are derived from here.
 * Note how this and all its derivatives are Cloneable.
 */

public
abstract class ConstantObject extends ClassComponent implements Cloneable
{
    public int references = 0;
    public int ldcReferences = 0;
    public boolean shared = false;
    public ConstantPool containingPool; // if shared == true, then container.

    public int index;

    // The type of constant pool entry.
    public int tag;

    // The number of slots this thing "uses" -- usually 1, sometimes 2
    public int nSlots;


    public void incldcReference() {
        ldcReferences++;
    }

    public void decldcReference() {
        ldcReferences--;
    }
 
    public void clearldcReference(){
        ldcReferences = 0;
    }

    // Some items are reference counted so that the most frequently
    // accessed ones can be placed in the first 256 entries of the
    // constant table.  Others are just marked as used so we know
    // to include them somewhere in the table.
    public void incReference() {
	references++;
    }

    public void decReference() {
	references--;
    }

    public void clearReference(){
	references = 0;
    }

    public Object clone(){
	ConstantObject newC;
	try {
	    newC = (ConstantObject)super.clone();
	    newC.references = 0;
	    newC.shared     = false;
	    newC.index      = 0;
	    return newC;
	} catch( CloneNotSupportedException e ){
	    e.printStackTrace();
	    System.exit(1);
	}
	return null;
    }

    public abstract boolean isResolved();


    static public ConstantObject readObject( DataInput i ) throws IOException{
	// read the tag and dispatch accordingly
	int tag = i.readUnsignedByte();
	switch( tag ){
	case Const.CONSTANT_UTF8:
	    return UnicodeConstant.read( tag, i );
	case Const.CONSTANT_INTEGER:
	case Const.CONSTANT_FLOAT:
	    return SingleValueConstant.read( tag, i );
	case Const.CONSTANT_DOUBLE:
	case Const.CONSTANT_LONG:
	    return DoubleValueConstant.read( tag, i );
	case Const.CONSTANT_STRING:
	    return StringConstant.read( tag, i );
	case Const.CONSTANT_NAMEANDTYPE:
	    return NameAndTypeConstant.read( tag, i );
	case Const.CONSTANT_CLASS:
	    return ClassConstant.read( tag, i );
	case Const.CONSTANT_FIELD:
	    return FieldConstant.read( tag, i );
	case Const.CONSTANT_METHOD:
	    return MethodConstant.read( tag, i );
	case Const.CONSTANT_INTERFACEMETHOD:
	    return InterfaceConstant.read( tag, i );
	default:
	    throw new DataFormatException("Format error (constant tag "+tag+" )");
	}
    }
}
