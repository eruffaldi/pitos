/*
 *	CodeHacker.java	1.16	99/06/23 SMI
 *
 * Copyright (c) 1997,1999 Sun Microsystems, Inc. All Rights Reserved.
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
import components.*;
import java.io.PrintStream;
import Const;
import EVMConst;
import util.DataFormatException;
import util.SignatureIterator;

/*
 * This class "quickens" Java bytecodes.
 * That is, it looks at the instructions having symbolic references,
 * such as get/putfield and methodcalls, attempts to resolve the references
 * and re-writes the instructions in their JVM-specific, quick form.
 *
 * Resolution failures are not (any longer) considered fatal errors, they
 * just result in code that cannot be considered as read-only, as more
 * resolution will be required at runtime, as we refuse to re-write any
 * instructions having unresolved references.
 *
 * The following is an exception to the above rule (and an unfortunate hack):
 * References to array classes (having names starting in "[") are 
 * quickened anyway, as we have confidence that these classes, though not
 * instantiated when this code is run, will be instantiated in time.
 * (Perhaps this should be under control of a flag?)
 * 
 */
public
class CodeHacker {

    ConstantPool	pool;
    boolean		warn;
    boolean		verbose;
    PrintStream		log;
    boolean		useLosslessOpcodes = false;
    ClassInfo		java_lang_Object;

    private boolean	success;
    private byte        newopcode;


    public CodeHacker( ConstantPool p, boolean lossless, boolean w, boolean v ){
	pool = p;
	warn = w;
	verbose = v;
	log = System.out;
	java_lang_Object = ClassInfo.lookupClass("java/lang/Object");
	if ( java_lang_Object == null ){
	    log.println("CodeHacker: could not find java/lang/Object");
	}
	useLosslessOpcodes = lossless;
    }


    private void lookupFailed( ClassConstant me, FMIrefConstant target ){
	log.println("Quickening "+me.name.string+": lookup failed for "+target);
	success = false;
    }

    private static int getInt( byte code[], int w ){
	return	(  (int)code[w]   << 24 ) |
		(( (int)code[w+1] &0xff ) << 16 ) |
		(( (int)code[w+2] &0xff ) << 8 ) |
		 ( (int)code[w+3] &0xff );
    }

    private static int getUnsignedShort( byte code[], int w ){
	return	(( (int)code[w] &0xff ) << 8 ) | ( (int)code[w+1] &0xff );
    }

    private static void putShort( byte code[], int w, short v ){
	code[w]   = (byte)(v>>>8);
	code[w+1] = (byte)v;
    }

    private FMIrefConstant dup( FMIrefConstant c ){
	return (FMIrefConstant) pool.dup( c );
    }

    /*
     * Returns true if the reference was fully quickened.
     * Returns false if the reference could not be deleted, either because
     *   the lookup failed, or because we had to use the wide variant.
     * Sets this.success to false if the lookup failed or there was no
     * usable variant.
     */
    private boolean quickenFieldAccess(
	ClassConstant me,
	byte code[],
	int loc,
	boolean isStatic,
	ConstantObject c[],
	int oneWord,
	int twoWords,
	int wideReference,
	int refReference,
	int wideRefReference
    ){
	FieldConstant fc = (FieldConstant)c[ getUnsignedShort( code, loc+1 ) ];
	FieldInfo  fi = fc.find();
	if ( fi == null ){
	    //
	    // never even try to quicken anything we cannot resolve.
	    //
	    lookupFailed( me, fc );
	    return false;
	}
	byte newCode;
	switch (fc.sig.type.string.charAt(0) ){
	case '[':
	case 'L':
	    newCode = (byte)refReference;
	    wideReference = wideRefReference; // may be used below.
	    break;
	case 'D':
	case 'J':
	    newCode = (byte)twoWords;
	    break;
	default:
	    newCode = (byte)oneWord;
	    break;
	}
	if ( isStatic ) {
	    if ( fi.parent.vmClass.hasStaticInitializer ){
		// use a different quick opcode
		// if you have to check for running of static initializers!
		// be careful of opcode mapping relations.
		// This could get burned if things change too much!
		newCode +=  Const.opc_getstatic_checkinit_quick - Const.opc_getstatic_quick;
	    }
	    code[loc] = newCode;
	    return false; // still references constant pool!
	} else {
	    int fieldOffset = fi.instanceOffset;
	    // EVM wants field offset as actual WORD offset
	    // from the start of the object. This requires
	    // knowing the size of the header.
	    // JVM just wanted a 0-based word index.
	    // 
	    fieldOffset += EVMConst.ObjHeaderWords;
	    if ( fieldOffset < 0 ){
		lookupFailed( me, fc );
		log.println("Fieldoffset < 0 ");
		return false;
	    }
	    if ( fieldOffset > 0xFF ||  useLosslessOpcodes ){
		if ( wideReference == 0 ){
		    lookupFailed( me, fc );
		    log.println("need unavailable wide op");
		}
		code[loc] = (byte)wideReference;
		return false; // still references constant pool!
	    }
	    code[loc+1] =  (byte)fieldOffset;
	    code[loc+2] =  0;
	    code[loc] = newCode;
	    return true;
	}
    }

