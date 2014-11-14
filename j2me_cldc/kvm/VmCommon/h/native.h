/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
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
 *
 */

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Native function interface
 * FILE:      native.h
 * OVERVIEW:  This file contains the definitions of native functions
 *            needed by the Java virtual machine. The implementation
 *            is _not_ based on JNI (Java Native Interface),
 *            because it seems too complicated for small devices.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (hashing, nano libraries)
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Type definitions and global variables
 *=======================================================================*/

/*  Class pointer has been removed from native function lookup */
/*  algorithm in the PalmPilot version in order to avoid having */
/*  to load huge API classes -- rather, the programmer can introduce */
/*  native functions like function prototypes in C. */

typedef  void NativeFunctionType(void);
typedef  NativeFunctionType *NativeFunctionPtr;


typedef struct {
    const char *const name;
    const NativeFunctionPtr implementation;
} NativeImplementationType;

typedef struct {
    const char *const name;
    const NativeImplementationType *const implementation;
} ClassNativeImplementationType;

#define NATIVE_END_OF_LIST { NULL, NULL }

extern const ClassNativeImplementationType nativeImplementations[];

/*
 * prototypes
 */

void Java_java_lang_Object_getClass(void);
void Java_java_lang_Object_hashCode(void);
void Java_java_lang_System_identityHashCode(void);
void Java_java_lang_Object_notify(void);
void Java_java_lang_Object_notifyAll(void);
void Java_java_lang_Object_wait(void);
void Java_java_lang_Math_randomInt(void);
void Java_java_lang_Class_isInterface(void);
void Java_java_lang_Class_isPrimitive(void);
void Java_java_lang_Class_forName(void);
void Java_java_lang_Class_newInstance(void);
void Java_java_lang_Class_getName(void);
void Java_java_lang_Class_isInstance(void);
void Java_java_lang_Class_isArray(void);
void Java_java_lang_Class_isAssignableFrom(void);
void Java_java_lang_Class_getPrimitiveClass(void) ;
void Java_java_lang_Class_exists(void);
void Java_java_lang_Class_getResourceAsStream0(void);
void Java_java_lang_Thread_activeCount(void);
void Java_java_lang_Thread_currentThread(void);
void Java_java_lang_Thread_yield(void);
void Java_java_lang_Thread_sleep(void);
void Java_java_lang_Thread_start(void);
void Java_java_lang_Thread_isAlive(void);
void Java_java_lang_Thread_setPriority0(void);
void Java_java_lang_Thread_setPriority(void);
void Java_java_lang_Runtime_exitInternal(void);
void Java_java_lang_Runtime_freeMemory(void);
void Java_java_lang_Runtime_totalMemory(void);
void Java_java_lang_Runtime_gc(void);
void Java_java_lang_System_arraycopy (void);
void Java_java_lang_System_currentTimeMillis(void);
void Java_java_lang_System_getProperty0(void);
void Java_java_lang_System_getProperty(void);
void Java_java_lang_String_intern(void);
void Java_java_util_Calendar_init(void);
void Java_java_io_PrintStream_putchar(void);

void Java_com_sun_kjava_DebugIO_putchar(void);
void Java_com_sun_kjava_Spotlet_register(void);
void Java_com_sun_kjava_Spotlet_unregister(void);
void Java_com_sun_kjava_Spotlet_register0(void);
void Java_com_sun_kjava_Spotlet_setPalmEventOptions(void);

void Java_com_sun_kjava_Spotlet_unregister0(void);
void Java_com_sun_kjava_Spotlet_beamSend(void);
void Java_com_sun_kjava_Spotlet_getFlashID(void);
void Java_com_sun_kjava_Graphics_copyOffScreenRegion(void);
void Java_com_sun_kjava_Graphics_copyRegion(void);
void Java_com_sun_kjava_Graphics_drawBitmap(void);
void Java_com_sun_kjava_Graphics_drawBorder(void);
void Java_com_sun_kjava_Graphics_drawLine(void);
void Java_com_sun_kjava_Graphics_drawRectangle(void);
void Java_com_sun_kjava_Graphics_drawString(void);
void Java_com_sun_kjava_Graphics_getHeight(void);
void Java_com_sun_kjava_Graphics_getWidth(void);
void Java_com_sun_kjava_Graphics_playSound(void);
void Java_com_sun_kjava_Graphics_resetDrawRegion(void);
void Java_com_sun_kjava_Graphics_setDrawRegion(void);
void Java_com_sun_kjava_Database_createDatabase(void);
void Java_com_sun_kjava_Database_openDatabase(void);
void Java_com_sun_kjava_Database_closeDatabase(void);
void Java_com_sun_kjava_Database_getNumberOfRecords(void);
void Java_com_sun_kjava_Database_getRecord(void);
void Java_com_sun_kjava_Database_setRecord(void);
void Java_com_sun_kjava_Database_deleteRecord(void);
void printString(INSTANCE string);

/***
void Java_com_sun_cldc_io_j2me_storage_Protocol_open0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_createDirectory0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_delete0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_rename0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_getLength0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_getModificationDate0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_isDirectory0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_read0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_readBytes0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_available0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_write0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_writeBytes0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_close0(void);
void Java_com_sun_cldc_io_j2me_storage_Protocol_seek0(void);
***/

void Java_com_sun_cldc_io_j2me_datagram_Protocol_open0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_send0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_receive0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_close0(void);

void Java_com_sun_cldc_io_j2me_socket_Protocol_open0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_read0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_write0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_available0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_close0(void);

void Java_com_sun_cldc_io_j2me_serversocket_Protocol_open(void);
void Java_com_sun_cldc_io_j2me_serversocket_Protocol_accept(void);
void Java_com_sun_cldc_io_j2me_serversocket_Protocol_close(void);

void Java_com_sun_cldc_io_j2me_debug_PrivateOutputStream_putchar(void);

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_open(void);
void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_close(void);
void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readInt(void);
void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readByteArray(void);

#if INCLUDE_ALL_CLASSES
void BetterHandleEvent(ulong64);
#endif

/*=========================================================================
 * Operations on native functions
 *=======================================================================*/

NativeFunctionPtr getNativeFunction(const char* className,
                                                                        const char* methodName);

void  invokeNativeFunction(METHOD thisMethod);
void  nativeInitialization(int *argc, char **argv);

#if INCLUDEDEBUGCODE
void printNativeFunctions(void);
#endif
