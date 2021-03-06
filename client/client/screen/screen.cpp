#include "screen.h"
//---------------------------------------------------------------------------
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
//---------------------------------------------------------------------------
#define DEF_STEP	19
#define OFF_SET		24
//---------------------------------------------------------------------------
LPBITMAPINFO screen_create_bitmap(unsigned char bitcount, unsigned int width, unsigned int height)
{
	// biBitCount 为1 (黑白二色图) 、4 (16 色图) 、8 (256 色图) 时由颜色表项数指出颜色表大小
	// biBitCount 为16 (16 位色图) 、24 (真彩色图, 不支持) 、32 (32 位色图) 时没有颜色表

	INT numberofcolors = bitcount <= 8 ? (1 << bitcount) : 0;
	INT i;
	COLORREF color;
	DWORD bisize;
	BITMAPINFO *pbmi;
	BITMAPINFOHEADER *lpbmih;
	HWND hdesktop;
	HDC hdc;
	HBITMAP hbitmap;

	bisize = sizeof(BITMAPINFOHEADER) + (numberofcolors * sizeof(RGBQUAD));

	pbmi = (BITMAPINFO *)MALLOC(bisize);
	if (pbmi != NULL)
	{
		lpbmih = &pbmi->bmiHeader;
		lpbmih->biSize = sizeof(BITMAPINFOHEADER);
		lpbmih->biWidth = width;
		lpbmih->biHeight = height;
		lpbmih->biPlanes = 1;
		lpbmih->biBitCount = bitcount;
		lpbmih->biCompression = BI_RGB;
		lpbmih->biXPelsPerMeter = 0;
		lpbmih->biYPelsPerMeter = 0;
		lpbmih->biClrUsed = 0;
		lpbmih->biClrImportant = 0;
		lpbmih->biSizeImage = (((lpbmih->biWidth*lpbmih->biBitCount + 31)&~31) >> 3)*lpbmih->biHeight;

		if (bitcount < 15)
        {
			//Windows 95和Windows 98：如果lpvBits参数为NULL并且GetDIBits成功地填充了BITMAPINFO结构，那么返回值为位图中总共的扫描线数。
			//Windows NT：如果lpvBits参数为NULL并且GetDIBits成功地填充了BITMAPINFO结构，那么返回值为非0。如果函数执行失败，那么将返回0值。Windows NT：若想获得更多错误信息，请调用callGetLastError函数。

			hdesktop = GetDesktopWindow();
			hdc = GetDC(hdesktop);
			hbitmap = CreateCompatibleBitmap(hdc, 1, 1); // 高宽不能为0
			GetDIBits(hdc, hbitmap, 0, 0, NULL, pbmi, DIB_RGB_COLORS);
			ReleaseDC(hdesktop, hdc);
			DeleteObject(hbitmap);
        }
	}

	return(pbmi);
}

void screen_initialize(struct screen_info *psi)
{
	psi->bitcount = 0;

	psi->width = 0;
	psi->height = 0;
	psi->increment = 0;
	psi->startline = 0;

	//SwitchInputDesktop();

	psi->hdc = NULL;
	psi->hdcpart = NULL;
	psi->hdcline = NULL;
	psi->bits = NULL;
	psi->linebits = NULL;

	psi->pbmi = NULL;
	psi->pbmipart = NULL;
	psi->pbmiline = NULL;

	psi->hbitmap = NULL;
	psi->hline = NULL;

	psi->buffer0 = NULL;
	psi->buffer1 = NULL;
	psi->buffersize0 = 0;
	psi->buffersize1 = 0;
	psi->rowsize = 0;

	psi->lastcursory = 0;
}
void screen_uninitialize(struct screen_info *psi)
{
	DeleteDC(psi->hdc);
	DeleteDC(psi->hdcpart);
	DeleteDC(psi->hdcline);

	DeleteObject(psi->hbitmap);
	DeleteObject(psi->hline);

	if (psi->buffer0)
	{
		FREE(psi->buffer0);
		psi->buffer0 = NULL;
	}
	if (psi->buffer1)
	{
		FREE(psi->buffer1);
		psi->buffer1 = NULL;
	}
	FREE(psi->pbmi);
	FREE(psi->pbmipart);
	FREE(psi->pbmiline);
}

