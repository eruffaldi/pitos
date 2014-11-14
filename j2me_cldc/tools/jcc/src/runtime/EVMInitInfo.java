/*
 *	EVMInitInfo.java	1.1	99/05/03	SMI
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
package runtime;

class EVMInitInfo {
    /*
     * This is just a tripl of:
     *     from-address
     *     to-address
     *     byte count
     * that get stuffed in an array-of-triples
     * for interpretation by the startup code.
     */
    String fromAddress;
    String toAddress;
    String byteCount;
    EVMInitInfo next;

    EVMInitInfo( String f, String t, String c, EVMInitInfo n ){
	fromAddress = f;
	toAddress = t;
	byteCount = c;
	next = n;
    }

    static EVMInitInfo initList = null;

    public static void
    addInfo( String f, String t, String c ){
	initList = new EVMInitInfo( f, t, c, initList );
    }

    public static void
    write( CCodeWriter out, String typename, String dataname, String nname ){
	int n = 0;
	out.println("const struct "+typename+" "+dataname+"[] = {");
	for ( EVMInitInfo p = initList; p != null; p = p.next ){
	    out.println("    { "+p.fromAddress+", "+p.toAddress+", "+p.byteCount+" },");
	    n++;
	}
	out.println("};");
	out.println("const int "+nname+" = "+n+";");
    }
}
