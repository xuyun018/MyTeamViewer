#include "tomato_screen.h"
//---------------------------------------------------------------------------
void tomato_screen_initialize(struct tomato_screen *pscreen)
{
	pscreen->bi_full = NULL;
	pscreen->bi_part = NULL;

	pscreen->hwnd = NULL;

	pscreen->hdc = NULL;
	pscreen->hobject = NULL;
	pscreen->hbitmap = NULL;
	pscreen->bits = NULL;

	pscreen->messagebufferlength = 0;
}
void tomato_screen_uninitialize(struct tomato_exports *pes, struct tomato_screen *pscreen)
{
	if (pscreen->hobject)
	{
		SelectObject(pscreen->hdc, pscreen->hobject);
		pscreen->hobject = NULL;
	}

	if (pscreen->hbitmap)
	{
		DeleteObject(pscreen->hbitmap);
		pscreen->hbitmap = NULL;
	}
	pscreen->bits = NULL;

	if (pscreen->hdc)
	{
		DeleteDC(pscreen->hdc);
		pscreen->hdc = NULL;
	}

	if (pscreen->bi_full)
	{
		FREE((void *)pscreen->bi_full);
		pscreen->bi_full = NULL;
	}
	if (pscreen->bi_part)
	{
		FREE((void *)pscreen->bi_part);
		pscreen->bi_part = NULL;
	}
}
//---------------------------------------------------------------------------
VOID SetScrollBoxDimension(struct tomato_screen *pscreen)
{
	ZeroMemory(&pscreen->sih, sizeof(SCROLLINFO));
	ZeroMemory(&pscreen->siv, sizeof(SCROLLINFO));

	pscreen->sih.cbSize = pscreen->siv.cbSize = sizeof(SCROLLINFO);
	pscreen->sih.fMask = pscreen->siv.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	pscreen->sih.nMax = pscreen->bi_full->bmiHeader.biWidth;
	pscreen->siv.nMax = pscreen->bi_full->bmiHeader.biHeight;
}