void screen_capture(struct screen_info *psi, unsigned int offset, unsigned char bitcount)
{
	HWND hdesktop;
	HDC hdc;

	switch (bitcount)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 32:
		break;
	default:
		bitcount = 8;
		break;
	}

	psi->bitcount = bitcount;

	psi->width = GetSystemMetrics(SM_CXSCREEN);
	psi->height = GetSystemMetrics(SM_CYSCREEN);
	psi->increment = 32 / bitcount;
	psi->startline = 0;

	//SwitchInputDesktop();

	hdesktop = GetDesktopWindow();
	hdc = GetDC(hdesktop);

	psi->hdc = CreateCompatibleDC(hdc);
	psi->hdcpart = CreateCompatibleDC(NULL);
	psi->hdcline = CreateCompatibleDC(NULL);
	psi->bits = NULL;
	psi->linebits = NULL;

	psi->pbmi = screen_create_bitmap(bitcount, psi->width, psi->height);
	psi->pbmipart = screen_create_bitmap(bitcount, psi->width, 1);
	psi->pbmiline = screen_create_bitmap(bitcount, psi->width, 1);

	psi->hbitmap = CreateDIBSection(hdc, psi->pbmi, DIB_RGB_COLORS, (void **)&psi->bits, NULL, NULL);
	psi->hline = CreateDIBSection(hdc, psi->pbmiline, DIB_RGB_COLORS, (void **)&psi->linebits, NULL, NULL);

	SelectObject(psi->hdc, psi->hbitmap);
	SelectObject(psi->hdcline, psi->hline);

	ReleaseDC(hdesktop, hdc);

	SetRect(&psi->rectangle, 0, 0, psi->width, psi->height);

	psi->buffersize0 = offset + psi->pbmi->bmiHeader.biSize + (bitcount <=8 ? (sizeof(RGBQUAD) << bitcount) : 0) + psi->pbmi->bmiHeader.biSizeImage;
	psi->buffersize1 = psi->pbmi->bmiHeader.biSizeImage;
	psi->buffer0 = (unsigned char *)MALLOC(psi->buffersize0);
	psi->buffer1 = (unsigned char *)MALLOC(psi->buffersize1);
	psi->rowsize = psi->pbmi->bmiHeader.biSizeImage / psi->height;

	psi->lastcursory = 0;
}

void screen_copy_rectangle(struct screen_info *psi, unsigned int *offset, const RECT *prectangle)
{
	int width = prectangle->right - prectangle->left;
	int height = prectangle->bottom - prectangle->top;
	HWND hdesktop;
	HDC hdc;
	HGDIOBJ hobject;
	HBITMAP	hbitmap;
	LPVOID bits;

	psi->pbmipart->bmiHeader.biWidth = width;
	psi->pbmipart->bmiHeader.biHeight = height;
	psi->pbmipart->bmiHeader.biSizeImage = (((psi->pbmipart->bmiHeader.biWidth*psi->pbmipart->bmiHeader.biBitCount + 31)&~31) >> 3)*psi->pbmipart->bmiHeader.biHeight;

	if (*offset + sizeof(RECT) + psi->pbmipart->bmiHeader.biSizeImage < psi->buffersize0)
	{

		hdesktop = GetDesktopWindow();
		hdc = GetDC(hdesktop);

		hbitmap = CreateDIBSection(hdc, psi->pbmipart, DIB_RGB_COLORS, &bits, NULL, NULL);
		hobject = SelectObject(psi->hdcpart, hbitmap);
		BitBlt(psi->hdc, prectangle->left, prectangle->top, width, height, hdc, prectangle->left, prectangle->top, SRCCOPY);
		BitBlt(psi->hdcpart, 0, 0, width, height, psi->hdc, prectangle->left, prectangle->top, SRCCOPY);

		CopyMemory(psi->buffer0 + *offset, (LPBYTE)prectangle, sizeof(RECT));
		(*offset) += sizeof(RECT);
		CopyMemory(psi->buffer0 + *offset, (LPBYTE)bits, psi->pbmipart->bmiHeader.biSizeImage);
		(*offset) += psi->pbmipart->bmiHeader.biSizeImage;
		SelectObject(psi->hdcpart, hobject);
		DeleteObject(hbitmap);

		ReleaseDC(hdesktop, hdc);
	}
}
int screen_scan_changed_rectangle(struct screen_info *psi, unsigned int *offset, int startline)
{
	unsigned int *p0;
	unsigned int *p1;
	LONG j;
	int result = 0;
	HWND hdesktop;
	HDC hdc;

	hdesktop = GetDesktopWindow();
	hdc = GetDC(hdesktop);
	BitBlt(psi->hdcline, 0, 0, psi->width, 1, hdc, 0, startline, SRCCOPY);
	ReleaseDC(hdesktop, hdc);

	p0 = (unsigned int *)((unsigned char *)psi->bits + ((psi->height - 1 - startline) * psi->rowsize));
	p1 = (unsigned int *)psi->linebits;

	SetRect(&psi->rectangle, -1, startline - DEF_STEP, -1, startline + DEF_STEP + DEF_STEP);

	for (j = 0; j < psi->width; j += psi->increment)
	{
		if (*p0 != *p1)
		{
			if (psi->rectangle.right < 0)
			{
				psi->rectangle.left = j - OFF_SET;
			}

			psi->rectangle.right = j + OFF_SET;
		}

		p0++;
		p1++;
	}

	if (psi->rectangle.right > -1)
	{
		psi->rectangle.left = max(psi->rectangle.left, 0);
		psi->rectangle.top = max(psi->rectangle.top, 0);
		psi->rectangle.right = min(psi->rectangle.right, psi->width);
		psi->rectangle.bottom = min(psi->rectangle.bottom, psi->height);

		screen_copy_rectangle(psi, offset, &psi->rectangle);

		result = TRUE;
	}

	return(result);
}

