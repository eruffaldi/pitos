/*
 *  Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

package javax.microedition.io;

import java.io.*;

/**
 * This interface defines the stream connection over which
 * content is passed.
 *
 * @author  Nik Shaylor
 * @version 1.0 12/21/99
 */
public interface ContentConnection extends StreamConnection {

    /**
     * Returns the type of content that the resource connected to is providing.
     * E.g. if the connection is via HTTP, then the value of the
     * <code>content-type</code> header field is returned.
     *
     * @return  the content type of the resource that the URL references,
     *          or <code>null</code> if not known.
     */
    public String getType();

    /**
     * Returns a string describing the encoding of the content which
     * the resource connected to is providing.
     * E.g. if the connection is via HTTP, the value of the
     * <code>content-encoding</code> header field is returned.
     *
     * @return  the content encoding of the resource that the URL references,
     *          or <code>null</code> if not known.
     */
    public String getEncoding();

    /**
     * Returns the length of the content which is being provided.
     * E.g. if the connection is via HTTP, then the value of the
     * <code>content-length</code> header field is returned.
     *
     * @return  the content length of the resource that this connection's URL
     *          references, or <code>-1</code> if the content length is
     *          not known.
     */
    public long getLength();

}

