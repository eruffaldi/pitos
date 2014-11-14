/*
 *      PALMWriter.java     1.8     00/04/28     SMI
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
import Const;
import EVMConst;
import components.*;
import vm.*;
import jcc.Util;
import util.*;
import java.util.Vector;
import java.util.Enumeration;
import java.util.Hashtable;
import java.io.OutputStream;


/*
 * The CoreImageWriter for the Embedded VM
 */

public class PALMWriter extends KVMWriter implements CoreImageWriter, Const, EVMConst {

    public PALMWriter() { 
	relocatableROM = true;
    }


    public boolean setAttribute( String attribute ){
	if (attribute.equals("relocating")) { 
	    buildingRelocationTable = true;
	    return true;
	}
        return super.setAttribute(attribute);
    }

    
    protected void writeEpilog() {
	out.println("#include \"src/runtime/PalmMain.c\"");
	out.println("#endif");
    }

    protected void writeRelocationFile(ClassClass classes[]) { 
	out.println("#include <global.h>");
	out.println("#include <stddef.h>");
	out.println("#include <rom.h>");
	out.println();
	try { 
	    writeAllClassDefinitions(classes); 
	} catch (DataFormatException e) {}
	out.println("#if ROMIZING");
	out.println();

	Vector nativeMethods = new Vector();
        for (int i = 0; i < classes.length; i++){
	    boolean classHasNatives = false;
            EVMClass cc = (EVMClass)classes[i];
            if (cc.isPrimitiveClass() || cc.isArrayClass()) { 
                continue;
            }
            EVMMethodInfo  m[] = cc.methods;
            int nmethod = (m == null) ? 0 : m.length;
            for (int j = 0; j < nmethod; j++){
                EVMMethodInfo meth = m[j];
                MethodInfo mi = meth.method;
		if ((mi.access & Const.ACC_NATIVE) == 0) { 
		    continue;
		}
		out.println("extern void " + 
			    mi.getNativeName(true) + "(void);");
		nativeMethods.addElement(meth);
	    }
	}
	out.println();
	out.println();

	out.println("long NativeRelocationCount = " + nativeMethods.size() + ";");
	out.println("struct NativeRelocationStruct NativeRelocations[] = {");
	for (Enumeration e = nativeMethods.elements(); e.hasMoreElements();) {
	    EVMMethodInfo meth = (EVMMethodInfo)e.nextElement();
	    MethodInfo mi = meth.method;
	    ClassInfo  ci = mi.parent;
	    EVMClass   EVMci = (EVMClass)(ci.vmClass);
	    out.println("\tNATIVE_RELOCATION_METHOD(  \\");
	    out.println("\t\t" + EVMci.getNativeName()  + ", " 
			+ mi.index + ", \\");
	    out.println("\t\t" + mi.getNativeName(true) + ")" + 
			(e.hasMoreElements() ? "," : "") );
	}
	out.println("};");
	out.println();
	out.println("#endif");
    }
}

