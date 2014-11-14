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
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Native function interface
 * FILE:      winGraphics.c
 * OVERVIEW:  This file allows the Windows version of the KVM
 *            to emulate a "Palm lookalike" graphical user interface
 *            on Windows.  This feature is provided simply as a 
 *            convenience for the user and to enable the debugging
 *            of the VM on a platform where good debugging tools
 *            are available.
 * AUTHOR:    Frank Yellin
 *            Ioi Lam and Rich Berlin converted to Windows
 *=======================================================================*/

/*** IMPORTANT: there is no current way to finalize the data that is   ***/
/*** allocated in this module.  This is antisocial (it's a system-wide ***/
/*** memory leak) and could cause the OS to misbehave.                 ***/

/*=========================================================================
 * Include files
 *=======================================================================*/

#define STRICT
#include <windows.h>

#include "../../VmUnix/h/PalmFont0.h"
#include "machine_md.h"

/*=========================================================================
 * Definitions
 *=======================================================================*/

static int windowSystemInitialized = FALSE;

extern int wantPalmSysKeys; /* true if Spotlet wants ALL Palm hard buttons */

/* This is very ugly.  We want some data structures out of nativeSpotlet.h.  
 * We can't include everything because of same name clashes with windows.h.  So
 * we just redefine the types that cause us problems to be something innocuous,
 * but the same size.
 */
typedef void *INSTANCE, *THREAD, *BYTEARRAY, *METHOD;
typedef int bool_t;
#include "nativeSpotlet.h"

#define NumberOf(x) (sizeof(x)/sizeof(x[0]))

/*=========================================================================
 * Functions
 *=======================================================================*/

void *
sysMapMemory(int memSize, int *pageSize) {
    SYSTEM_INFO sysInfo;
    void *memoryStart = VirtualAlloc(NULL, memSize, MEM_COMMIT, PAGE_READONLY);
    GetSystemInfo(&sysInfo);
    *pageSize = sysInfo.dwPageSize;
    return memoryStart;
}

int
sysProtectPage(void *ptr, int sz, int write) {
    long o;			/* old protection value */
    return VirtualProtect(ptr, sz, write ? PAGE_READWRITE : PAGE_READONLY, &o);
}

void
sysUnmapMemory(void *ptr, int memSize) {
    VirtualFree(ptr, memSize, MEM_RELEASE);
}


typedef struct _Rectangle {
    int x;
    int y;
    int width;
    int height;
} _Rectangle;
#define Inside(x, y, _r) \
((x >= _r.x) && (x < (_r.x + _r.width)) && (y >= _r.y) && (y < (_r.y + _r.height)))

static LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

#define RefreshScreen() InvalidateRect(hMainWindow, &paintRect, TRUE)
#define RefreshArea(x,y,w,h)              \
do {                                      \
  RECT r;                                 \
  r.left = x; r.right  = x + w + 1;       \
  r.top  = y; r.bottom = y + h + 1;       \
  InvalidateRect(hMainWindow, &r, TRUE);  \
} while (0)

/*----------------------------------------------------------------------*/
/* Palm-specific stuff                                                  */
#define WIDTH 224
#define HEIGHT 340

#define PAINT_WIDTH  160
#define PAINT_HEIGHT 160

typedef enum {
    PALM_power    = 0x01, 
    PALM_hard1    = 0x204,
    PALM_hard2    = 0x205,
    PALM_hard3    = 0x206,
    PALM_hard4    = 0x207,
    PALM_pageUp   = 0x0B,
    PALM_pageDown = 0x0C
} PalmKeyType;

typedef struct  { 
    PalmKeyType button;
    _Rectangle bounds;
} Key;

#define MODE_PLAIN  1
#define MODE_GRAY   2
#define MODE_ERASE  3
#define MODE_INVERT 4

