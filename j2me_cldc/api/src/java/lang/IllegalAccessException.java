/*
 * @(#)IllegalAccessException.java      1.8 99/12/04
 *
 * Copyright 1995-1999 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package java.lang;

/**
 * Thrown when an application tries to load in a class, 
 * but the currently executing method does not have access to the 
 * definition of the specified class, because the class is not public 
 * and in another package. 
 * <p>
 * An instance of this class can also be thrown when an application 
 * tries to create an instance of a class using the 
 * <code>newInstance</code> method in class <code>Class</code>, but 
 * the current method does not have access to the appropriate 
 * zero-argument constructor. 
 *
 * @author  unascribed
 * @version 1.8, 12/04/99
 * @see     java.lang.Class#forName(java.lang.String)
 * @see     java.lang.Class#newInstance()
 * @~~~see     java.lang.ClassLoader#findSystemClass(java.lang.String)
 * @~~~see     java.lang.ClassLoader#loadClass(java.lang.String, boolean)
 * @since   JDK1.0
 */
public class IllegalAccessException extends Exception {
    /**
     * Constructs an <code>IllegalAccessException</code> without a 
     * detail message. 
     */
    public IllegalAccessException() {
        super();
    }

    /**
     * Constructs an <code>IllegalAccessException</code> with a detail message. 
     *
     * @param   s   the detail message.
     */
    public IllegalAccessException(String s) {
        super(s);
    }
}
