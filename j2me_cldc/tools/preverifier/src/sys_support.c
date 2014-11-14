/*
 * @(#)sys_support.c	1.1 99/12/23
 *
 * Copyright 1995-1999 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 * 
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "typedefs.h"
#include "sys_api.h"
#include "path.h"

cpe_t **
sysGetClassPath(void)
{
    static cpe_t **classpath;

    if (classpath == 0) {
		char *cps, *s;
		int ncpe = 1;
		cpe_t **cpp;
		if ((cps = getenv("CLASSPATH")) == 0) {
			cps = ".";
		}
		if ((cps = strdup(cps)) == 0) {
			return 0;
		}
		for (s = cps; *s != '\0'; s++) {
			if (*s == PATH_SEPARATOR) {
				ncpe++;
			}
		}
		cpp = classpath = sysMalloc((ncpe + 1) * sizeof(cpe_t *));
		if (cpp == 0) {
			return 0;
		}
		while (cps && *cps) {
			char *path = cps;
			cpe_t *cpe;
			if ((cps = strchr(cps, PATH_SEPARATOR)) != 0) {
				*cps++ = '\0';
			}
			if (*path == '\0') {
				path = ".";
			}
			cpe = sysMalloc(sizeof(cpe_t));
			if (cpe == 0) {
				return 0;
			}
			cpe->type = CPE_DIR;
			cpe->u.dir = path;
			*cpp++ = cpe;
		}
		*cpp = 0;
    }
    return classpath;
}

#ifdef WIN32

#undef DEBUG_PATH		/* Define this to debug path code */

#define isfilesep(c) ((c) == '/' || (c) == '\\')
#define islb(c)      (IsDBCSLeadByte((BYTE)(c)))


/* Convert a pathname to native format.  On win32, this involves forcing all
   separators to be '\\' rather than '/' (both are legal inputs, but Win95
   sometimes rejects '/') and removing redundant separators.  The input path is
   assumed to have been converted into the character encoding used by the local
   system.  Because this might be a double-byte encoding, care is taken to
   treat double-byte lead characters correctly. */

char *
sysNativePath(char *path)
{
    char *src = path, *dst = path;
    char *colon = NULL;		/* If a drive specifier is found, this will
				   point to the colon following the drive
				   letter */

    /* Assumption: '/', '\\', ':', and drive letters are never lead bytes */
    sysAssert(!islb('/') && !islb('\\') && !islb(':'));

    /* Check for leading separators */
    while (isfilesep(*src)) src++;
    if (isalpha(*src) && !islb(*src) && src[1] == ':') {
	/* Remove leading separators if followed by drive specifier.  This
	   hack is necessary to support file URLs containing drive
	   specifiers (e.g., "file://c:/path").  As a side effect,
	   "/c:/path" can be used as an alternative to "c:/path". */
	*dst++ = *src++;
	colon = dst;
	*dst++ = ':'; src++;
    } else {
	src = path;
	if (isfilesep(src[0]) && isfilesep(src[1])) {
	    /* UNC pathname: Retain first separator; leave src pointed at
	       second separator so that further separators will be collapsed
	       into the second separator.  The result will be a pathname
	       beginning with "\\\\" followed (most likely) by a host name. */
	    src = dst = path + 1;
	    path[0] = '\\';	/* Force first separator to '\\' */
	}
    }

    /* Remove redundant separators from remainder of path, forcing all
       separators to be '\\' rather than '/' */
    while (*src != '\0') {
	if (isfilesep(*src)) {
	    *dst++ = '\\'; src++;
	    while (isfilesep(*src)) src++;
	    if (*src == '\0') {	/* Check for trailing separator */
		if (colon == dst - 2) break;                      /* "z:\\" */
		if (dst == path + 1) break;                       /* "\\" */
		if (dst == path + 2 && isfilesep(path[0])) {
		    /* "\\\\" is not collapsed to "\\" because "\\\\" marks the
		       beginning of a UNC pathname.  Even though it is not, by
		       itself, a valid UNC pathname, we leave it as is in order
		       to be consistent with sysCanonicalPath() (below) as well
		       as the win32 APIs, which treat this case as an invalid
		       UNC pathname rather than as an alias for the root
		       directory of the current drive. */
		    break;
		}
		dst--;		/* Path does not denote a root directory, so
				   remove trailing separator */
		break;
	    }
	} else {
	    if (islb(*src)) {	/* Copy a double-byte character */
		*dst++ = *src++;
		if (*src) {
		    *dst++ = *src++;
		}
	    } else {		/* Copy a single-byte character */
		*dst++ = *src++;
	    }
	}
    }

    *dst = '\0';
#ifdef DEBUG_PATH
    jio_fprintf(stderr, "sysNativePath: %s\n", path);
#endif DEBUG_PATH
    return path;
}