const static Key PalmKeys[] = {
    {PALM_power,    {0, 281, 16, 22}},
    {PALM_pageUp,   {100, 280, 25, 12}},
    {PALM_pageDown, {100, 302, 25, 12}},
    {PALM_hard1,    {24, 277, 28, 29}},
    {PALM_hard2,    {63, 277, 28, 29}},
    {PALM_hard3,    {134, 277, 28, 29}},
    {PALM_hard4,    {173, 277, 28, 29}}
};

const static short ArrowKeyMap[] = {
    PALM_hard2,    /* 37, left arrow */
    PALM_pageUp,   /* 38, up arrow */
    PALM_hard3,    /* 39, right arrow */
    PALM_pageDown, /* 40, down arrow */
};

/* Palm-specific stuff                                                  */
/*----------------------------------------------------------------------*/

static HBITMAP       hScreenBitmap;
static HBITMAP		 hOffScreenBitmap;
static HBITMAP       hFontBitmap;
static HWND          hMainWindow;
static RECT          paintRect;
static HRGN          clipRegion = NULL;
static KVMEventType *kvmEvent;
static int           gotEvent;
static int           x_offset = (WIDTH - PAINT_WIDTH)/2 + 2;
static int           y_offset = 33;
static HBRUSH        hGrayBrush;
static HBITMAP       hGrayBits;
static HBRUSH        oldBrush;
static HPEN          oldPen;

static HDC 
GetBitmapDC(int mode) 
{
    HDC hdc, hdcMem;

    hdc = GetDC(hMainWindow);
    hdcMem = CreateCompatibleDC(hdc);

    /* REMIND: should probably select these out again */
    SelectObject(hdcMem, hScreenBitmap);
    SelectClipRgn(hdcMem, clipRegion);

    switch (mode) {
    case MODE_GRAY:
      oldBrush = SelectObject(hdcMem, hGrayBrush);
      oldPen = SelectObject(hdcMem, GetStockObject(NULL_PEN));
      break;

    case MODE_INVERT:
      SetROP2(hdcMem, R2_NOT);
      /* FALLTHROUGH*/

    case MODE_ERASE:
      oldBrush = SelectObject(hdcMem, GetStockObject(WHITE_BRUSH));
      oldPen = SelectObject(hdcMem, GetStockObject(WHITE_PEN));
      break;

    case MODE_PLAIN:
      oldBrush = SelectObject(hdcMem, GetStockObject(BLACK_BRUSH));
      oldPen = SelectObject(hdcMem, GetStockObject(BLACK_PEN));
      break;
    }      

    DeleteDC(hdc);
    return hdcMem;
}

static void
DeleteBitmapDC(HDC hdcMem)
{
    SetROP2(hdcMem, R2_COPYPEN);
    SelectObject(hdcMem, oldBrush);
    SelectObject(hdcMem, oldPen);
    DeleteDC(hdcMem);
}

static void
fillRectangle(HDC hdc, int x, int y, int w, int h, int color, int cornerDiam)
{
    HBRUSH brush = SelectObject(hdc, CreateSolidBrush(color)); 
    HPEN pen = SelectObject(hdc, GetStockObject(NULL_PEN));

    if (cornerDiam) {
		cornerDiam <<= 1; /* use diameter instead of radius */
	RoundRect(hdc, x, y, x + w, y + h, cornerDiam, cornerDiam);
    } else {
	Rectangle(hdc, x, y, x + w, y + h);
    }

    SelectObject(hdc, pen);
    DeleteObject(SelectObject(hdc, brush));
}

static void 
drawLine(HDC hdc, int x1, int y1, int x2, int y2, int color)
{
    HPEN   hpen  = SelectObject(hdc, CreatePen(PS_SOLID, 1, color));

    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc,   x2, y2);
    LineTo(hdc,   x2+1, y2+1);  // force drawing of last point

    DeleteObject(SelectObject(hdc, hpen));
}