VOID tomato_set_bmi(struct tomato_screen *pscreen, HWND hwnd, const unsigned char *buffer, int length)
{
	HDC hdc;

	pscreen->bi_full = (BITMAPINFO *)MALLOC(length);
	pscreen->bi_part = (BITMAPINFO *)MALLOC(length);

	CopyMemory(pscreen->bi_full, buffer, length);
	CopyMemory(pscreen->bi_part, pscreen->bi_full, length);

	hdc = GetDC(hwnd);
	pscreen->hdc = CreateCompatibleDC(hdc);
	pscreen->hbitmap = CreateDIBSection(hdc, pscreen->bi_full, DIB_RGB_COLORS, (void **)&pscreen->bits, NULL, NULL);
	pscreen->hobject = SelectObject(pscreen->hdc, pscreen->hbitmap);
	ReleaseDC(hwnd, hdc);

	SetScrollBoxDimension(pscreen);
}
void tomato_draw_first_screen(struct tomato_screen *pscreen, const unsigned char *buffer, unsigned int length)
{
	CopyMemory(pscreen->bits, buffer, length);

	RECT rectangle;
	UINT cx, cy;
	GetClientRect(pscreen->hwnd, &rectangle);

	cx = rectangle.right - rectangle.left;
	cy = rectangle.bottom - rectangle.top;

	pscreen->sih.nPage = cx;
	pscreen->siv.nPage = cy;

	if ((cx = pscreen->sih.nMax - cx) < pscreen->sih.nPos)
	{
		pscreen->sih.nPos = cx < 0 ? 0 : cx;
	}

	if ((cy = pscreen->siv.nMax - cy) < pscreen->siv.nPos)
	{
		pscreen->siv.nPos = cy < 0 ? 0 : cy;
	}

	SetScrollInfo(pscreen->hwnd, SB_HORZ, &pscreen->sih, FALSE);
	SetScrollInfo(pscreen->hwnd, SB_VERT, &pscreen->siv, FALSE);

	InvalidateRect(pscreen->hwnd, NULL, TRUE);
}
void tomato_draw_next_screen(struct tomato_screen *pscreen, const unsigned char *buffer, int length)
{
	// 根据鼠标是否移动和鼠标是否在变化的区域判断是否重绘鼠标，防止鼠标闪烁
	BOOL redraw = FALSE;
	int headerlength = sizeof(POINT); // 光标位置 + 光标类型索引
	unsigned char *bits = pscreen->bits;
	const unsigned char *next = buffer + headerlength;
	DWORD size = length - headerlength;
	RECT rectangle0, rectangle1;	// 保存上次鼠标所在的位置
	PRECT prectangle;
	HDC hdc;

	SetRect(&rectangle0, pscreen->cursor.x, pscreen->cursor.y, pscreen->cursor.x + pscreen->hotspotx, pscreen->cursor.y + pscreen->hotspoty);

	CopyMemory(&pscreen->cursor, buffer, sizeof(POINT));

	// 判断鼠标是否移动
	if ((rectangle0.left != pscreen->cursor.x) || (rectangle0.top != pscreen->cursor.y))
		redraw = TRUE;

	// 光标类型发生变化
	int	oldcursorindex = pscreen->cursorindex;
	pscreen->cursorindex = buffer[sizeof(POINT)];
	if (oldcursorindex != pscreen->cursorindex)
	{
		redraw = TRUE;
		//if(control && !tracecursor)
		//SetClassLong(Handle,GCL_HCURSOR,(LONG)m_CursorInfo.getCursorHandle(cursorindex==(BYTE)-1?1:cursorindex));
	}

	// 判断鼠标所在区域是否发生变化
	UINT offset = 0;
	while (offset < size && !redraw)
	{
		prectangle = (PRECT)(next + offset);

		if (IntersectRect(&rectangle1, &rectangle0, prectangle))
			redraw = TRUE;

		offset += sizeof(RECT) + pscreen->bi_part->bmiHeader.biSizeImage;
	}
	redraw = redraw && pscreen->tracecursor;

	//bIsReDraw=TRUE;
	//////////////////////////////////////////////////////////////////////////
	hdc = GetDC(pscreen->hwnd);

	offset = 0;
	while (offset<size)
	{
		prectangle = (PRECT)(next + offset);
		int	width = prectangle->right - prectangle->left;
		int	height = prectangle->bottom - prectangle->top;

		pscreen->bi_part->bmiHeader.biWidth = width;
		pscreen->bi_part->bmiHeader.biHeight = height;
		pscreen->bi_part->bmiHeader.biSizeImage = (((pscreen->bi_part->bmiHeader.biWidth*pscreen->bi_part->bmiHeader.biBitCount + 31)&~31) >> 3)*pscreen->bi_part->bmiHeader.biHeight;

		StretchDIBits(pscreen->hdc, prectangle->left, prectangle->top, width,
			height, 0, 0, width, height, next + offset + sizeof(RECT),
			pscreen->bi_part, DIB_RGB_COLORS, SRCCOPY);

		// 不需要重绘鼠标的话，直接重绘变化的部分
		/*
		if (!bIsReDraw)
		StretchDIBits(hdc,lpRect->left-m_HScrollPos,lpRect->top - m_VScrollPos,nRectWidth,
		nRectHeight, 0, 0, nRectWidth, nRectHeight, (LPBYTE)lpNextScreen+dwOffset+sizeof(RECT),
		lpbi_part,DIB_RGB_COLORS,SRCCOPY);
		*/
		if (!redraw)
			StretchDIBits(hdc, prectangle->left - pscreen->sih.nPos, prectangle->top - pscreen->siv.nPos, width,
				height, 0, 0, width, height, next + offset + sizeof(RECT),
				pscreen->bi_part, DIB_RGB_COLORS, SRCCOPY);

		offset += sizeof(RECT) + pscreen->bi_part->bmiHeader.biSizeImage;
	}

	ReleaseDC(pscreen->hwnd, hdc);

	if (redraw)
	{
		InvalidateRect(pscreen->hwnd, NULL, TRUE);
	}
}
void ResetScreen(struct tomato_screen *pscreen, const char *buffer, int length)
{
	INT width, height;
	HDC hdc;

	if (pscreen->bi_full != NULL)
	{
		width = pscreen->bi_full->bmiHeader.biWidth;
		height = pscreen->bi_full->bmiHeader.biHeight;

		FREE((void *)pscreen->bi_full);
		FREE((void *)pscreen->bi_part);

		pscreen->bi_full = (BITMAPINFO *)MALLOC(length);
		pscreen->bi_part = (BITMAPINFO *)MALLOC(length);

		CopyMemory(pscreen->bi_full, buffer, length);
		CopyMemory(pscreen->bi_part, pscreen->bi_full, length);

		hdc = GetDC(pscreen->hwnd);
		SelectObject(pscreen->hdc, pscreen->hobject);
		DeleteObject(pscreen->hbitmap);
		pscreen->hbitmap = CreateDIBSection(hdc, pscreen->bi_full, DIB_RGB_COLORS, (void **)&pscreen->bits, NULL, NULL);
		pscreen->hobject = SelectObject(pscreen->hdc, pscreen->hbitmap);
		ReleaseDC(pscreen->hwnd, hdc);

		// 分辨率发生改变
		if (width != pscreen->bi_full->bmiHeader.biWidth || height != pscreen->bi_full->bmiHeader.biHeight)
		{
			SetScrollBoxDimension(pscreen);
		}
	}
}
//---------------------------------------------------------------------------
VOID ScreenWindowOnCreate(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;
	CREATESTRUCT *pcs;
	HMENU hmenu;

	pcs = (CREATESTRUCT *)lparam;
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)pcs->lpCreateParams);

	pscreen = (struct tomato_screen *)pcs->lpCreateParams;

	pscreen->hwnd = hwnd;

	ICONINFO ii;
	GetIconInfo(LoadCursor(NULL, IDC_ARROW), &ii);
	if (ii.hbmMask != NULL)
	{
		DeleteObject(ii.hbmMask);
	}
	if (ii.hbmColor != NULL)
	{
		DeleteObject(ii.hbmColor);
	}

	pscreen->cursor.x = 0;
	pscreen->cursor.y = 0;
	pscreen->hotspotx = ii.xHotspot;
	pscreen->hotspoty = ii.yHotspot;
	pscreen->tracecursor = FALSE;

	pscreen->control = FALSE; // 默认不控制
	pscreen->framescount = 0;
	pscreen->cursorindex = 1;
	pscreen->tracecursor = FALSE;

	pscreen->bi_full = NULL;
	pscreen->bi_part = NULL;
}
VOID ScreenWindowOnClose(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}
VOID ScreenWindowOnDestroy(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;
	char *lpbuffer;
	int offset;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}