BITMAPINFO *screen_get_bmi(struct screen_info *psi, unsigned int *size)
{
	UINT numberofcolors;

	if (size)
	{
		numberofcolors = psi->bitcount <= 8 ? (1 << psi->bitcount) : 0;

		*size = sizeof(BITMAPINFOHEADER) + numberofcolors * sizeof(RGBQUAD);
	}

	return(psi->pbmi);
}
unsigned int screen_get_bmisize(struct screen_info *psi)
{
	UINT numberofcolors = psi->bitcount <= 8 ? (1 << psi->bitcount) : 0;

	return(sizeof(BITMAPINFOHEADER) + numberofcolors*sizeof(RGBQUAD));
}
unsigned char *screen_get_first(struct screen_info *psi, unsigned int *size)
{
	HWND hdesktop;
	HDC hdc;

	hdesktop = GetDesktopWindow();
	hdc = GetDC(hdesktop);
	BitBlt(psi->hdc, 0, 0, psi->width, psi->height, hdc, 0, 0, SRCCOPY);
	ReleaseDC(hdesktop, hdc);

	if (size)
	{
		*size = psi->pbmi->bmiHeader.biSizeImage;
	}

	return(psi->bits);
}
unsigned char *screen_get_next(struct screen_info *psi, unsigned int *size, unsigned int offset)
{
	int hotspot, i;
	POINT *ppoint;
	BYTE cursorindex;

	ppoint = (POINT *)(psi->buffer0 + offset);
	ppoint->x = 0;
	ppoint->y = 0;
	GetCursorPos(ppoint);
	offset += sizeof(POINT);

	// 写入当前光标类型
	//cursorindex = 1;
	//CopyMemory(psi->buffer0 + offset, &cursorindex, sizeof(BYTE));
	//offset += sizeof(BYTE);

	// 鼠标位置发变化并且热点区域如果发生变化，以(发生变化的行 + DEF_STEP)向下扫描
	// 向上提
	if (ppoint->y > DEF_STEP)
	{
		hotspot = ppoint->y - DEF_STEP;
	}
	else
	{
		hotspot = 0;
	}
	for (i = ((ppoint->y != psi->lastcursory) && screen_scan_changed_rectangle(psi, &offset, hotspot)) ? (hotspot + DEF_STEP) : psi->startline; i < psi->height; i += DEF_STEP)
	{
		if (screen_scan_changed_rectangle(psi, &offset, i))
		{
			i += DEF_STEP;
		}
	}
	psi->lastcursory = ppoint->y;

	psi->startline = (psi->startline + 3) % DEF_STEP;

	*size = offset;

	return(psi->buffer0);
}
//---------------------------------------------------------------------------