static void
drawRectangle(HDC hdc, int x, int y, int w, int h, int color) 
{
    HBRUSH hbrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    HPEN   hpen   = SelectObject(hdc, CreatePen(PS_SOLID, 1, color));
    
    Rectangle(hdc, x, y, x + w, y + h);

    DeleteObject(SelectObject(hdc, hpen));
    SelectObject(hdc, hbrush);
}

static void
CreateScreenBitmap(HDC hdc) 
{
    int i;
    int black = RGB(0, 0, 0);
    int gray  = RGB(127, 127, 127);
    int white = RGB(255, 255, 255);
    HDC hdcMem = CreateCompatibleDC(hdc);

    hScreenBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    SelectObject(hdcMem, hScreenBitmap);

    fillRectangle(hdcMem, 0, 0, WIDTH, HEIGHT, RGB(127, 127, 127), 0);

    for (i = 0; i < NumberOf(PalmKeys); i++) { 
	int x = PalmKeys[i].bounds.x;
	int y = PalmKeys[i].bounds.y;
	int width = PalmKeys[i].bounds.width;
	int height = PalmKeys[i].bounds.height;
	/* 	if (i < 3)  */
	fillRectangle(hdcMem, x, y, width, height, RGB(0, 0, 0), i ? height : 8);
	fillRectangle(hdcMem, x+2, y+2, width-4, height-4, RGB(127,127,127),
		      i ? height : 8);
    }


    fillRectangle(hdcMem, x_offset - 2, y_offset - 2, 
		  PAINT_WIDTH + 4, PAINT_HEIGHT + 4, white, 4);

    /* Draw a box around the silked screen area */
    drawRectangle(hdcMem, 25, 197, WIDTH-50, 259-197, black);

    /* Draw a box around the graffiti area */
    drawRectangle(hdcMem, 61, 202, WIDTH-122, 254-202, black);

    DeleteDC(hdcMem);
}

static void
CreateOffScreenBitmap(HDC hdc) 
{
    int white = RGB(255, 255, 255);
    HDC hdcMem = CreateCompatibleDC(hdc);

    hOffScreenBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    SelectObject(hdcMem, hOffScreenBitmap);

    fillRectangle(hdcMem, 0, 0, WIDTH, HEIGHT, white, 0);

    DeleteDC(hdcMem);
}

static void 
CreateFontBitmap(HDC hdc) 
{
    int i, x, y, k;
    int black = RGB(0, 0, 0);
    int white = RGB(255, 255, 255);
    HPEN hpen;
    HDC hdcMem = CreateCompatibleDC(hdc);
    hFontBitmap = CreateCompatibleBitmap(hdc, 
					 PalmFont0_width, PalmFont0_height);
    SelectObject(hdcMem, hFontBitmap);

    fillRectangle(hdcMem, 0, 0, PalmFont0_width+1, PalmFont0_height+1, white, 0);
    hpen = CreatePen(PS_SOLID, 1, black);
    SelectObject(hdcMem, hpen);

    i = 0;
    for (y=0; y<PalmFont0_height; y++) {
	for (x=0; x<PalmFont0_width; x+=8) {
	    int bits = PalmFont0_bits[i++];
	    int mask = 0x1;

	    for (k=0; k<8; k++) {
		if (bits & mask) {
		    MoveToEx(hdcMem, x+k, y, NULL);
		    LineTo(hdcMem, x+k+1, y+1);
		}
		mask <<= 1;
	    }
	}
    }
    DeleteObject(hpen);
    DeleteDC(hdcMem);
}

static void
DrawBitmap(HDC hdc, HBITMAP hBitmap, int x, int y)
{
    BITMAP bm;
    HDC hdcMem;
    POINT ptSize, ptOrg;

    hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hBitmap);
    SetMapMode(hdcMem, GetMapMode(hdc));
    GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP(hdc, &ptSize, 1);

    ptOrg.x = 0;
    ptOrg.y = 0;

    DPtoLP(hdcMem, &ptOrg, 1);

    BitBlt(hdc, x, y, ptSize.x, ptSize.y,
	   hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);

    DeleteDC(hdcMem);
}