VOID ScreenWindowOnResize(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;
	LONG cx = LOWORD(lparam);
	LONG cy = HIWORD(lparam);

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pscreen->bi_full == NULL)
	{
		return;
	}

	pscreen->sih.nPage = cx;
	pscreen->siv.nPage = cy;

	if ((cx = pscreen->sih.nMax - cx) < pscreen->sih.nPos)
	{
		pscreen->sih.nPos = cx < 0 ? 0 : cx;
	}

	if ((cy = pscreen->siv.nMax - cy) < pscreen->siv.nPos)
	{
		pscreen->siv.nPos = cy < 0 ? 0 : cy;
	}

	SetScrollInfo(hwnd, SB_HORZ, &pscreen->sih, FALSE);
	SetScrollInfo(hwnd, SB_VERT, &pscreen->siv, FALSE);
	InvalidateRect(hwnd, NULL, FALSE);
}
VOID ScreenWindowOnPaint(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;
	PAINTSTRUCT paint;
	HDC hdc;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pscreen->hbitmap)
	{
		hdc = BeginPaint(hwnd, &paint);
		BitBlt(hdc, -pscreen->sih.nPos, -pscreen->siv.nPos, pscreen->bi_full->bmiHeader.biWidth, pscreen->bi_full->bmiHeader.biHeight, pscreen->hdc, 0, 0, SRCCOPY);
		EndPaint(hwnd, &paint);

		//if(ps->tracecursor)
		{
		}
	}
}
VOID ScreenWindowOnHScroll(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;
	LONG dx = 0;
	LONG pos;
	UINT code;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	pos = (LONG)(short)HIWORD(wparam);

	code = (UINT)LOWORD(wparam);
	switch (code)
	{
	case SB_LINEUP:
		dx -= 10;
		break;
	case SB_LINEDOWN:
		dx = 10;
		break;
	case SB_PAGEUP:
		dx = dx - pscreen->sih.nPage;
		break;
	case SB_PAGEDOWN:
		dx = pscreen->sih.nPage;
		break;
	case SB_THUMBTRACK:
		dx = pos - pscreen->sih.nPos;
		break;
	default:
		return;
	}

	dx = max(-pscreen->sih.nPos, min(dx, pscreen->sih.nMax - pscreen->sih.nPos));

	if (dx != 0)
	{
		pscreen->sih.nPos += dx;

		if (pscreen->sih.nPos>pscreen->sih.nMax - pscreen->sih.nPage)
		{
			pscreen->sih.nPos = pscreen->sih.nMax - pscreen->sih.nPage + 1;
		}

		SetScrollInfo(hwnd, SB_HORZ, &pscreen->sih, FALSE);
		InvalidateRect(hwnd, NULL, FALSE);
	}
}
VOID ScreenWindowOnVScroll(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;
	LONG dy = 0;
	LONG pos;
	UINT code;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	pos = (LONG)(short)HIWORD(wparam);

	code = (UINT)LOWORD(wparam);

	switch (code)
	{
	case SB_LINEUP:
		dy = -10;
		break;
	case SB_LINEDOWN:
		dy = 10;
		break;
	case SB_PAGEUP:
		dy = dy - pscreen->siv.nPage;
		break;
	case SB_PAGEDOWN:
		dy = pscreen->siv.nPage;
		break;
	case SB_THUMBTRACK:
		dy = pos - pscreen->siv.nPos;
		break;
	default:
		return;
	}

	dy = max(-pscreen->siv.nPos, min(dy, pscreen->siv.nMax - pscreen->siv.nPos));

	if (dy != 0)
	{
		pscreen->siv.nPos += dy;

		if (pscreen->siv.nPos>pscreen->siv.nMax - pscreen->siv.nPage)
		{
			pscreen->siv.nPos = pscreen->siv.nMax - pscreen->siv.nPage + 1;
		}

		SetScrollInfo(hwnd, SB_VERT, &pscreen->siv, FALSE);
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

void write_messagebuffer(struct tomato_screen *pscreen, MSG *pmsg)
{
	unsigned int messagebuffersize;

	messagebuffersize = sizeof(pscreen->messagebuffer);
	if (sizeof(MSG) + pscreen->messagebufferlength <= messagebuffersize)
	{
		memcpy(pscreen->messagebuffer + pscreen->messagebufferlength, pmsg, sizeof(MSG));
		pscreen->messagebufferlength += sizeof(MSG);
	}
}

VOID ScreenWindowOnMouse(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	MSG msg;
	int positionh;
	int positionv;

	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		if (TRUE)
		{
			positionh = pscreen->sih.nPos;
			positionv = pscreen->siv.nPos;

			msg.message = message;
			msg.wParam = wparam;
			msg.lParam = MAKEDWORD(HIWORD(lparam) + positionv, LOWORD(lparam) + positionh);

			write_messagebuffer(pscreen, &msg);
		}
		break;

	default:
		break;
	}
}
VOID ScreenWindowOnKey(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	MSG msg;
	int positionh;
	int positionv;

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (wparam != VK_LWIN && wparam != VK_RWIN)
		{
			//if (ps->control)
			{
				positionh = pscreen->sih.nPos;
				positionv = pscreen->siv.nPos;

				msg.message = message;
				msg.wParam = wparam;
				msg.lParam = MAKEDWORD(HIWORD(lparam) + positionv, LOWORD(lparam) + positionh);

				write_messagebuffer(pscreen, &msg);
			}
		}
		if (wparam == VK_RETURN || wparam == VK_ESCAPE)
		{
			return;
		}
		break;

	default:
		break;
	}
}

