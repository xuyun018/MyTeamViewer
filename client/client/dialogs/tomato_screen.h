//---------------------------------------------------------------------------
// 内存块管理的基础类
//---------------------------------------------------------------------------
#ifndef TOMATO_SCREEN_H
#define TOMATO_SCREEN_H
//---------------------------------------------------------------------------
#include "../../../xyannounce.h"
#include "../../../tomato_coder.h"

#include "tomato_common.h"
//---------------------------------------------------------------------------
#define WM_TOMATO_UID														(WM_USER + 100)
// 远程主机连接过来
#define WM_TOMATO_LOGONED													(WM_USER + 101)
#define WM_TOMATO_DRAW														(WM_USER + 102)
//---------------------------------------------------------------------------
#define MAKEDWORD(h, l)        												(((unsigned long)h << 16) | l)
//---------------------------------------------------------------------------
// 包括了转发(4字节), 头(4字节)
#define SCREEN_HEADERSIZE													8
// 控制命令(4字节)
#define SCREEN_BUFFEROFFSET													(SCREEN_HEADERSIZE + 4)
//---------------------------------------------------------------------------
struct tomato_screen
{
	SCROLLINFO sih, siv;

	BITMAPINFO *bi_full;
	BITMAPINFO *bi_part;

	HWND hwnd;

	HDC hdc;
	HGDIOBJ hobject;
	HBITMAP hbitmap;
	unsigned char *bits;

	unsigned char messagebuffer[1024];
	unsigned int messagebufferlength;

	POINT cursor;
	WORD hotspotx, hotspoty;

	BOOL tracecursor;
	BOOL control;
	UINT framescount;
	UINT cursorindex;

	LPVOID *parameter;
};
//---------------------------------------------------------------------------
void tomato_screen_initialize(struct tomato_screen *pscreen);
void tomato_screen_uninitialize(struct tomato_exports *pes, struct tomato_screen *pscreen);

HWND ScreenWindowCreate(struct tomato_screen *pscreen, HINSTANCE hinstance);
//---------------------------------------------------------------------------
#endif