    /*
     * Should  a call to the specified methodblock be turned into
     * an invokesuper_quick instead of a invokenonvirtual_quick?
     *
     * The fourTHREE conditions that have to be satisfied:
     *    The method isn't private
     *    The method isn't an <init> method
     *    XXXXThe ACC_SUPER flag is set in the current class
     *    The method's class is a superclass (and not equal) to 
     *	     the current class.
     */
    private boolean
    isSpecialSuperCall( MethodInfo me, MethodInfo callee ){
	String name = callee.name.string;
	if ( (callee.access&Const.ACC_PRIVATE) != 0 ) return false;
	if ( name.equals("<init>") ) return false;
	ClassInfo myclass = me.parent;
	ClassInfo hisclass = callee.parent;
	if ( myclass == hisclass ) return false;
	// walk up my chain, looking for other's class
	while ( myclass != null ){
	    myclass = myclass.superClassInfo;
	    if ( myclass == hisclass ) return true;
	}
	return false;
    }

    private boolean quickenCode( MethodInfo m, ConstantObject c[] ) throws DataFormatException {
	byte code[] = m.code;
	int list[] = m.ldcInstructions;
	ConstantObject		co;
	FieldConstant		fc;
	MethodConstant		mc;
	NameAndTypeConstant	nt;
	ClassConstant		cc;
	String			t;
	ClassConstant		me = m.parent.thisClass;
	MethodInfo		mi;
	ClassInfo		ci;

	success = true;
	if (verbose){
	    log.println("Q>>METHOD "+m.name.string );
	}
	if ( list != null ){
	    for ( int i = 0; i < list.length; i++ ){
		int loc = list[i];
		if ( loc < 0 ) continue;
		switch( (int)code[loc]&0xff ){
		case Const.opc_ldc:
		    //
		    // no danger of lookup failure here,
		    // so don't even examine the referenced object.
		    //
		    co = c[(int)code[loc+1]&0xff];
		    if (co instanceof StringConstant) {
			code[loc] = (byte)Const.opc_aldc_quick;
		    } else {
			code[loc] = (byte)Const.opc_ldc_quick;
		    }
		    co.incldcReference();
		    break;
		default:
		    throw new DataFormatException( "unexpected opcode in ldc="+
			((int)code[loc]&0xff)+" at loc="+loc+
			" in "+m.qualifiedName() );
		}
	    }
	}
	list = m.wideConstantRefInstructions;
	if ( list != null ){
	    MethodInfo[] tList = null;
	    int tli = 0;		// index into tList
	    if (VMMethodInfo.SAVE_TARGET_METHODS) {
		tList = new MethodInfo[list.length];
	    }
	    for ( int i = 0; i < list.length; i++ ){
		int loc = list[i];
		if ( loc < 0 ) continue;
		co = c[ getUnsignedShort(code, loc+1) ];
		if ( ! co.isResolved() ){
		    //
		    // don't try to quicken unresolved references.
		    // this is not fatal!
		    //
		    // Do quicken if its a reference to an array!!
		    // What a hack!
		    if ( (co instanceof ClassConstant ) &&
			 ((ClassConstant)co).name.string.charAt(0) == Const.SIGC_ARRAY ){
			((ClassConstant)co).forget(); // never mind, we'll fix later...
		    } else {
			if ( warn ){
			    log.println("Warning: could not quicken reference from " + m.qualifiedName() + " to "+co );
			}
			continue;
		    }
		}
		switch( (int)code[loc]&0xff ){
		case Const.opc_ldc_w:
		    if (co instanceof StringConstant) {
			code[loc] = (byte)Const.opc_aldc_w_quick;
		    } else {
			code[loc] = (byte)Const.opc_ldc_w_quick;
		    }
		    co.incldcReference();
		    break;
		case Const.opc_ldc2_w:
		    code[loc] = (byte)Const.opc_ldc2_w_quick;
		    break;
		case Const.opc_getstatic:
		    quickenFieldAccess( me, code, loc, true, c,
			Const.opc_getstatic_quick,
			Const.opc_getstatic2_quick, 0,
			Const.opc_agetstatic_quick, 0);
		    break;
		case Const.opc_putstatic:
		    quickenFieldAccess( me, code, loc, true, c,
			Const.opc_putstatic_quick,
			Const.opc_putstatic2_quick, 0,
			Const.opc_aputstatic_quick, 0);
		    break;
		case Const.opc_getfield:
		    if (quickenFieldAccess( me, code, loc, false, c,
		      Const.opc_getfield_quick,
		      Const.opc_getfield2_quick,
		      Const.opc_getfield_quick_w,
		      Const.opc_agetfield_quick,
		      Const.opc_agetfield_quick_w) ){
			// doesn't reference constant pool any more
			list[i] = -1;
		    }
		    break;
		case Const.opc_putfield:
		    if (quickenFieldAccess( me, code, loc, false, c,
		      Const.opc_putfield_quick,
		      Const.opc_putfield2_quick,
		      Const.opc_putfield_quick_w,
		      Const.opc_aputfield_quick,
		      Const.opc_aputfield_quick_w) ){
			// doesn't reference constant pool any more
			list[i] = -1;
		    }
		    break;
		case Const.opc_invokevirtual:
		    mc = (MethodConstant)co;
		    mi = mc.find(); // must succeed, if isResolved succeeded!
		    int x = -1;
		    if ( (x=mi.methodTableIndex )<= 255 && ! useLosslessOpcodes ){
			if ( mi.parent == java_lang_Object ){
			    newopcode = (byte)Const.opc_invokevirtualobject_quick;
			} else {
			    String sig = mc.sig.type.string;
			    new SignatureIterator(sig) {
				public void do_array(int d, int st, int end) {
				    newopcode =
				        (byte)Const.opc_ainvokevirtual_quick;
				}
				public void do_object(int st, int end) {
				    newopcode =
				        (byte)Const.opc_ainvokevirtual_quick;
				}
				public void do_scalar(char c) {
				    switch(c) {
					case Const.SIGC_LONG:
					case Const.SIGC_DOUBLE:
					    newopcode = (byte)
						Const.opc_dinvokevirtual_quick;
					    break;
					case Const.SIGC_VOID:
					    newopcode = (byte)
						Const.opc_vinvokevirtual_quick;
					    break;
					default:
					    newopcode = (byte)
						Const.opc_invokevirtual_quick;
					    break;
				    }
				}
			    }.iterate_returntype();
			}
			code[loc] = newopcode;
			code[loc+1] = (byte) x;
			code[loc+2] = (byte)mi.argsSize;
			list[i] = -1; // doesn't reference constant pool any more
			if (VMMethodInfo.SAVE_TARGET_METHODS) {
			    // Save the target method info for inlining
			    tList[tli++] = mi;
			}
		    } else {
			//
			// big index OR useLosslessOpcodes
			//
			code[loc] = (byte)Const.opc_invokevirtual_quick_w;
		    }
		    break;
		case Const.opc_invokeinterface:
		    code[loc] = (byte)Const.opc_invokeinterface_quick;
		    break;
		case Const.opc_invokestatic:
		    mc = (MethodConstant)co;
		    mi = mc.find(); // must succeed, if isResolved succeeded!
		    code[loc] = (byte)((mi.parent.vmClass.hasStaticInitializer)
				? Const.opc_invokestatic_checkinit_quick
				: Const.opc_invokestatic_quick);
		    break;
		case Const.opc_new:
		    code[loc] = (byte)Const.opc_new_quick;
		    break;
		case Const.opc_anewarray:
		    code[loc] = (byte)Const.opc_anewarray_quick;
		    break;
		case Const.opc_checkcast:
		    code[loc] = (byte)Const.opc_checkcast_quick;
		    break;
		case Const.opc_instanceof:
		    code[loc] = (byte)Const.opc_instanceof_quick;
		    break;
		case Const.opc_multianewarray:
		    code[loc] = (byte)Const.opc_multianewarray_quick;
		    break;
		case Const.opc_invokespecial:
		    mc = (MethodConstant)co;
		    mi = mc.find(); // must succeed.
		    byte newop;
		    if ( false ){
			newop = (byte)Const.opc_invokesuper_quick;
		    } else {
			newop = (byte)Const.opc_invokenonvirtual_quick;
		    }
		    code[loc] = newop;
		    break;
		default: 
		    throw new DataFormatException( "unexpected opcode in wideConstantRef="+
			((int)code[loc]&0xff)+" at loc="+loc+
			" in "+m.qualifiedName() );
		}
	    }
	    // Alloc and copy to new targetMethods array
	    if (VMMethodInfo.SAVE_TARGET_METHODS) {
		m.targetMethods = new MethodInfo[tli];
		System.arraycopy(tList, 0, m.targetMethods, 0, tli);
	    }
	}
	return success;
    }

    public boolean
    quickenCode( ClassInfo c ){
	ConstantObject constants[] = c.constants;
	MethodInfo     method[]= c.methods;
	int n = method.length;
	int i = 0;
	boolean result = true;
	try {
	    for ( i = 0; i < n; i++ ){
		if ( ! quickenCode( method[i], constants ) )
		    result = false;
	    }
	} catch( DataFormatException e ){
	    System.err.println("Quickening "+method[i].qualifiedName()+" got exception:");
	    e.printStackTrace();
	    return false;
	}
	return result;
    }

}