static void
DrawBitmap2(HDC hdc, HBITMAP hBitmap,
	    int srcx, int srcy, int srcw, int srch, 
	    int dstx, int dsty, int rop) 
{
    HDC hdcMem;

    hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hBitmap);

    BitBlt(hdc, dstx, dsty, srcw, srch, hdcMem, srcx, srcy, rop);

    DeleteDC(hdcMem);
}

static LRESULT CALLBACK 
WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    int x, y;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (iMsg) {
    case WM_CREATE :
	return 0 ;

    case WM_PAINT :
	hdc = BeginPaint(hwnd, &ps);
	DrawBitmap(hdc, hScreenBitmap, 0, 0);
	EndPaint(hwnd, &ps);
	return 0 ;

    case WM_DESTROY :
	PostQuitMessage (0) ;
	exit(0);

    case WM_CHAR:
	kvmEvent->type = keyDownKVMEvent;
	kvmEvent->chr  = wParam;
	gotEvent = TRUE;
	return 0;

    case WM_KEYDOWN:
	if (wParam >= 37 && wParam <= 40) {
		kvmEvent->chr = ArrowKeyMap[wParam - 37];
		kvmEvent->type = keyDownKVMEvent;
		gotEvent = TRUE;
		return 0;
	}
	break;

    case WM_LBUTTONDOWN:
	kvmEvent->type = penDownKVMEvent;
	goto button;

    case WM_LBUTTONUP:
	kvmEvent->type = penUpKVMEvent;
	goto button;

    case WM_MOUSEMOVE:
	if (! (wParam & MK_LBUTTON)) {
	    return 0;
	}
	kvmEvent->type = penMoveKVMEvent;

    button: {
        int i;
        int button;

	    x = LOWORD(lParam);
	    y = HIWORD(lParam);

        if (kvmEvent->type == penDownKVMEvent) {
            for (i = 0; i < NumberOf(PalmKeys); ++i) { 
                if (Inside(x, y, PalmKeys[i].bounds)) {
                    button = PalmKeys[i].button;
                    if (button == PALM_pageUp || button == PALM_pageDown
                            || wantPalmSysKeys) {
                        kvmEvent->type = keyDownKVMEvent;
                        kvmEvent->chr  = (short)button;
                        gotEvent = TRUE;
                        return 0;
                    }
                    if (button == PALM_power) {
                        kvmEvent->type = appStopKVMEvent;
                        gotEvent = TRUE;
                        return 0;
					}
				}
			}
	    }

	    x -= x_offset;
	    y -= y_offset;

	    if (x > 0 && x < PAINT_WIDTH && y > 0 && y < PAINT_HEIGHT) { 
			kvmEvent->screenX = x;
			kvmEvent->screenY = y;
			gotEvent = TRUE;
	    }
	}
	return 0;
    }

    return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}


/* public API */

