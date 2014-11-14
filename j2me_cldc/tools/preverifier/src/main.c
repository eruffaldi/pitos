/*
 * @(#)main.c	1.4 00/04/12
 *
 * Copyright 1997, 1998 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 * 
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

/*
 * Run the Java class verifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "sys_api.h"
#include "path_md.h"

int processedfile = 0;

char *extra_class_path = NULL;
bool_t check_on = FALSE;

extern void VerifyFile(register char *fn);

static void usage(char *progname);

static void recurse_dir(char *dirname, char *pkgname)
{
	struct dirent *ent;
	DIR *dir = opendir(dirname);
	if (dir == NULL) {
		fprintf(stderr, "Can't open dir %s\n", dirname);
		exit(1);
	}
	for (ent = readdir(dir); ent; ent = readdir(dir)) {
		struct stat stat_buf;
		char *name = ent->d_name;
		char buf[256];
		char pkgbuf[256];
		int len;

		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
			continue;
		}

		strcpy(pkgbuf, pkgname);
		if (pkgname[0] != 0) {
			strcat(pkgbuf, "/");
		}
		strcat(pkgbuf, name);
		
		strcpy(buf, dirname);
		strcat(buf, name);

		stat(buf, &stat_buf);

		if (stat_buf.st_mode & S_IFDIR) {
			strcat(buf, "/");
			recurse_dir(buf, pkgbuf);
			continue;
		}
		len = strlen(pkgbuf);
		if (len > 6 && strcmp(pkgbuf + len - 6, ".class") == 0) {
			pkgbuf[len - 6] = 0;
			VerifyFile(pkgbuf);
		}
	}
	closedir(dir);
}

static void doit(char *argname)
{
	char buf[256];
	struct stat stat_buf;
	int res = stat(argname, &stat_buf);
	strcpy(buf, argname);
	
	if ((res == 0) && (stat_buf.st_mode & S_IFDIR)) {
		int orig_len = strlen(argname);
		/* Append dir separator if it does not yet exist. */
		if (buf[orig_len - 1] != LOCAL_DIR_SEPARATOR &&
			buf[orig_len - 1] != DIR_SEPARATOR) {
			buf[orig_len] = DIR_SEPARATOR;
			buf[orig_len + 1] = 0;
		}
		extra_class_path = buf;
		recurse_dir(buf, "");
		extra_class_path = NULL;
	} else {
		char *p;
   		/* Convert all periods in the argname to slashes */
		for (p = buf; ((p = strchr(p, '.')) != 0); *p++ = '/');
		
		VerifyFile(buf);
	}
}

main(argc, argv)
    register char **argv;
{
    char *progname;
    char *argv0 = argv[0];

    if ((progname = strrchr(argv[0], LOCAL_DIR_SEPARATOR)) != 0) {
	progname++;
    } else {
	progname = argv[0];
    }

    while (--argc > 0)
	if ((++argv)[0][0] == '-') {
            if (strcmp(argv[0], "-verbose") == 0) {
                extern bool_t verbose;
                verbose = TRUE;
#ifdef DEBUG_VERIFIER
	    } else if (strcmp(argv[0], "-verify-verbose") == 0) {
		extern int verify_verbose;
		verify_verbose++;
#endif
            } else if (strcmp(argv[0], "-check") == 0) {
                extern bool_t check_on;
                check_on = TRUE;
            } else if (strcmp(argv[0], "-Xns") == 0) {
                extern bool_t stack_map_on;
                stack_map_on = FALSE;
            } else if (strcmp(argv[0], "-Xni") == 0) {
                extern bool_t inline_jsr_on;
                inline_jsr_on = FALSE;
            } else if (strcmp(argv[0], "-d") == 0) {
                extern char *output_dir;
                output_dir = strdup(argv[1]);
		argc--; argv++;
	    } else if (strcmp(argv[0], "-classpath") == 0) {
		if (argc > 1) {
		    char *buf = (char *)malloc(strlen(argv[1]) + 32);
		    sprintf(buf, "CLASSPATH=%s", argv[1]);
		    putenv(buf);
		    argc--; argv++;
		} else {
		    fprintf(stderr,
			 "-classpath requires class path specification\n");
		    usage(progname);
		    exit(1);
		}
	    } else {
		fprintf(stderr, "%s: Illegal option %s\n\n", progname, argv[0]);
		usage(progname);
		exit(1);
	    }
	} else if (argv[0][0] == '@') {
	    int new_argv_size = 2;
	    char **new_argv = (char **)malloc(sizeof(char *) * new_argv_size);
	    int new_argc = 1;
	    char buf[1024];
	    FILE *fp;

	    new_argv[0] = argv0;

	    fp = fopen(&argv[0][1], "r");
	    if (fp == NULL) {
		fprintf(stderr, "Can't open %s\n", &argv[0][1]);
		exit(1);
	    }


	    while (fscanf(fp, "%s", buf) == 1) {
		if (new_argc + 1 == new_argv_size) {
		    new_argv_size *= 2;
		    new_argv = (char **)realloc(new_argv, sizeof(char *) * new_argv_size);
		}
		new_argv[new_argc++] = strdup(buf);
	    }
	    fclose(fp);
	    main(new_argc, new_argv);
	    while (new_argc > 1) { /* don't delete shared argv[0] */
		free(new_argv[--new_argc]);
	    }
	    free(new_argv);
	    argc--; argv++;
	} else {
	    processedfile++;
	    doit(argv[0]);
	}
    if (processedfile == 0) {
	usage(progname);
	exit(1);
    }
    return 0;
}

static void usage(char *progname) {
    fprintf(stderr, "Usage: %s [options] classnames|dirnames ...\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "where options include:\n");
    fprintf(stderr, "   -classpath <directories separated by '%c'>\n", PATH_SEPARATOR);
    fprintf(stderr, "                  Directories in which to look for classes\n");
    fprintf(stderr, "   -d <directory> Directory in which output is written (default is ./output/)\n");
    fprintf(stderr, "   @<filename>    Read command line arguments from a text file\n");
    fprintf(stderr, "\n");
}
    

int
OpenCode(char *fn, char *sfn, char *dir, struct stat * st)
{
    long codefd;
#ifdef SOLARIS2
    if (fn == 0 || (codefd = open(fn, 0, 0644)) < 0
	    || fstat(codefd, st) < 0)
#endif
#ifdef WIN32
    if (fn == 0 || (codefd = open(fn, O_BINARY, 0644)) < 0
	    || fstat(codefd, st) < 0)
#endif
	return -2;
    return codefd;
}