DIR *
opendir(const char *dirarg)
{
    DIR *dirp = (DIR *)sysMalloc(sizeof(DIR));
    unsigned long fattr;
    char alt_dirname[4] = { 0, 0, 0, 0 };
	char dirname_buf[1024];
	char *dirname = dirname_buf;

    if (dirp == 0) {
	errno = ENOMEM;
	return 0;
    }

	strcpy(dirname, dirarg);
	sysNativePath(dirname);

    /*
     * Win32 accepts "\" in its POSIX stat(), but refuses to treat it
     * as a directory in FindFirstFile().  We detect this case here and
     * prepend the current drive name.
     */
    if (dirname[1] == '\0' && dirname[0] == '\\') {
	alt_dirname[0] = _getdrive() + 'A' - 1;
	alt_dirname[1] = ':';
	alt_dirname[2] = '\\';
	alt_dirname[3] = '\0';
	dirname = alt_dirname;
    }

    dirp->path = (char *)sysMalloc(strlen(dirname) + 5);
    if (dirp->path == 0) {
	sysFree(dirp);
	errno = ENOMEM;
	return 0;
    }
    strcpy(dirp->path, dirname);

    fattr = GetFileAttributes(dirp->path);
    if (fattr == 0xffffffff) {
	sysFree(dirp->path);
	sysFree(dirp);
	errno = ENOENT;
	return 0;
    } else if (fattr & FILE_ATTRIBUTE_DIRECTORY == 0) {
	sysFree(dirp->path);
	sysFree(dirp);
	errno = ENOTDIR;
	return 0;
    }

    /* Append "*.*", or possibly "\\*.*", to path */
    if (dirp->path[1] == ':'
	&& (dirp->path[2] == '\0'
	    || (dirp->path[2] == '\\' && dirp->path[3] == '\0'))) {
	/* No '\\' needed for cases like "Z:" or "Z:\" */
	strcat(dirp->path, "*.*");
    } else {
	strcat(dirp->path, "\\*.*");
    }

    dirp->handle = FindFirstFile(dirp->path, &dirp->find_data);
    if (dirp->handle == INVALID_HANDLE_VALUE) {
        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
	    sysFree(dirp->path);
	    sysFree(dirp);
	    errno = EACCES;
	    return 0;
	}
    }
    return dirp;
}

struct dirent *
readdir(DIR *dirp)
{
    if (dirp->handle == INVALID_HANDLE_VALUE) {
	return 0;
    }

    strcpy(dirp->dirent.d_name, dirp->find_data.cFileName);

    if (!FindNextFile(dirp->handle, &dirp->find_data)) {
	if (GetLastError() == ERROR_INVALID_HANDLE) {
	    errno = EBADF;
	    return 0;
	}
	FindClose(dirp->handle);
	dirp->handle = INVALID_HANDLE_VALUE;
    }

    return &dirp->dirent;
}

int
closedir(DIR *dirp)
{
    if (dirp->handle != INVALID_HANDLE_VALUE) {
	if (!FindClose(dirp->handle)) {
	    errno = EBADF;
	    return -1;
	}
	dirp->handle = INVALID_HANDLE_VALUE;
    }
    sysFree(dirp->path);
    sysFree(dirp);
    return 0;
}

#endif