void 
InitializeWindowSystem()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    static char szAppName[] = "Kauai";
    WNDCLASSEX  wndclass ;
    HWND hwnd;
    HDC hdc;
    static WORD graybits[] = {0xaaaa, 0x5555, 0xaaaa, 0x5555, 
			      0xaaaa, 0x5555, 0xaaaa, 0x5555};

    unsigned int width = WIDTH, height = HEIGHT;

    if (windowSystemInitialized) {
	return;
    }

    wndclass.cbSize        = sizeof (wndclass) ;
    wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = WndProc ;
    wndclass.cbClsExtra    = 0 ;
    wndclass.cbWndExtra    = 0 ;
    wndclass.hInstance     = hInstance ;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = szAppName ;
    wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION) ;

    RegisterClassEx (&wndclass) ;

    hwnd = CreateWindow (szAppName,      // window class name
			 "KVM/Palm Emulator",     // window caption
			 WS_OVERLAPPEDWINDOW,     // window style
			 CW_USEDEFAULT,           // initial x position
			 CW_USEDEFAULT,           // initial y position
			 width,                   // initial x size
			 height,                  // initial y size
			 NULL,                    // parent window handle
			 NULL,                    // window menu handle
			 hInstance,               // program instance handle
			 NULL) ;		         // creation parameters

    hMainWindow = hwnd;

    /* rectangle that contains the "display" */
    paintRect.left   = x_offset;
    paintRect.top    = y_offset;
    paintRect.right  = x_offset + PAINT_WIDTH;
    paintRect.bottom = y_offset + PAINT_HEIGHT;
    clipRegion = CreateRectRgnIndirect(&paintRect);

    hdc = GetDC(hwnd);
    CreateScreenBitmap(hdc);
	CreateOffScreenBitmap(hdc);
    CreateFontBitmap(hdc);

    hGrayBits  = CreateBitmap(8, 8, 1, 1, (LPVOID)graybits);
    hGrayBrush = CreatePatternBrush(hGrayBits);

    DeleteDC(hdc);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    windowSystemInitialized = TRUE;
    //    printf("Hello\n");
}

int 
winGetNextKVMEvent(KVMEventType *event, int forever)
{
    MSG msg ;

    kvmEvent = event;
    gotEvent = FALSE;

    if (forever) {
	while ((gotEvent == FALSE) && GetMessage(&msg, NULL, 0, 0)) {
	    if (msg.message == WM_QUIT) {
		break;
	    }

	    TranslateMessage (&msg) ;
	    DispatchMessage (&msg) ;
	}
    } else {
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	    if (msg.message == WM_QUIT) {
		break;
	    }

	    TranslateMessage (&msg) ;
	    DispatchMessage (&msg) ;
	}
    }

    return gotEvent;
}

void 
winResetClip() 
{
    DeleteObject(clipRegion);
    clipRegion = CreateRectRgnIndirect(&paintRect);
}

void 
winSetClip(int x, int y, int width, int height) 
{
    DeleteObject(clipRegion);
    x += x_offset;
    y += y_offset;
    clipRegion = CreateRectRgn(x, y, x + width - 1, y + height - 1);
}

void 
winDrawRectangle(int x, int y, int w, int h, int mode) 
{
    HDC hdcMem = GetBitmapDC(mode);

    x += x_offset;
    y += y_offset;

   if (mode == MODE_GRAY) {
        Rectangle(hdcMem, x,     y,     x + w + 1, y + 2);
	Rectangle(hdcMem, x,     y + 1, x + 2,     y + h + 1);
	Rectangle(hdcMem, x + w, y + 1, x + w + 2, y + h + 1);
	Rectangle(hdcMem, x,     y + h, x + w + 2, y + h + 2);
    } else {
        HBRUSH hbrush = SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
	Rectangle(hdcMem, x, y, x + w + 1, y + h + 1);
	SelectObject(hdcMem, hbrush);
    }
    DeleteBitmapDC(hdcMem);
    RefreshArea(x, y, w, h);
}

void winFillRectangle(int x, int y, int w, int h, 
		      int cornerDiam, int mode) {
    HDC hdcMem = GetBitmapDC(mode);
    HPEN pen = SelectObject(hdcMem, GetStockObject(NULL_PEN));
    
    x += x_offset;
    y += y_offset;

    if (cornerDiam) {
		cornerDiam <<= 1; /* use diameter instead of radius */
	RoundRect(hdcMem, x, y, x + w + 1, y + h + 1, cornerDiam, cornerDiam);
    } else {
	Rectangle(hdcMem, x, y, x + w + 1, y + h + 1);
    }

    SelectObject(hdcMem, pen);
    DeleteBitmapDC(hdcMem);
    RefreshArea(x, y, w, h);
}

