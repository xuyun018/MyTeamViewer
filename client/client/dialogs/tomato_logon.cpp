#include "tomato_logon.h"
//---------------------------------------------------------------------------
void tomato_logon_initialize(struct tomato_logon *plogon)
{
	plogon->uid_local = 0;
	plogon->uid_remote = 0;

	plogon->pwd_local[0] = '\0';
	plogon->pwd_remote[0] = '\0';

	struct tomato_connect_parameter *pcp = plogon->pcp;

	pcp->hevent = CreateEvent(NULL, TRUE, TRUE, NULL);

	pcp->fd = -1;
}
void tomato_logon_uninitialize(struct tomato_logon *plogon)
{
	struct tomato_connect_parameter *pcp = plogon->pcp;

	if (pcp->hevent)
	{
		SetEvent(pcp->hevent);

		WaitForSingleObject(pcp->hevent, INFINITE);
		CloseHandle(pcp->hevent);
	}
}

VOID tomato_dispose_controlmessages(const unsigned char *buffer, unsigned int length)
{
	MSG *pmsg;
	UINT i, count;
	POINT point;

	//SwitchInputDesktop();

	if (length % sizeof(MSG) == 0)
	{
		count = length / sizeof(MSG);

		for (i = 0; i < count; i++)
		{
			pmsg = (MSG *)(buffer + i * sizeof(MSG));
			switch (pmsg->message)
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
				point.x = LOWORD(pmsg->lParam);
				point.y = HIWORD(pmsg->lParam);
				SetCursorPos(point.x, point.y);
				SetCapture(WindowFromPoint(point));
				break;
			default:
				break;
			}

			switch (pmsg->message)
			{
			case WM_LBUTTONDOWN:
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				break;
			case WM_LBUTTONUP:
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				break;
			case WM_RBUTTONDOWN:
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				break;
			case WM_RBUTTONUP:
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				break;
			case WM_LBUTTONDBLCLK:
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				break;
			case WM_RBUTTONDBLCLK:
				mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				break;
			case WM_MBUTTONDOWN:
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
				break;
			case WM_MBUTTONUP:
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
				break;
			case WM_MOUSEWHEEL:
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, GET_WHEEL_DELTA_WPARAM(pmsg->wParam), 0);
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				keybd_event(pmsg->wParam, MapVirtualKey(pmsg->wParam, 0), 0, 0);
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				keybd_event(pmsg->wParam, MapVirtualKey(pmsg->wParam, 0), KEYEVENTF_KEYUP, 0);
				break;
			default:
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
VOID LogonWindowOnCreate(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	CREATESTRUCT *pcs;
	struct tomato_logon *plogon;
	HINSTANCE hinstance;
	HWND hedit;
	HWND hbutton;

	pcs = (CREATESTRUCT *)lparam;

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcs->lpCreateParams);

	plogon = (struct tomato_logon *)pcs->lpCreateParams;

	hinstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	plogon->hwnd = hwnd;

	plogon->huid_local =
		hedit = CreateWindowEx(WS_EX_LTRREADING | WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILDWINDOW | WS_VISIBLE | BS_LEFT, 4, 8, 190, 22, hwnd, NULL, hinstance, NULL);

	WindowSetGUIFont(hedit);

	plogon->hpwd_local =
		hedit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | BS_LEFT, 4, 34, 190, 22, hwnd, NULL, NULL, NULL);

	WindowSetGUIFont(hedit);

	unsigned int ra = GetTickCount();

	WCHAR wcs[24];
	char cs[24];
	generate_key(cs, sizeof(cs), &ra, 6);
	unsigned int i;
	i = 0;
	while (plogon->pwd_local[i] = cs[i])
	{
		i++;
	}
	i = 0;
	while (wcs[i] = cs[i])
	{
		i++;
	}
	SetWindowText(hedit, wcs);

	plogon->hpassword =
		hbutton = CreateWindowEx(WS_EX_LTRREADING, WC_BUTTON, L"刷新",
			WS_CHILDWINDOW |
			WS_VISIBLE,
			104, 72, 90, 24, hwnd, NULL, hinstance, NULL);

	WindowSetGUIFont(hbutton);

	plogon->huid_remote =
		hedit = CreateWindowEx(WS_EX_LTRREADING | WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILDWINDOW | WS_VISIBLE | BS_LEFT, 260, 8, 190, 22, hwnd, NULL, hinstance, NULL);

	WindowSetGUIFont(hedit);

	plogon->hpwd_remote =
		hedit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | BS_LEFT, 260, 34, 190, 22, hwnd, NULL, NULL, NULL);

	WindowSetGUIFont(hedit);

	plogon->hconnect =
		hbutton = CreateWindowEx(WS_EX_LTRREADING, WC_BUTTON, L"连接",
			WS_CHILDWINDOW |
			WS_VISIBLE,
			360, 72, 90, 24, hwnd, NULL, hinstance, NULL);

	WindowSetGUIFont(hbutton);

	//ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);

	//DragAcceptFiles(hwnd, TRUE);

	struct screen_info *psi = plogon->psi;

	screen_initialize(psi);

	WM_DROPFILES;
	WM_COPYDATA;

	plogon->sent_screen = 0;

	plogon->connect_count = 0;

	plogon->timer = SetTimer(hwnd, 1, 10, NULL);
}
VOID LogonWindowOnDestroy(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_logon *plogon;

	plogon = (struct tomato_logon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}
VOID LogonWindowOnClose(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_logon *plogon;

	plogon = (struct tomato_logon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (plogon->timer != 0)
	{
		KillTimer(hwnd, plogon->timer);
		plogon->timer = 0;
	}

	struct screen_info *psi = plogon->psi;

	screen_uninitialize(psi);
}
VOID LogonWindowOnSize(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	//
}
VOID LogonWindowOnTimer(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	//SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	struct tomato_logon *plogon;

	plogon = (struct tomato_logon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (plogon->timer == wparam)
	{
		plogon->connect_count++;
		if (plogon->connect_count == 100)
		{
			plogon->connect_count = 0;

			struct tomato_connect_parameter *pcp = plogon->pcp;
			XYSOCKET *ps = plogon->ps;
			SOCKADDR_IN sai;

			if (WaitForSingleObject(pcp->hevent, 0) == WAIT_OBJECT_0)
			{
				if (pcp->fd == -1)
				{
					pcp->tickcount = GetTickCount();
					ResetEvent(pcp->hevent);

					sai.sin_family = AF_INET;
					sai.sin_port = htons(1024);
					sai.sin_addr.s_addr = inet_addr("127.0.0.1");
					//sai.sin_addr.s_addr = inet_addr("192.168.0.5");

					int sendbuffersize;
					sendbuffersize = 256 * 1024;
					if (XYTCPConnect(ps, (LPVOID)pcp, (const SOCKADDR *)&sai, sizeof(sai), sendbuffersize) != INVALID_SOCKET)
					{
					}
				}
			}
		}

		if (plogon->sent_screen)
		{
			struct tomato_exports *pes = plogon->pes;

			struct screen_info *psi = plogon->psi;
			unsigned char *data;
			unsigned int size;
			unsigned int length;
			unsigned int offset = 0;
			unsigned int l0, l1;
			unsigned int l;

			data = screen_get_next(psi, &size, 12);

			offset = 8;
			data[offset++] = 1;	// 表示发送
			data[offset++] = 1;	// 下一张
			offset += 2;		// 保留

			length = encode_buffer(pes, data, 4 + size, 4, psi->buffer1, psi->buffersize1, TOMATO_POINT_SCREEN, (const unsigned char *)plogon->pwd_remote, 6, TOMATO_BUFFER_FLAG_COMPRESS);
			length = encode_buffer(pes, data, length, 0, NULL, 0, TOMATO_SERVER_TRANSMIT, NULL, 0, 0);

			XYTCPSend(plogon->pcp->fd, (const char *)data, length, 5);
		}
	}
}
VOID LogonWindowOnCommand(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_logon *plogon;
	HWND hedit0;
	HWND hedit1;
	WCHAR wcs[256];
	char cs[256];
	unsigned int ra;
	unsigned int l;
	UINT l0;
	UINT l1;
	UINT i, j;
	UINT o;
	tomato_uid uid;

	plogon = (struct tomato_logon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	//htreeview = pbuilder->htreeview;

	switch (HIWORD(wparam))
	{
	case BN_CLICKED:
		if (lparam == (LPARAM)plogon->hpassword)
		{
			ra = GetTickCount();

			generate_key(cs, sizeof(cs), &ra, 6);
			i = 0;
			while (plogon->pwd_local[i] = cs[i])
			{
				i++;
			}
			i = 0;
			while (wcs[i] = cs[i])
			{
				i++;
			}
			SetWindowText(plogon->hpwd_local, wcs);
		}
		else if (lparam == (LPARAM)plogon->hconnect)
		{
			XYSOCKET *ps = plogon->ps;
			struct tomato_connect_parameter *pcp = plogon->pcp;

			struct tomato_exports *pes = plogon->pes;

			if (pcp->fd != -1)
			{
				GetWindowText(plogon->hpwd_remote, wcs, sizeof(wcs) / sizeof(wcs[0]));
				i = 0;
				while (i < sizeof(plogon->pwd_remote) && (plogon->pwd_remote[i] = wcs[i]))
				{
					i++;
				}
				plogon->pwd_remote[i] = '\0';

				GetWindowText(plogon->huid_remote, wcs, sizeof(wcs) / sizeof(wcs[0]));

				plogon->uid_remote =
					uid = _wcs2ll(wcs, 0, (int *)&o);
				writevalue8((unsigned char *)cs + 4, uid);
				l = encode_buffer(pes, (unsigned char *)cs, sizeof(tomato_uid), 0, NULL, 0, TOMATO_SERVER_UID, NULL, 0, 0);

				XYTCPSend(pcp->fd, cs, l, 5);
			}
		}
		break;
	default:
		break;
	}
}

VOID LogonWindowOnUID(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_logon *plogon;
	WCHAR cs[24];

	plogon = (struct tomato_logon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	_ll2wcs(plogon->uid_local, cs, 0, 0);

	SetWindowText(plogon->huid_local, cs);
}
VOID LogonWindowOnLogon(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_logon *plogon;

	plogon = (struct tomato_logon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	SetWindowText(hwnd, L"验证通过");

	HINSTANCE hinstance;

	hinstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	struct tomato_exports *pes = plogon->pes;

	struct screen_info *psi = plogon->psi;
	BITMAPINFO *pbmi;
	unsigned char *bits;
	unsigned int bitssize;
	unsigned int bmisize;

	unsigned char *p;
	unsigned char *data;
	unsigned int size;
	unsigned int length;
	unsigned int offset = 0;
	unsigned int l0, l1;
	unsigned int l;

	switch (wparam)
	{
	case 0:
		screen_capture(psi, 8, 16);
		bits = screen_get_first(psi, &bitssize);
		pbmi = screen_get_bmi(psi, &bmisize);

		data = psi->buffer0;

		p = data + 12;

		writevalue2(p, bmisize);
		p += 2;
		memcpy(p, pbmi, bmisize);
		p += bmisize;
		memcpy(p, bits, bitssize);
		p += bitssize;

		size = p - data;
		size -= 8;

		offset = 8;
		data[offset++] = 1;	// 表示发送
		data[offset++] = 0;	// 第一张
		data[offset++] = pbmi->bmiHeader.biBitCount;
		offset++;			// 保留

		length = encode_buffer(pes, data, size, 4, psi->buffer1, psi->buffersize1, TOMATO_POINT_SCREEN, (const unsigned char *)plogon->pwd_remote, 6, TOMATO_BUFFER_FLAG_COMPRESS);
		length = encode_buffer(pes, data, length, 0, NULL, 0, TOMATO_SERVER_TRANSMIT, NULL, 0, 0);

		XYTCPSend(plogon->pcp->fd, (const char *)data, length, 5);

		plogon->sent_screen = 1;
		break;
	default:
		break;
	}
}
VOID LogonWindowOnDraw(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	struct tomato_logon *plogon;

	plogon = (struct tomato_logon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	HINSTANCE hinstance;

	hinstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	unsigned char *buffer = (unsigned char *)lparam;
	unsigned int length = (unsigned int)wparam;

	if (*buffer != 2)
	{
		struct tomato_screen *pscreen = plogon->pscreen;

		if (pscreen->hwnd == NULL)
		{
			ScreenWindowCreate(pscreen, hinstance);
		}

		if (pscreen->hwnd)
		{
			SendMessage(pscreen->hwnd, WM_TOMATO_DRAW, wparam, lparam);
		}

		if (pscreen->messagebufferlength)
		{
			struct tomato_exports *pes = plogon->pes;

			unsigned char codebuffer[4096];
			unsigned char data[2048];
			unsigned char *p;
			unsigned int offset;
			unsigned int size;
			unsigned int l;

			p = data + 12;

			memcpy(p, pscreen->messagebuffer, pscreen->messagebufferlength);
			size = pscreen->messagebufferlength;

			offset = 8;
			data[offset++] = 1;	// 表示发送
			data[offset++] = 2;	// 控制
			offset += 2;		// 保留

			l = encode_buffer(pes, data, 4 + size, 4, codebuffer, sizeof(codebuffer), TOMATO_POINT_SCREEN, (const unsigned char *)plogon->pwd_remote, 6, TOMATO_BUFFER_FLAG_COMPRESS);
			l = encode_buffer(pes, data, l, 0, NULL, 0, TOMATO_SERVER_TRANSMIT, NULL, 0, 0);

			XYTCPSend(plogon->pcp->fd, (const char *)data, l, 5);

			pscreen->messagebufferlength = 0;
		}
	}
	else
	{
		buffer += 3;
		length -= 3;
		tomato_dispose_controlmessages(buffer, length);
	}
}

LRESULT CALLBACK LogonWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	switch (message)
	{
	case WM_CREATE:
		LogonWindowOnCreate(hwnd, wparam, lparam);
		break;
	case WM_DESTROY:
		LogonWindowOnDestroy(hwnd, wparam, lparam);

		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		LogonWindowOnClose(hwnd, wparam, lparam);
		break;
	case WM_SIZE:
		LogonWindowOnSize(hwnd, wparam, lparam);
		break;
	case WM_KILLFOCUS:
		//WindowOnKillFocus(hwnd, wparam, lparam);
		break;
	case WM_TIMER:
		LogonWindowOnTimer(hwnd, wparam, lparam);
		break;
	case WM_COMMAND:
		LogonWindowOnCommand(hwnd, wparam, lparam);
		break;

	case WM_TOMATO_UID:
		LogonWindowOnUID(hwnd, wparam, lparam);
		break;
	case WM_TOMATO_LOGONED:
		LogonWindowOnLogon(hwnd, wparam, lparam);
		break;
	case WM_TOMATO_DRAW:
		LogonWindowOnDraw(hwnd, wparam, lparam);
		break;
	default:
		//WCHAR debugtext[256];
		//wsprintf(debugtext, L"message %d, wparam %d, lparam %d\r\n", message, wparam, lparam);
		//OutputDebugString(debugtext);
		break;
	}

	if (result == 0)
	{
		result = DefWindowProc(hwnd, message, wparam, lparam);
	}

	return(result);
}

HWND LogonWindowCreate(struct tomato_logon *plogon, HINSTANCE hinstance)
{
	HWND hwnd;
	WNDCLASSEX winclass;
	unsigned int width = 460, height = 240;

	winclass.cbSize = sizeof(WNDCLASSEX);
	// CS_OWNDC
	winclass.style = CS_DBLCLKS;
	winclass.lpfnWndProc = LogonWindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//winclass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	winclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = L"TomatoLogonClass";

	RegisterClassEx(&winclass);

	hwnd = CreateWindowEx(0,
		winclass.lpszClassName,
		L"Tomato",
		WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
		(GetSystemMetrics(SM_CXSCREEN) - width) / 2, (GetSystemMetrics(SM_CYSCREEN) - height) / 2,
		width, height,
		NULL,
		NULL,
		hinstance,
		(LPVOID)plogon);

	return(hwnd);
}
//---------------------------------------------------------------------------