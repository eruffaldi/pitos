/*
 * @(#)Error.java       1.11 99/12/04
 *
 * Copyright 1995-1999 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */

package java.lang;

/**
 * An <code>Error</code> is a subclass of <code>Throwable</code>
 * that indicates serious problems that a reasonable application
 * should not try to catch. Most such errors are abnormal conditions.
 * <p>
 * A method is not required to declare in its <code>throws</code>
 * clause any subclasses of <code>Error</code> that might be thrown
 * during the execution of the method but not caught, since these
 * errors are abnormal conditions that should never occur.
 *
 * @author  Frank Yellin
 * @version 1.11, 12/04/99
 * @~~~see     java.lang.ThreadDeath
 * @since   JDK1.0
 */
public
class Error extends Throwable {
    /**
     * Constructs an <code>Error</code> with no specified detail message.
     */
    public Error() {
        super();
    }

    /**
     * Constructs an Error with the specified detail message.
     *
     * @param   s   the detail message.
     */
    public Error(String s) {
        super(s);
    }
}
