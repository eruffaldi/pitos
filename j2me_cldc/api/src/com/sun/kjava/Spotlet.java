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

package com.sun.kjava;

import java.io.*;
import javax.microedition.io.*;

/**
 * This class provides callbacks for event handling. Applications extend this
 * class and override the relevant event handling methods. An application may
 * use more than one Spotlet object, but at most one Spotlet can have the
 * <i>focus</i> at any one time. That is, events will only trigger the
 * callbacks of one Spotlet at any given time, the Spotlet with the current
 * focus.
 * <p> To become the focus, a Spotlet invokes the <code>register</code> method
 * which also removes the focus from the previously registered Spotlet (if any).
 */
public class Spotlet {

    /**
     * Constants for the page up/down "hard" keys.
     */
    public static final int PAGEUP = 0x0B;
    public static final int PAGEDOWN = 0x0C;

    /**
     * Constants for the other Palm system "hard" keys.
     */
    public static final int KEY_HARD1 = 0x0204;
    public static final int KEY_HARD2 = 0x0205;
    public static final int KEY_HARD3 = 0x0206;
    public static final int KEY_HARD4 = 0x0207;
    public static final int KEY_POWER = 0x0208;

    /**
     * Constant for the calculator icon.
     */
    public static final int CALCICON = 0x010B;

    /**
     * Constant for the menu icon.
     */
    public static final int MENUICON = 0x0105;

    /**
     * Constants for the eventOptions of register().
     */
    public static final int NO_EVENT_OPTIONS = 0x00;
    public static final int WANT_SYSTEM_KEYS = 0x01;


    private static InputConnection con;
    private static DataInput in;
    private static Spotlet current;

    static {
        try {
            con = (InputConnection)Connector.open("events:");
            in  = (DataInput)con.openInputStream();
            new Thread() {
                public void run() {
                    while(true) {
                        try {
                            int event = in.readInt();
                            if(current != null) {
                                current.dispatch(event, in);
                            }
                        } catch(Exception x) {
                            System.out.println(x);
                        }
                    }
                }
            }.start();
        } catch(IOException x) {
            System.out.println(x);
        }
    }

    private static final int beamReceiveKVMEvent = 0;
    private static final int penDownKVMEvent     = 1;
    private static final int penUpKVMEvent       = 2;
    private static final int penMoveKVMEvent     = 3;
    private static final int keyDownKVMEvent     = 4;
    private static final int appStopKVMEvent     = 5;

    /*
     * Dispatcher for this class
     */
    public void dispatch(int event, DataInput in) throws IOException {
        switch(event) {
            case penDownKVMEvent:       penDown(in.readInt(), in.readInt());    break;
            case penUpKVMEvent:         penUp(in.readInt(), in.readInt());      break;
            case penMoveKVMEvent:       penMove(in.readInt(), in.readInt());    break;
            case keyDownKVMEvent:       keyDown(in.readInt());                  break;
            case beamReceiveKVMEvent: {
                                        // Start of temporary hack
                                        com.sun.cldc.io.j2me.events.Protocol e = (com.sun.cldc.io.j2me.events.Protocol) con;
                                        beamReceive(e.readByteArray(in));
                                        break;
                                        // End of temporary hack
            }
            default:                    unknownEvent(event, in);                break;
        }
    }

    /**
     * Catchall routine
     */
    public void unknownEvent(int event, DataInput in) {
    }

    /**
     * Register the event handlers of this object. This effectively makes this
     * Spotlet the <i>focus</i> for event handling. A side effect
     * this is that all previously registered handlers (if any) are
     * unregistered and the Spotlet to which they belong loses the focus.
     *
     * @param eventOptions one of NO_EVENT_OPTIONS or WANT_SYSTEM_KEYS
     */
    public void register(int eventOptions) {
        current = this;
        setPalmEventOptions(eventOptions);  // new-style event handling
    }

    public static native void setPalmEventOptions(int eventOptions);


    /**
     * Unregister the event handlers of this object. It is only necessary to
     * use this method when not transferring the <i>focus</i> from this Spotlet
     * to another one via a subsequent call to <code>register</code>. If this
     * Spotlet does not currently have the focus, this method does nothing.
     */
    public void unregister() {
        current = null;
    }


    /**
     * This method is invoked if the user places the pen on the display.
     *
     * @param x the x coordinate of the point at which the pen was placed
     * @param y the y coordinate of the point at which the pen was placed
     */
    public void penDown(int x, int y) {
    }

    /**
     * This method is invoked if the user removes the pen from the display.
     *
     * @param x the x coordinate of the point from which the pen was removed
     * @param y the y coordinate of the point from which the pen was removed
     */
    public void penUp(int x, int y) {
    }

    /**
     * This method is invoked if the user moves the pen over the display.
     *
     * @param x the x coordinate of the destination point of the move
     * @param y the y coordinate of the destination point of the move
     */
    public void penMove(int x, int y) {
    }

    /**
     * This method is invoked if the user presses either of the page up or page
     * down hard keys, taps the calculator or menu icon, or enters a character
     * (e.g. via Graffiti). If it is one of the hard key presses, then it will
     * match one of the corresponding constants defined in this class.
     *
     * @param keyCode the code of the key the user entered
     */
    public void keyDown(int keyCode) {
    }

    /**
     * This method is used for receiving packets of data via infrared
     * from other Palm devices. The data that is read is received in
     * a byte array that is allocated automatically by the virtual machine.
     */
    public void beamReceive(byte[] data) {
    }

    /**
     * This method is used for beaming data packets via infrared to
     * another Palm device.
     *
     * IMPORTANT: Unlike the methods above, this method is not an
     * event handler.  Rather, you call this method explicitly to
     * beam data to another device.  The other device must have
     * registered a beamReceive handler in its current Spotlet
     * to receive data.
     *
     * @return true if beaming succeeded, false otherwise.
     */
    public native static boolean beamSend(byte[] data);

    /**
     * This method is used to get the flashID of the Palm device.
     *
     * IMPORTANT: Unlike the methods above, this method is not an
     * event handler.
     *
     * @return a String containing the flashID.
     */
    public native static String getFlashID();

}

