/*
 * @(#)jamHttp.c	1.5 00/04/25
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

/*=========================================================================
 * JAM
 *=========================================================================
 * SYSTEM:    JAM
 * SUBSYSTEM: HTTP downloading.
 * FILE:      http.c
 * OVERVIEW:  Downloads web pages and JAR files for the browser and the
 *            Java Application Manager.
 * AUTHOR:    Ioi Lam
 *            KVM integration by Todd Kennedy and Sheng Liang
 *=======================================================================*/

#ifndef WIN32

#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <global.h>

typedef int SOCKET;

#define IS_INVALID(sock) ((sock) < 0)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close

#else

#include <windows.h>
typedef int bool_t;
#include <machine_md.h>
#include <main.h>
#include <long.h>

#define IS_INVALID(sock) ((sock) == INVALID_SOCKET)
#define WSA_VERSION_REQD MAKEWORD(1,1)

#endif

#include <sys/stat.h>
#include <fcntl.h>

#include "jam.h"

#ifdef WIN32

static char * wsaErrorTable[] = {
    "WSAEWOULDBLOCK",
    "WSAEINPROGRESS",
    "WSAEALREADY",
    "WSAENOTSOCK",
    "WSAEDESTADDRREQ",
    "WSAEMSGSIZE",
    "WSAEPROTOTYPE",
    "WSAENOPROTOOPT",
    "WSAEPROTONOSUPPORT",
    "WSAESOCKTNOSUPPORT",
    "WSAEOPNOTSUPP",
    "WSAEPFNOSUPPORT",
    "WSAEAFNOSUPPORT",
    "WSAEADDRINUSE",
    "WSAEADDRNOTAVAIL",
    "WSAENETDOWN",
    "WSAENETUNREACH",
    "WSAENETRESET",
    "WSAECONNABORTED",
    "WSAECONNRESET",
    "WSAENOBUFS",
    "WSAEISCONN",
    "WSAENOTCONN",
    "WSAESHUTDOWN",
    "WSAETOOMANYREFS",
    "WSAETIMEDOUT",
    "WSAECONNREFUSED",
    "WSAELOOP",
    "WSAENAMETOOLONG",
    "WSAEHOSTDOWN",
    "WSAEHOSTUNREACH",
    "WSAENOTEMPTY",
    "WSAEPROCLIM",
    "WSAEUSERS",
    "WSAEDQUOT",
    "WSAESTALE",
    "WSAEREMOTE",
};


static void reportSocketStatus() {
    int errCode = WSAGetLastError();

    if ((errCode >= WSAEWOULDBLOCK) && (errCode <= WSAEREMOTE)) {
        fprintf(stderr, "WSA last error = %d: %s\n", errCode,
                wsaErrorTable[errCode - WSAEWOULDBLOCK]);
    }
}

#else
static void reportSocketStatus() {

}
#endif


static int hasEOF = 0;

/*
 * If fake_mime is defined to be TRUE, then we accept all URL's with a
 * .jam prefix as having a mime type of application/x-jam This can be
 * overridden by the environment variable FAKE_MIME. For example
 *
 *      setenv FAKE_MIME 0
 */

static int fake_mime = 1;

#define MAX_BUF 1024
#define MAX_URL 256

static int parseURL(char * url, char *host, int *port, char *path);

int expandURL(char* oldURL, char* newURL) {
    char host[MAX_URL];
    char path[MAX_URL];
    int port;
    int rc;

    if ((rc=parseURL(oldURL, host, &port, path)) != 0) {
        sprintf(newURL, "http://%s:%d%s", host, port, path);
    } else {
        newURL[0] = '\0';
    }
    return rc;
}

static int parseURL(char * url, char *host, int *port, char *path) {
    char * start, * p;

    if (strncmp(url, "http://", 7) != 0) {
      /*        fprintf(stderr, "bad protocol, must be http://\n");*/
        return 0;
    }

    /*
     * Get host name.
     */
    start = p = url + 7;
    while (*p && (isalnum((int)*p) || *p == '.' || *p == '-'|| *p == '_')) {
        p++;
    }
    if (p - start >= MAX_BUF) {
        fprintf(stderr, "bad host name: too long\n");
        return 0;
    }
    strnzcpy(host, start, p-start);


    /*
     * Get port number
     */
    if (*p == ':') {
        char num[10];
        p++;
        start = p;
        while (isdigit((int)*p)) {
            p ++;
        }
        if ((start - p) > 5) {
            fprintf(stderr, "bad port number\n");
            return 0;
        }
        strnzcpy(num, start, p-start);
        *port = atoi(num);
        if (*port <= 0 || *port >= 65535) {            
            fprintf(stderr, "bad port number %d\n", *port);
            return 0;
        }
    } else {
        *port = 80;
    }

    /*
     * Get path
     */

    if (*p == 0) {
        strcpy(path, "/");
    }
    else if (*p != '/') {
        fprintf(stderr, "bad path: must start with \"/\"\n");
    }
    else if (strlen(p) >= MAX_BUF) {
        fprintf(stderr, "bad path: too long\n");
    }
    else {
        strcpy(path, p);
    }

    return 1;
}


