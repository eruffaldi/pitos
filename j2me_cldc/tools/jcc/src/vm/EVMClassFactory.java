/*
 *	EVMClassFactory.java	1.2	99/05/11 SMI
 *
 * Copyright (c) 1999 Sun Microsystems, Inc. All Rights Reserved.
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

package vm;
import Const;
import components.ClassInfo;

public class EVMClassFactory implements VMClassFactory{
    
    public ClassClass newVMClass( ClassInfo c ){
	return new EVMClass( c );
    }

    private static boolean
    setType( String name, int value ){
	ClassInfo ci = ClassInfo.lookupClass( name );
	ClassClass cc;
	if ( (ci == null) || ( ( cc = ci.vmClass ) == null) ){
	    return false;
	}
	((EVMClass)cc).typeCode = value;
	return true;


    }

    public void
    setTypes(){
	setType( "void", Const.T_VOID );
	setType( "boolean", Const.T_BOOLEAN );
	setType( "char", Const.T_CHAR );
	setType( "byte", Const.T_BYTE );
	setType( "short", Const.T_SHORT );
	setType( "int", Const.T_INT );
	setType( "long", Const.T_LONG );
	setType( "float", Const.T_FLOAT );
	setType( "double", Const.T_DOUBLE );
    }
}
