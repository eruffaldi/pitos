/*
 * @(#)TimeZone.java    1.38 98/09/30
 *
 * (C) Copyright Taligent, Inc. 1996 - All Rights Reserved
 * (C) Copyright IBM Corp. 1996 - All Rights Reserved
 *
 * Portions copyright (c) 1996-1998 Sun Microsystems, Inc. All Rights Reserved.
 *
 *   The original version of this source code and documentation is copyrighted
 * and owned by Taligent, Inc., a wholly-owned subsidiary of IBM. These
 * materials are provided under terms of a License Agreement between Taligent
 * and Sun. This technology is protected by multiple US and International
 * patents. This notice and attribution to Taligent may not be removed.
 *   Taligent is a registered trademark of Taligent, Inc.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies. Please refer to the file "copyright.html"
 * for further important copyright and licensing information.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

package java.util;

import com.sun.cldc.util.TimeZoneImplementation;

/**
 * <code>TimeZone</code> represents a time zone offset, and also figures out daylight
 * savings.
 *
 * <p>
 * Typically, you get a <code>TimeZone</code> using <code>getDefault</code>
 * which creates a <code>TimeZone</code> based on the time zone where the program
 * is running. For example, for a program running in Japan, <code>getDefault</code>
 * creates a <code>TimeZone</code> object based on Japanese Standard Time.
 *
 * <p>
 * You can also get a <code>TimeZone</code> using <code>getTimeZone</code> along
 * with a time zone ID. For instance, the time zone ID for the Pacific
 * Standard Time zone is "PST". So, you can get a PST <code>TimeZone</code> object
 * with:
 * <blockquote>
 * <pre>
 * TimeZone tz = TimeZone.getTimeZone("PST");
 * </pre>
 * </blockquote>
 *
 * <p> This class is a pure subset of the java.util.TimeZone class in J2SE.
 * <p> The only time zone ID that is required to be supported is "GMT".
 * <p>
 * Apart from the methods and variables being subset, the semantics of the
 * getTimeZone() method may also be subset: custom IDs such as "GMT-8:00"
 * are not required to be supported.
 *
 * @see          Calendar
 * @version      1.0 (J2ME MIDP)
 * @author       Mark Davis, David Goldsmith, Chen-Lieh Huang, Alan Liu, Brian Modra
 */
public abstract class TimeZone {

  private static TimeZoneImplementation  defaultZone = null;
  private static String platform = null;
  private static String classRoot = null;


  public TimeZone() {
  }

  /**
   * Gets offset, for current date, modified in case of
   * daylight savings. This is the offset to add *to* GMT to get local time.
   * Gets the time zone offset, for current date, modified in case of daylight
   * savings. This is the offset to add *to* GMT to get local time. Assume
   * that the start and end month are distinct. This method may return incorrect
   * results for rules that start at the end of February (e.g., last Sunday in
   * February) or the beginning of March (e.g., March 1).
   *
   * @param era           The era of the given date (0 = BC, 1 = AD).
   * @param year          The year in the given date.
   * @param month         The month in the given date. Month is 0-based. e.g.,
   *                      0 for January.
   * @param day           The day-in-month of the given date.
   * @param dayOfWeek     The day-of-week of the given date.
   * @param millis        The milliseconds in day in <em>standard</em> local time.
   * @return              The offset to add *to* GMT to get local time.
   * @exception IllegalArgumentException the era, month, day,
   * dayOfWeek, or millis parameters are out of range
   */
  public abstract int getOffset(int era, int year, int month, int day,
                                int dayOfWeek, int millis);

  /**
   * Gets the GMT offset for this time zone.
   */
  public abstract int getRawOffset();

  /**
   * Queries if this time zone uses Daylight Savings Time.
   */
  public abstract boolean useDaylightTime();


  /**
   * Gets the ID of this time zone.
   * @return the ID of this time zone.
   */
  public String getID() {
    return null;
  }

  /**
   * Gets the <code>TimeZone</code> for the given ID.
   * @param ID the ID for a <code>TimeZone</code>, either an abbreviation such as
   * "GMT", or a full name such as "America/Los_Angeles".
   * <p> The only time zone ID that is required to be supported is "GMT".
   *
   * @return the specified <code>TimeZone</code>, or null if the given ID
   * cannot be understood.
   */

  /* <p>
   * The following is information for implementers. Applications should not need to be aware of this
   * or rely on it, because each implementation may do it differently:
   * <p>
   * The Calendar will look up a class the name of which includes the platform name.
   * The class name will take the form: <p>
   *
   * <code>{classRoot}.util.{platform}.CalendarImpl</code>
   *
   * <p>
   * The classRoot is derived from the system by looking up the system property "microedition.implpath"
   * If this property key is not found or the associated class is not present then "com.sun.cldc" is used.
   * <p>
   * The platform name is derived from the system by looking for the system property
   * "microedition.platform".
   * If this property key is not found or the associated class is not present then one of two default
   * directories are used. These are called "j2me" and "j2se". If the property
   * "microedition.configuration" is non-null then "j2me" is used, otherwise "j2se" is assumed.
   */
  public static synchronized TimeZone getTimeZone(String ID) {
    getDefault();
    return defaultZone.getInstance(ID);
  }

  /**
   * Gets the default <code>TimeZone</code> for this host.
   * The source of the default <code>TimeZone</code>
   * may vary with implementation.
   * @return a default <code>TimeZone</code>.
   */
  public static synchronized TimeZone getDefault() {
    if (defaultZone == null) {
      if (platform == null) {
        /* Setup the platform name */

/****
        platform = System.getProperty("microedition.platform");

        if (platform == null) {
          // Work out if we are running on a J2ME system
          if (System.getProperty("microedition.configuration") != null)
            platform = "j2me";
          else
            platform = "j2se";
        }
****/

        platform = "j2me";

        /* See if there is an alternate protocol class root */
        classRoot = System.getProperty("microedition.implpath");
        if (classRoot == null) {
          classRoot = "com.sun.cldc";
        }
      }

      try {
        /* Using the platform and protocol names lookup a class to implement the connection */
        Class clazz = Class.forName(classRoot+".util."+platform+".TimeZoneImpl");

        /* Construct a new instance */
        defaultZone = (TimeZoneImplementation)clazz.newInstance();
        defaultZone = (TimeZoneImplementation)defaultZone.getInstance(null);
      }
      catch (InstantiationException x) {}
      catch (ClassNotFoundException x) {}
      catch (IllegalAccessException x) {}
    }

    return defaultZone;
  }

  /** Gets all the available IDs supported.
   * @return  an array of IDs.
   */
  public static String[] getAvailableIDs() {
    getDefault();
    return defaultZone.getIDs();
  }
}
