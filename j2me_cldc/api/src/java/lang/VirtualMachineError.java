/*
 * @(#)VirtualMachineError.java	1.10 99/12/04
 *
 * Copyright 1995-1999 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package java.lang;

/**
 * Thrown to indicate that the Java Virtual Machine is broken or has 
 * run out of resources necessary for it to continue operating. 
 *
 *
 * @author  Frank Yellin
 * @version 1.10, 12/04/99
 * @since   JDK1.0
 */
abstract public
class VirtualMachineError extends Error {
    /**
     * Constructs a <code>VirtualMachineError</code> with no detail message.
     */
    public VirtualMachineError() {
	super();
    }

    /**
     * Constructs a <code>VirtualMachineError</code> with the specified 
     * detail message. 
     *
     * @param   s   the detail message.
     */
    public VirtualMachineError(String s) {
	super(s);
    }
}
