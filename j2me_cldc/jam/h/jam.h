/*
 * @(#)jam.h	1.3 00/04/25
 *
 * Copyright (c) 1999 Sun Microsystems, Inc. All Rights Reserved.
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

#ifndef _JAM_H_
#define _JAM_H_


/*=========================================================================
 * SYSTEM:    JAM
 * SUBSYSTEM: Java Application Manager.
 * FILE:      jam.h
 * OVERVIEW:  Declaration of internal functions used strictly within
 *            the JAM prototype implementation. None of these functions
 *            are public and should not be used by outside modules.
 *
 * AUTHOR:    Ioi Lam, Consumer & Embedded, Sun Microsystems, Inc.
 *            KVM integration by Todd Kennedy and Sheng Liang
 *=======================================================================*/

#define APPLICATION_NAME_TAG    "Application-Name"
#define APPLICATION_VERSION_TAG "Application-Version"
#define KVM_VERSION_TAG         "KVM-Version"
#define JAR_FILE_URL_TAG        "JAR-File-URL"
#define JAR_FILE_SIZE_TAG       "JAR-File-Size"
#define MAIN_CLASS_TAG          "Main-Class"
#define USE_ONCE_TAG       	"Use-Once"

typedef struct JamApp {
    struct JamApp *next;
    char * jarName;
    char * jarURL;
    char * appName;
    char * mainClass;
    char * version;
} JamApp;

/*
 * Main function called from KVM main
 */

int JamRunURL(char *url, bool_t retryOnError);

/*
 * Application management functions (platform independent)
 */

int JamGetAppCount(void);
JamApp * JamGetAppList(void);
JamApp * JamGetCurrentApp(void);
void JamFreeApp(JamApp *app);

int JamInvokeDescriptorFile(char * jamContent, char *parentURL,
                            char **nextURL_ret);
void JamInitialize(char *appsdir);

extern int JamRunApp(JamApp* app, char ** nextURL_ret);
extern char * JamGetProp(char *buffer, char * name, int *length);
extern void strnzcpy(char * dst, char *src, int len);
extern int JamCheckVersion(char * ver, int verLen);
extern int JamCompareVersion(char *ver1, int ver1Len, char * ver2, int  
                             ver2Len);

/*
 * Storage management functions (platform specific).
 */

void JamInitializeStorage(char * dir);
int JamGetFreeSpace(void);
int JamGetUsedSpace(void);
int JamGetTotalSpace(void);
int JamGetAppJarSize(JamApp *app);
int JamGetAppTotalSize(JamApp *app);

int JamOpenAppsDatabase(void);
JamApp * JamGetNextAppEntry(void);
void JamCloseAppsDatabase(void);
int JamSaveAppsDatabase(void);

int JamSaveJARFile(JamApp *app, char * jarContent, int len);
int JamDeleteJARFile(JamApp *app);

/* Internal functions -- will disappear */
char *JamGetAppsDir();
char *JamGetCurrentJar();
void JamInitializeUsedSpace(JamApp* app);

/* MISC : TODO move to somewhere else */
char * browser_ckalloc(int size);
#define browser_malloc malloc
#define browser_free  free

extern int JamDownloadErrorPage(int error);
extern int JamCheckSecurity(char * url1, char * url2);
extern int expandURL(char* oldURL, char* newURL);

int JamError(char *msg);

void JamFreeAppUsedSpace(JamApp *app);

/* HTTP functions */
void HTTP_Initialize();
char *HTTPGet(char * url, int *contentType, int *contentLength, bool_t retryOnError);

#define JAM_BAD_URL             1
#define JAM_INVALID_MANIFEST    2
#define JAM_MISC_ERROR          3

#define CONTENT_HTML          1
#define CONTENT_JAVA_MANIFEST 2

#define JAM_RETURN_OK       0
#define JAM_RETURN_ERR      -1023

/* Misc utility macros */

#define IS_SPACE(c) ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t')

#if 0
#define ASSERT(x) ((x) ? 1 : (*(int*)(0x0)))
#else
#define ASSERT(x)
#endif

#ifndef SLEEP_MILLIS
#if WINDOWS
#define SLEEP_MILLIS(x) (Sleep((x)))
#else
#define SLEEP_MILLIS(x) (usleep((x)*1000))
#endif
#endif

/* O_BINARY is only useful on Win32 */
#ifndef O_BINARY
#define O_BINARY 0
#endif

#endif /* _JAM_H_ */
