/* This is a generated file.  Do not modify.
 * Generated on Thu Jan 31 17:10:08 CET 2002
 */


#include <global.h>

#if !ROMIZING
extern void Java_java_lang_Object_getClass(void);
extern void Java_java_lang_Object_hashCode(void);
extern void Java_java_lang_Object_notify(void);
extern void Java_java_lang_Object_notifyAll(void);
extern void Java_java_lang_Object_wait(void);
extern void Java_java_lang_System_currentTimeMillis(void);
extern void Java_java_lang_System_arraycopy(void);
extern void Java_java_lang_System_identityHashCode(void);
extern void Java_java_lang_System_getProperty0(void);
extern void Java_java_lang_Class_forName(void);
extern void Java_java_lang_Class_newInstance(void);
extern void Java_java_lang_Class_isInstance(void);
extern void Java_java_lang_Class_isAssignableFrom(void);
extern void Java_java_lang_Class_isInterface(void);
extern void Java_java_lang_Class_isArray(void);
extern void Java_java_lang_Class_getName(void);
extern void Java_java_lang_Thread_currentThread(void);
extern void Java_java_lang_Thread_yield(void);
extern void Java_java_lang_Thread_sleep(void);
extern void Java_java_lang_Thread_start(void);
extern void Java_java_lang_Thread_isAlive(void);
extern void Java_java_lang_Thread_activeCount(void);
extern void Java_java_lang_Thread_setPriority0(void);
extern void Java_com_sun_cldc_io_j2me_debug_PrivateOutputStream_putchar(void);
extern void Java_java_lang_Runtime_exitInternal(void);
extern void Java_java_lang_Runtime_freeMemory(void);
extern void Java_java_lang_Runtime_totalMemory(void);
extern void Java_java_lang_Runtime_gc(void);


const NativeImplementationType java_lang_Object_natives[] = {
    { "getClass",            Java_java_lang_Object_getClass},
    { "hashCode",            Java_java_lang_Object_hashCode},
    { "notify",              Java_java_lang_Object_notify},
    { "notifyAll",           Java_java_lang_Object_notifyAll},
    { "wait",                Java_java_lang_Object_wait},
    NATIVE_END_OF_LIST
};

const NativeImplementationType java_lang_System_natives[] = {
    { "currentTimeMillis",   Java_java_lang_System_currentTimeMillis},
    { "arraycopy",           Java_java_lang_System_arraycopy},
    { "identityHashCode",    Java_java_lang_System_identityHashCode},
    { "getProperty0",        Java_java_lang_System_getProperty0},
    NATIVE_END_OF_LIST
};

const NativeImplementationType java_lang_Class_natives[] = {
    { "forName",             Java_java_lang_Class_forName},
    { "newInstance",         Java_java_lang_Class_newInstance},
    { "isInstance",          Java_java_lang_Class_isInstance},
    { "isAssignableFrom",    Java_java_lang_Class_isAssignableFrom},
    { "isInterface",         Java_java_lang_Class_isInterface},
    { "isArray",             Java_java_lang_Class_isArray},
    { "getName",             Java_java_lang_Class_getName},
    NATIVE_END_OF_LIST
};

const NativeImplementationType java_lang_Thread_natives[] = {
    { "currentThread",       Java_java_lang_Thread_currentThread},
    { "yield",               Java_java_lang_Thread_yield},
    { "sleep",               Java_java_lang_Thread_sleep},
    { "start",               Java_java_lang_Thread_start},
    { "isAlive",             Java_java_lang_Thread_isAlive},
    { "activeCount",         Java_java_lang_Thread_activeCount},
    { "setPriority0",        Java_java_lang_Thread_setPriority0},
    NATIVE_END_OF_LIST
};

const NativeImplementationType com_sun_cldc_io_j2me_debug_PrivateOutputStream_natives[] = {
    { "putchar",             Java_com_sun_cldc_io_j2me_debug_PrivateOutputStream_putchar},
    NATIVE_END_OF_LIST
};

const NativeImplementationType java_lang_Runtime_natives[] = {
    { "exitInternal",        Java_java_lang_Runtime_exitInternal},
    { "freeMemory",          Java_java_lang_Runtime_freeMemory},
    { "totalMemory",         Java_java_lang_Runtime_totalMemory},
    { "gc",                  Java_java_lang_Runtime_gc},
    NATIVE_END_OF_LIST
};

const ClassNativeImplementationType nativeImplementations[] = {
    { "java/lang/Object",         java_lang_Object_natives },
    { "java/lang/System",         java_lang_System_natives },
    { "java/lang/Class",          java_lang_Class_natives },
    { "java/lang/Thread",         java_lang_Thread_natives },
    { "com/sun/cldc/io/j2me/debug/PrivateOutputStream", com_sun_cldc_io_j2me_debug_PrivateOutputStream_natives },
    { "java/lang/Runtime",        java_lang_Runtime_natives },
NATIVE_END_OF_LIST
};
#endif