/*
 * This is a Bresenham line draw from (startX, startY) to (endX, endY).
 * THIS CODE MAY FAIL ON PERFECTLY VERTICAL OR PERFECTLY HORIZONTAL LINES.  
 */
static void
MidpointLine(HDC hdc, int startX, int startY, int endX, int endY)
{
    int dx   = endX - startX;
    int dy   = endY - startY;
    int adx  = (dx < 0) ? -dx : dx;
    int ady  = (dy < 0) ? -dy : dy;
    int dmin = (adx < ady) ? adx : ady;
    int dmax = adx + ady - dmin;

    int axialStep    = dmin<<1;
    int diagonalStep = dmax<<1;
    int errorTerm    = dmax;

    register int x  = startX;
    register int y  = startY;
    register int lx = (dx < 0) ? -1 : 1;
    register int ly = (dy < 0) ? -1 : 1;
    register int numSteps = dmax;

    if (adx >= ady) {
        for (;;) {
	    Rectangle(hdc, x, y, x + 2, y + 2);
	    if (--numSteps <= 0) break;
	    x += lx;
	    if ((errorTerm -= axialStep) <= 0) {
	      y += ly;
	      errorTerm += diagonalStep;
	    }
	}
    } else {
        for (;;) {
	    Rectangle(hdc, x, y, x + 2, y + 2);
	    if (--numSteps <= 0) break;
	    y += ly;
	    if ((errorTerm -= axialStep) <= 0) {
	      x += lx;
	      errorTerm += diagonalStep;
	    }
	}
    }
    Rectangle(hdc, endX, endY, endX + 2, endY + 2);
}

void 
winDrawLine(int x1, int y1, int x2, int y2, int mode) 
{
    //    printf("drawline: %d,%d %d,%d [mode=%d]\n", x1, y1, x2, y2, mode);
    HDC hdcMem = GetBitmapDC(mode);

    x1 += x_offset;
    y1 += y_offset;
    x2 += x_offset;
    y2 += y_offset;

    if (mode == MODE_GRAY) {
        if (x1 == x2) {
	    int start = (y1 > y2) ? y2 : y1;
	    int end   = y1 + y2 - start;
	    Rectangle(hdcMem, x1, start, x1 + 2, end + 1);
	} else if (y1 == y2) {
	    int start = (x1 > x2) ? x2 : x1;
	    int end   = x1 + x2 - start;
	    Rectangle(hdcMem, start, y1, end + 1, y1 + 2);
	} else {
 	    /* because pattern is not used for lines, we need Bresenham */
	    MidpointLine(hdcMem, x1, y1, x2, y2);
	}
    } else {
        MoveToEx(hdcMem, x1, y1, NULL);
        LineTo(hdcMem,   x2, y2);
        LineTo(hdcMem,   x2+1, y2+1);	// force drawing of last point
    }
    DeleteBitmapDC(hdcMem);

    /* find the rectangle that was touched */
    {
	if (x2 < x1) {
	    int t = x1;
	    x1 = x2;
	    x2 = t;
	}

	if (y2 < y1) {
	    int t = y1;
	    y1 = y2;
	    y2 = t;
	}

	RefreshArea(x1, y1, x2 - x1, y2 - y1);
    }
}

void 
winDrawChar(int c, int x, int y, int mode)
{
    HDC hdcMem = GetBitmapDC(mode);
    int start = PalmFont0_widths[(int)c];
    int end = PalmFont0_widths[(int)(c + 1)];

    int rop = (mode == MODE_PLAIN)  ? SRCCOPY
            : (mode == MODE_ERASE)  ? MERGEPAINT
            : (mode == MODE_INVERT) ? NOTSRCCOPY
            : 0xb8074a;

    x += x_offset;
    y += y_offset;

    if (end > start) {
	    DrawBitmap2(hdcMem, hFontBitmap,
		    start, 0,
		    end-start, PalmFont0_height,
		    x, y, rop);
    }
    DeleteBitmapDC(hdcMem);
}

