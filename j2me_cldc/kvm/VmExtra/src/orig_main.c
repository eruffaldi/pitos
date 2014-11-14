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
 * Spotless Virtual Machine
 *=========================================================================
 * SYSTEM:    Spotless VM
 * SUBSYSTEM: Main program
 * FILE:      main.c
 * OVERVIEW:  Main program & system initialization
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            JAM integration by Sheng Liang
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

#if USE_JAM
#include "jam.h"
#endif

int
main (int argc, char* argv[]) {
    int result;

#if USE_JAM
    char *jamInstalledAppsDir = "./instapps";
#endif

    Verbose_On = FALSE;
    Debugging_On = FALSE;
    JamEnabled = FALSE;
    JamRepeat = FALSE;
    
    while (argc > 1) {
        if (strcmp(argv[1], "-verbose") == 0) {
            Verbose_On = TRUE;
            argv++; argc--;
        } else if (strcmp(argv[1], "-debug") == 0) {
            Debugging_On = TRUE;
            argv++; argc--;
        } else if ((strcmp(argv[1], "-classpath") == 0) && argc > 2) {
	    if (JamEnabled) {
		fprintf(stderr, "Can't combine -classpath with -jam options");
		exit(1);
	    }
            UserClassPath = argv[2];
            argv+=2; argc -=2;
#if USE_JAM
	} else if (strcmp(argv[1], "-jam") == 0) {
	    JamEnabled = TRUE;
	    argv++; argc--;
	} else if (JamEnabled && (strcmp(argv[1], "-repeat") == 0)) {
	    JamRepeat = TRUE;
	    argv++; argc--;
	} else if (JamEnabled && (strcmp(argv[1], "-appsdir") == 0)
		   && argc > 2) {
	    jamInstalledAppsDir = argv[2];
	    argv+=2; argc-=2;
#endif /* USE_JAM */
        } else {
            break;
        }
    }

    /* Skip program name */
    argc--;
    argv++;

    if (UserClassPath == NULL && !JamEnabled) { 
        UserClassPath = getenv("classpath");
        if (UserClassPath == NULL) { 
            /*  Just in case environment variable reading is case sensitive */
            UserClassPath = getenv("CLASSPATH");
            if (UserClassPath == NULL) { 
                fprintf(stdout, "Classpath is not set. Defaulting to \".\"\n");
                UserClassPath = ".";
            }
        }
    }
#if USE_JAM
    if (JamEnabled) {
	if (argc != 1 || 
	    ((strncmp(argv[0], "http://", 7) != 0) &&
	     (strncmp(argv[0], "file://", 7) != 0))) {
	    fprintf(stderr, 
		    "expecting an \"http://\" or \"file://\" URL "
		    "following the -jam option\n");
	    exit(1);
	}
        JamInitialize(jamInstalledAppsDir);
	do {
	    result = JamRunURL(argv[0], JamRepeat);
	    if (result == JAM_RETURN_ERR) {
		break;
	    }
	} while(JamRepeat);
    } else
#endif /* USE_JAM */
    {
	result = StartJVM(argc, argv);
    }
    return result;
}
