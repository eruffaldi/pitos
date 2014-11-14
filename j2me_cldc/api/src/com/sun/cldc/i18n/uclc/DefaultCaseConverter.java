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

package com.sun.cldc.i18n.uclc;

import java.io.*;
import com.sun.cldc.i18n.*;

/**
 * Default class converting the case of characters.
 *
 * @author  Nik Shaylor
 * @version 1.0 04/04/2000
 */
public class DefaultCaseConverter {

   /**
     * Determines if the specified character is a lowercase character.
     * This is currently only supported for ISO-LATIN-1 characters: "a" through "z".
     *
     * @param   ch   the character to be tested.
     * @return  <code>true</code> if the character is lowercase;
     *          <code>false</code> otherwise.
     * @since   JDK1.0
     */
    public static boolean isLowerCase(char ch) {
        return ch >= 'a' && ch <= 'z';
    }

   /**
     * Determines if the specified character is an uppercase character.
     * This is currently only supported for ISO-LATIN-1 characters: "A" through "Z".
     *
     * @param   ch   the character to be tested.
     * @return  <code>true</code> if the character is uppercase;
     *          <code>false</code> otherwise.
     * @see     java.lang.Character#isLowerCase(char)
     * @see     java.lang.Character#toUpperCase(char)
     * @since   1.0
     */
    public static boolean isUpperCase(char ch) {
        return ch >= 'A' && ch <= 'Z';
    }

    /**
     * The given character is mapped to its lowercase equivalent; if the
     * character has no lowercase equivalent, the character itself is
     * returned.
     * This is currently only supported for ISO-LATIN-1 characters.
     *
     * @param   ch   the character to be converted.
     * @return  the lowercase equivalent of the character, if any;
     *          otherwise the character itself.
     * @see     java.lang.Character#isLowerCase(char)
     * @see     java.lang.Character#isUpperCase(char)
     * @see     java.lang.Character#toUpperCase(char)
     * @since   JDK1.0
     */
    public static char toLowerCase(char ch) {
        if (isUpperCase(ch))
          return (char)(ch + ('a' - 'A'));
        else
          return ch;
    }

    /**
     * Converts the character argument to uppercase; if the
     * character has no lowercase equivalent, the character itself is
     * returned.
     * This is currently only supported for ISO-LATIN-1 characters.
     *
     * @param   ch   the character to be converted.
     * @return  the uppercase equivalent of the character, if any;
     *          otherwise the character itself.
     * @see     java.lang.Character#isLowerCase(char)
     * @see     java.lang.Character#isUpperCase(char)
     * @see     java.lang.Character#toLowerCase(char)
     * @since   JDK1.0
     */
    public static char toUpperCase(char ch) {
        if (isLowerCase(ch))
          return (char)(ch - ('a' - 'A'));
        else
          return ch;
    }

    /**
     * Determines if the specified character is a digit.
     * This is currently only supported for ISO-LATIN-1 digits: "0" through "9".
     *
     * @param   ch   the character to be tested.
     * @return  <code>true</code> if the character is a digit;
     *          <code>false</code> otherwise.
     * @since   JDK1.0
     */
    public static boolean isDigit(char ch) {
        return ch >= '0' && ch <= '9';
    }

    /**
     * Returns the numeric value of the character <code>ch</code> in the
     * specified radix.
     * This is only supported for ISO-LATIN-1 characters.
     *
     * @param   ch      the character to be converted.
     * @param   radix   the radix.
     * @return  the numeric value represented by the character in the
     *          specified radix.
     * @see     java.lang.Character#isDigit(char)
     * @since   JDK1.0
     */
    public static int digit(char ch, int radix) {
        int value = -1;
        if (radix >= Character.MIN_RADIX && radix <= Character.MAX_RADIX) {
          if (isDigit(ch)) {
                          value = ch - '0';
          }
          else if (isUpperCase(ch) || isLowerCase(ch)) {
            // Java supradecimal digit
            value = (ch & 0x1F) + 9;
          }
        }
        return (value < radix) ? value : -1;
    }



}