int 
winCharWidth(int c)
{
    int start = PalmFont0_widths[(int)c];
    int end = PalmFont0_widths[(int)(c + 1)];

    if (end > start) {
	return (end - start);
    } else {
	return 0;
    }
}

void 
winRefreshScreen(int x, int y, int w, int h) 
{
    RefreshArea(x + x_offset, y + y_offset, w, h);
}


static int rasterOp[] = {
    SRCCOPY, SRCAND, SRCERASE, SRCINVERT, SRCPAINT, NOTSRCCOPY
};

void 
winCopyRegion(int x, int y, int width, int height,
	      int dstX, int dstY, int mode) 
{
    HDC hdcMem = GetBitmapDC(mode);
    int rop = ((mode >= 0) && (mode < NumberOf(rasterOp))) 
	    ? rasterOp[mode] : SRCCOPY;

    x    += x_offset;
    y    += y_offset;
    dstX += x_offset;
    dstY += y_offset;

    BitBlt(hdcMem, dstX, dstY, width, height, hdcMem, x, y, rop);
    
    DeleteBitmapDC(hdcMem);
    RefreshArea(dstX, dstY, width, height);
}

#define ONSCREEN_WINDOW   0
#define OFFSCREEN_WINDOW  1
	
void 
winCopyRectangle(int srcWin, int dstWin, int x, int y,
		  int width, int height, int dstX, int dstY, int mode) 
{
	HDC hdcSrc;
	HDC hdcDst;
	HDC hdcWindow = GetBitmapDC(mode);
    int rop = ((mode >= 0) && (mode < NumberOf(rasterOp))) 
	    ? rasterOp[mode] : SRCCOPY;

    HDC hdcOffScreen = CreateCompatibleDC(hdcWindow);
    SelectObject(hdcOffScreen, hOffScreenBitmap);
    hdcSrc = hdcOffScreen;  /* assume offScreenBitmap */
	hdcDst = hdcOffScreen;

	if (srcWin == ONSCREEN_WINDOW) {
		hdcSrc = hdcWindow;
	}
	if (dstWin == ONSCREEN_WINDOW) {
		hdcDst = hdcWindow;
	}

	x    += x_offset;
	y    += y_offset;
	dstX += x_offset;
	dstY += y_offset;

	BitBlt(hdcDst, dstX, dstY, width, height, hdcSrc, x, y, rop);
	if (rop == SRCINVERT) {
		/* must re-invert on Windows (because 0=black & 1 = white) */
		BitBlt(hdcDst, dstX, dstY, width, height, hdcSrc, x, y, DSTINVERT);
	}
    
    DeleteDC(hdcOffScreen);
    DeleteBitmapDC(hdcWindow);
	if (dstWin == ONSCREEN_WINDOW) {
		RefreshArea(dstX, dstY, width, height);
	}
}

void
winDrawMonoBitmap(int x, int y, int width, int height,
		  int rowBytes, char *bitmap) 
{
    HDC hdcMem = GetBitmapDC(MODE_PLAIN);
    BITMAP bmd;
    HBITMAP hbitmap;

    bmd.bmType       = 0;
    bmd.bmWidth      = width;
    bmd.bmHeight     = height;
    bmd.bmWidthBytes = rowBytes;
    bmd.bmPlanes     = 1;
    bmd.bmBitsPixel  = 1;
    bmd.bmBits		 = bitmap;
    hbitmap = CreateBitmapIndirect(&bmd);

    x += x_offset;
    y += y_offset;

    DrawBitmap(hdcMem, hbitmap, x, y);
    
    DeleteObject(hbitmap);
    DeleteBitmapDC(hdcMem);
    RefreshArea(x, y, width, height);
}