VOID ScreenWindowOnDraw(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_screen *pscreen;
	unsigned char *p;
	unsigned char *buffer = (unsigned char *)lparam;
	unsigned int length = (unsigned int)wparam;
	unsigned int bmisize;
	unsigned int o;

	pscreen = (struct tomato_screen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	p = buffer;

	switch (*buffer)
	{
	case 0:
		p++;
		p++;
		p++;

		bmisize = readvalue2(p);
		p += 2;

		tomato_set_bmi(pscreen, hwnd, p, bmisize);
		p += bmisize;

		tomato_draw_first_screen(pscreen, p, pscreen->bi_full->bmiHeader.biSizeImage);
		break;
	case 1:
		p++;
		p++;
		p++;

		tomato_draw_next_screen(pscreen, p, length - 3);
		break;
	default:
		break;
	}
}
//---------------------------------------------------------------------------
LRESULT CALLBACK ScreenWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
		ScreenWindowOnCreate(hwnd, wparam, lparam);
		break;
	case WM_SIZE:
		ScreenWindowOnResize(hwnd, wparam, lparam);
		break;
	case WM_PAINT:
		ScreenWindowOnPaint(hwnd, wparam, lparam);
		break;
	case WM_SYSCOMMAND:
		break;
	case WM_HSCROLL:
		ScreenWindowOnHScroll(hwnd, wparam, lparam);
		break;
	case WM_VSCROLL:
		ScreenWindowOnVScroll(hwnd, wparam, lparam);
		break;
	case WM_CLOSE:
		ScreenWindowOnClose(hwnd, wparam, lparam);
		break;
	case WM_DESTROY:
		ScreenWindowOnDestroy(hwnd, wparam, lparam);
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		ScreenWindowOnMouse(hwnd, message, wparam, lparam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		ScreenWindowOnKey(hwnd, message, wparam, lparam);
		break;

	case WM_TOMATO_DRAW:
		ScreenWindowOnDraw(hwnd, wparam, lparam);
		break;

	default:
		break;
	}

	return(DefWindowProc(hwnd, message, wparam, lparam));
}
//---------------------------------------------------------------------------
HWND ScreenWindowCreate(struct tomato_screen *pscreen, HINSTANCE hinstance)
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX winclass;
	ULONG width = 780, height = 560;

	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC;
	winclass.lpfnWndProc = ScreenWindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = NULL;	// LoadIcon(hinstance, MAKEINTRESOURCE(ID_ICON_MAIN));
	winclass.hIconSm = NULL;	// LoadIcon(hinstance, MAKEINTRESOURCE(ID_ICON_MAIN));
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//winclass.hbrBackground=(HBRUSH)GetStockObject(GRAY_BRUSH);
	winclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = _T("TomatoRemoteScreenClass");

	RegisterClassEx(&winclass);

	hwnd = CreateWindowEx(NULL,
		winclass.lpszClassName,
		_T(""),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hinstance,
		(LPVOID)pscreen);

	return(hwnd);
}
//---------------------------------------------------------------------------