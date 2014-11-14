/*
 *	FMIrefConstant.java	1.7	99/04/06 SMI
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
import Const;
import jcc.Str2ID;
import util.*;

/*
 * Represents an CONSTANT_Fieldref, CONSTANT_Methodref or
 * CONSTANT_InterfaceMethodref.
 */

public 
class FMIrefConstant extends ConstantObject
{
    // These fields are filled in by Clas.resolveConstant().
    public NameAndTypeConstant sig;
    public ClassConstant clas;

    boolean	computedID;
    int		ID;

    // These fields are read from the class file
    public int classIndex;
    public int sigIndex;

    FMIrefConstant(){ nSlots = 1;
		     }

    protected FMIrefConstant( int t, ClassConstant c, NameAndTypeConstant s ){
	tag = t;
	clas = c;
	sig = s;
	resolved = true;
	nSlots = 1;
    }
	

    public void read( DataInput in ) throws IOException {
	classIndex = in.readUnsignedShort();
	sigIndex = in.readUnsignedShort();
    }

    public void resolve( ConstantObject table[] ){
	if (resolved) return;
	try{ //DEBUG
	sig = (NameAndTypeConstant)table[sigIndex];
	clas = (ClassConstant)table[classIndex];
	} catch( RuntimeException t ){//DEBUG
	    System.out.println(Localizer.getString("fmirefconstant.trouble_processing", this.toString()));
	    throw t;
	}//end DEBUG
	resolved = true;
    }

    public void externalize( ConstantPool p ){
	sig = (NameAndTypeConstant)p.add( sig );
	clas = (ClassConstant)p.dup( clas );
    }

    public void write( DataOutput out) throws IOException {
	out.writeByte( tag );
	if ( resolved ){
	    out.writeShort( clas.index );
	    out.writeShort( sig.index );
	} else {
	    throw new DataFormatException(Localizer.getString("fmirefconstant.unresolved_fmirefconstant.dataformatexception"));
	    //out.writeShort( classIndex );
	    //out.writeShort( sigIndex );
	}
    }

    public String toString(){
	String t = (tag==Const.CONSTANT_FIELD)?/*NOI18N*/"FieldRef: ":
		   (tag==Const.CONSTANT_METHOD)?/*NOI18N*/"MethodRef: ":
		   /*NOI18N*/"InterfaceRef: ";
	if (resolved)
	    return t+clas.name.string+/*NOI18N*/" . "+sig.name.string+/*NOI18N*/" : "+sig.type.string;
	else
	    return t+/*NOI18N*/"[ "+classIndex+/*NOI18N*/" . "+sigIndex+/*NOI18N*/" ]";
    }

    public void incReference() {
	references++;
	sig.incReference();
	clas.incReference();
    }

    public void decReference() {
	references--;
	sig.decReference();
	clas.decReference();
    }

    public int hashCode() {
	return tag + sig.hashCode() + clas.hashCode();
    }

    public boolean equals(Object o) {
	if (o instanceof FMIrefConstant) {
	    FMIrefConstant f = (FMIrefConstant) o;
	    return tag == f.tag && clas.name.equals(f.clas.name) &&
		sig.name.equals(f.sig.name) && sig.type.equals(f.sig.type);
	} else {
	    return false;
	}
    }

    public int getID(){
	if ( ! computedID ){
	    ID = Str2ID.sigHash.getID( sig.name, sig.type );
	    computedID = true;
	}
	return ID;
    }

    public ClassComponent find( boolean isMethod ){
	if ( ! computedID ){
	    ID = Str2ID.sigHash.getID( sig.name, sig.type );
	    computedID = true;
	}
	ClassInfo c = ClassInfo.lookupClass( clas.name.string );
	while ( c != null ){
	    ClassMemberInfo t[] = isMethod ? (ClassMemberInfo[])c.methods : (ClassMemberInfo[])c.fields;
	    int l = t.length;
	    int thisID = this.getID();
	    for ( int i = 0; i < l ; i++ ){
		if ( thisID == t[i].getID() )
		    return t[i];
	    }
	    c = c.superClassInfo;
	}
	return null;
    }

    public boolean isResolved(){ return find( tag!=Const.CONSTANT_FIELD ) != null; }
}
