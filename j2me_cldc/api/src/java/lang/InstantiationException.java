/*
 * @(#)InstantiationException.java	1.13 99/12/04
 *
 * Copyright 1995-1999 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package java.lang;

/**
 * Thrown when an application tries to create an instance of a class 
 * using the <code>newInstance</code> method in class 
 * <code>Class</code>, but the specified class object cannot be 
 * instantiated because it is an interface or is an abstract class. 
 *
 * @author  unascribed
 * @version 1.13, 12/04/99
 * @see     java.lang.Class#newInstance()
 * @since   JDK1.0
 */
public
class InstantiationException extends Exception {
    /**
     * Constructs an <code>InstantiationException</code> with no detail message.
     */
    public InstantiationException() {
	super();
    }

    /**
     * Constructs an <code>InstantiationException</code> with the 
     * specified detail message. 
     *
     * @param   s   the detail message.
     */
    public InstantiationException(String s) {
	super(s);
    }
}