static int
CreateSocketAddress(struct sockaddr_in *sockaddrPtr, char *host, int port) {
    struct hostent *hostent;		/* Host database entry */
    struct in_addr addr;		/* For 64/32 bit madness */

    memset(sockaddrPtr, '\0', sizeof(struct sockaddr_in));
    sockaddrPtr->sin_family = AF_INET;
    sockaddrPtr->sin_port = htons((unsigned short) (port & 0xFFFF));

    if (host == NULL) {
	addr.s_addr = INADDR_ANY;
    } else {
        addr.s_addr = inet_addr(host);
        if (addr.s_addr == -1) {
            hostent = gethostbyname(host);
            if (hostent != NULL) {
                memcpy(&addr, hostent->h_addr_list[0],
                       hostent->h_length);
            } else {
                return 0;	/* error */
            }
        }
    }

    sockaddrPtr->sin_addr.s_addr = addr.s_addr; 
    return 1;	/* Success. */
}

static SOCKET openHTTP(char *host, int port, char *path) {
    struct sockaddr_in sockaddr;	/* Socket address */
    SOCKET sock = INVALID_SOCKET;

    hasEOF = 0;
    if (!CreateSocketAddress(&sockaddr, host, port)) {
        fprintf(stderr, "bad host:port -- %s:%d\n", host, port);
        return INVALID_SOCKET;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (IS_INVALID(sock)) {
        fprintf(stderr, "Cannot create client socket: socket()\n");
	goto error;
    }

    if (connect(sock, (struct sockaddr *) &sockaddr,
                sizeof(sockaddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Cannot create client socket: connect()\n");
	goto error;
    }
    return sock;

error:
    if (!IS_INVALID(sock)) {
        closesocket(sock);
    }
    return INVALID_SOCKET;
}

static void sendData(SOCKET sock, char * data) {
    char * p = data;
    int len = strlen(data);
    int delta;

    while (len > 0) {
        delta = send(sock, p, len, 0);
        if (delta < 0) {
            return;
        }
        len -= delta;
        p += delta;
    }
}

static int getByte(SOCKET sock) {
    char c;
    int n;

    while (1) {
        n = recv(sock, &c, 1, 0);
        if (n <= 0 || n > 1) {
            hasEOF = 1;
            return 0;
        }
        if (n == 1) {
            return c;
        }
    }
}

static int getBuffer(SOCKET sock, char **buf, int *buf_len, int getline,
                     int *dataLen_ret) {
    int len = 0;
    int c;

    if (hasEOF) {
        return 0; /* ERROR */
    }

    while (1) {
        int needlen = len + 2;  /* Give some slack */

        if (needlen > *buf_len) {
            /* grow the buffer */
            int newlen = (*buf_len) * 2;
            char * newbuf = realloc(*buf, newlen);
            if (newbuf == NULL) {
                return 0;
            }
            *buf_len = newlen;
            *buf = newbuf;
        }

        c = getByte(sock);

        if (hasEOF) {
            (*buf)[len] = '\0';
            *dataLen_ret = len;
            return (len > 0);
        }
        if (getline && c == '\r') {
            /* Skip \r characters */
            continue;
        }
            

        if (getline && c == '\n') {
            (*buf)[len++] = c;
            (*buf)[len] = '\0';
            *dataLen_ret = len;
            return 1;
        } else {
            (*buf)[len] = c;
        }
        len ++;
    }
}

#define CODE 0
#define HEAD 1
#define BODY 2

/*
 * This function is a real implementation of HTTP GET for downloading
 * Web pages and JAR files into the phone emulator.
 *
 * The current implementation maintains an malloc'ed auto-growing buffer
 * to download all the data that comes from the HTTP server.
 */
static char *HTTPGet_prim(char *url, char **header, int *headerLength, char **content, int *contentLength, int *httpCode) {
    char hostname[MAX_BUF];
    int port;
    char path[MAX_BUF];
    SOCKET sock = INVALID_SOCKET;
    int len, state;
    char * buf = NULL;
    int buf_len;
    bool_t readBody = TRUE;

    if (!parseURL(url, hostname, &port, path)) {
        fprintf(stderr, "bad URL: %s\n", url);
        return NULL;
    }

    /*fprintf(stderr, "host=%s, port=%d, path=%s\n", hostname, port, path);*/

    /*
     * Send the request
     */
    if (IS_INVALID(sock = openHTTP(hostname, port, path))) {
        return NULL;
    }

    sendData(sock, "GET ");
    sendData(sock, path);
    sendData(sock, " HTTP/1.0\n\n");

    /*
     * Receive the data.
     */

    len = 0;
    state = CODE;
    buf_len = 100;
    if (!(buf = browser_malloc(100))) {
        goto error;
    }

    while (!hasEOF) {
        switch(state) {
        case CODE:
            if (!getBuffer(sock, &buf, &buf_len, 1, &len)) {
                *httpCode = 400;
                goto error;
            } else {
                sscanf(buf,"%*s %d %*s",httpCode);
                switch(*httpCode) {
                    case 200:                                          /* OK */
                        break;
                    case 404:          /* Resource not available - permanent */
                    case 503:          /* Resource not available - temporary */
                    default:
                        readBody = FALSE;
                        break;
                }
            }
            state = HEAD;
            break;

        case HEAD:
            /* TODO: parse Content-Length:*/
            if (!getBuffer(sock, &buf, &buf_len, 1, &len)) {
                fprintf(stderr, "error reading HTTP header\n");
                goto error;
            }
            else if ( (len == 1) && (buf[0] == '\n') ){
                /* got an empty line: body is coming */
                state = BODY;
            } else {
                if (*header == NULL ) {
                    *header = strdup(buf);
                    *headerLength = len + 1; /* Include trailing '\0' */
                } else {
                    /* Save header line */
                    char* newHead;
                    int   newHeadLen = *headerLength + len;
                    newHead = realloc(*header, newHeadLen);
                    if( newHead == NULL ) {
                        goto error;
                    }
                    strcat( newHead, buf );
                    *header = newHead;
                    *headerLength = newHeadLen;
                }
            }
            break;

        case BODY:
            if (readBody) {
                if (!getBuffer(sock, &buf, &buf_len, 0, &len)) {
                    goto error;
                }
            }

            *content = buf;
            *contentLength = len;
            closesocket(sock);
            return buf;
        }
    }


error:
    if (!IS_INVALID(sock)) {
        closesocket(sock);
    }
    browser_free(buf);

    reportSocketStatus();
    *content = NULL;
    return NULL;
}

static char *HTTP_GetField(char *header, char* field) {
    char* nextField;
    int   fieldLen;
    
    if( (field == NULL) || (header == NULL) ) {
        return NULL;
    }
    fieldLen = strlen(field);
    nextField = header;
    
    while (*nextField != '\0') {
        if (strncmp(nextField, field, fieldLen) == 0) {
            nextField += fieldLen;
            break;
        }
        
        nextField = strchr(nextField, '\n');
        if( nextField == NULL ) {
            break;
        }
        nextField++;
    }
    
    if ( (nextField != NULL) && (*nextField == '\0') ) {
        return NULL;
    }
    return nextField;
}

#define DEFAULT_RETRY_DELAY 2500

/* Implement HTTP policy for retransmissions, etc.. */
char *HTTPGet(char * url, int *contentType, int *contentLength, bool_t retryOnError) {
    int   httpCode;
    char *content;
    char *header;
    int   headerLen;
    char *field;
    int   retryAfter;

    do {
        header = NULL;
        content = NULL;
        *contentType = CONTENT_HTML;
        httpCode = ((retryOnError==TRUE)?503:404);

        HTTPGet_prim(url, &header, &headerLen, &content, contentLength, &httpCode);
	if (fake_mime) {
	    int i;
            /*
             * Treat any URL ending with ".jam" as a JAM file.
             */
	    i = strlen(url);
	    if (i > 4 &&
		url[i-4]=='.' && url[i-3]=='j' && 
		url[i-2]=='a' && url[i-1]=='m'){
		*contentType = CONTENT_JAVA_MANIFEST;
	    }
        }
        
        if ( (field = HTTP_GetField(header,
					   "Content-Type: ")) != NULL ) {
            if(strncmp(field, "application/x-jam", 17) == 0) {
                *contentType = CONTENT_JAVA_MANIFEST;
            }
        }

        if (httpCode != 503) {
            browser_free(header);
            break;
        }

        /* Use Retry-After: field, if it exists */
        if ( (field = HTTP_GetField(header, "Retry-After: ")) != NULL ) {
            sscanf(field, "%d", &retryAfter);
            retryAfter *= 1000;                         /* Convert to millis */
            if( retryAfter < 0 ) {
                retryAfter = DEFAULT_RETRY_DELAY;
            }
        } else {
            retryAfter = DEFAULT_RETRY_DELAY;
        }
        browser_free(header);

        if( retryOnError ) {
          SLEEP_MILLIS(retryAfter);
          continue;
        }
    } while (retryOnError);

    return content;
}

void HTTP_Initialize() {
    char * p;

#ifdef WIN32
    WSADATA wsaData;

    if (WSAStartup(WSA_VERSION_REQD, &wsaData) != 0) {
        fprintf(stderr, "Cannot initialize WSA\n");
        exit(0);
    }
    if (wsaData.wVersion != WSA_VERSION_REQD) {
        fprintf(stderr, "Wrong version of WSA\n");
        WSACleanup();
        exit(0);
    }
#endif

    if ((p = getenv("FAKE_MIME")) != NULL) {
        if (fake_mime) {
            if (*p == '0') {
                fake_mime = 0;
            }
        } else {
            if (*p == '1') {
                fake_mime = 1;
            }
        }
    }
    /*
    fprintf(stderr, "HTTP setting:\n");
    fprintf(stderr, "\tFAKE_MIME    = %d\n", fake_mime);
    */
}
