#include "tomato.h"
//---------------------------------------------------------------------------
int CALLBACK SocketProcedure(LPVOID parameter, LPVOID **pointer, LPVOID context, SOCKET s, BYTE type, BYTE number, SOCKADDR *psa, int *salength, const char *buffer, int length)
{
	PXYSOCKET ps = (PXYSOCKET)parameter;
	PXYSOCKET_CONTEXT psc = (PXYSOCKET_CONTEXT)context;
	struct tomato *ptomato = (struct tomato *)ps->parameter0;
	struct tomato_exports *pes = ptomato->pes;
	struct tomato_logon *plogon = ptomato->plogon;
	struct tomato_screen *pscreen = ptomato->pscreen;
	struct xypagebuffer *pb0;
	struct xypagebuffer *pb1;
	unsigned char data[128];
	unsigned char *p;
	unsigned int l;
	unsigned int l0, l1;
	tomato_uid uid;
	int result = 0;
	PSOCKADDR_IN psai;
	struct tomato_connect_parameter *pcp = (struct tomato_connect_parameter *)psc->context;
	unsigned char command;

	switch (number)
	{
	case XYSOCKET_CLOSE:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP:
			break;
		case XYSOCKET_TYPE_TCP0:
			OutputDebugString(L"Client Close\r\n");

			pcp->fd = -1;
			break;
		case XYSOCKET_TYPE_TCP1:
			break;
		default:
			break;
		}
		break;
	case XYSOCKET_CONNECT:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP0:
			switch (length)
			{
			case 0:
				// 成功
				if (pointer == NULL)
				{
					if (GetTickCount() - pcp->tickcount > 30000)
					{
						result = XYSOCKET_ERROR_TIMEOUT;

						OutputDebugString(L"Client Time out\r\n");

						SetEvent(pcp->hevent);
					}
				}
				else
				{
					OutputDebugString(L"Client Ok\r\n");

					l0 = 0;
					if (plogon->uid_local)
					{
						writevalue8(data + 4, plogon->uid_local);
						l0 = sizeof(tomato_uid);
					}
					l = encode_buffer(pes,
						data, l0, 0,
						NULL, 0, TOMATO_SERVER_CONNECTED,
						NULL, 0, 0);

					// 发送
					XYTCPSend(s, (const char *)data, l, 5);

					pcp->fd = s;

					SetEvent(pcp->hevent);
				}
				break;
			case XYSOCKET_ERROR_FAILED:
			case XYSOCKET_ERROR_REFUSED:
			case XYSOCKET_ERROR_OVERFLOW:
			default:
				if (TRUE)
				{
					TCHAR debugtext[256];
					wsprintf(debugtext, L"Client Failed %d\r\n", length);
					OutputDebugString(debugtext);
				}

				SetEvent(pcp->hevent);
				break;
			}
			break;
		case XYSOCKET_TYPE_TCP1:
			switch (length)
			{
			case XYSOCKET_ERROR_ACCEPT:
				break;
			case XYSOCKET_ERROR_ACCEPTED:
				break;
			case XYSOCKET_ERROR_OVERFLOW:
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case XYSOCKET_RECV:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP0:
			if (pointer == NULL)
			{
				pb0 = ptomato->pb0;
				pb1 = ptomato->pb1;

				l = xypagebuffer_write(pes, pb0, (const unsigned char *)buffer, length);
				if (l)
				{
					while (pb0->offset && result == 0)
					{
						p = decode_buffer(pes, pb1,
							pb0->buffer1, pb0->offset, &l1, &l,
							(const unsigned char *)plogon->pwd_local, 6, &command);
						if (p)
						{
							switch (command)
							{
							case TOMATO_SERVER_CONNECTED:
								break;
							case TOMATO_SERVER_UID:
								// 要连接此 UID
								if (l == sizeof(tomato_uid))
								{
									plogon->uid_local = readvalue8(p);

									PostMessage(plogon->hwnd, WM_TOMATO_UID, 0, 0);
								}
								else
								{
									result = 1;
								}
								break;
							case TOMATO_SERVER_DISCONNECTED:
								break;
							case TOMATO_SERVER_BAD:
								break;
							case TOMATO_SERVER_TRANSMIT:
								break;
							case TOMATO_SERVER_LINK:
								writevalue8(data + 4 + 4, plogon->uid_remote);
								l0 = sizeof(tomato_uid);
								l1 = strlen(plogon->pwd_remote);
								// 固定 6
								l1 = 6;
								memcpy(data + 4 + 4 + l0, plogon->pwd_remote, l1);
								l0 += l1;
								data[4 + 4 + l0++] = 0;// 表示屏幕

								l0 = encode_buffer(pes, data, l0, 4, pb1->buffer1, pb1->offset, TOMATO_POINT_PASSWORD, (const unsigned char *)plogon->pwd_remote, l1, 0);
								l = encode_buffer(pes, data, l0, 0, NULL, 0, TOMATO_SERVER_TRANSMIT, NULL, 0, 0);

								XYTCPSend(s, (const char *)data, l, 5);
								break;
							case TOMATO_SERVER_P2P:
								break;
							case TOMATO_POINT_PASSWORD:
								if (l == sizeof(tomato_uid) + 6 + 1)
								{
									uid = readvalue8(p);
									p += sizeof(tomato_uid);
									if (uid == plogon->uid_local && memcmp(p, plogon->pwd_local, 6) == 0)
									{
										p += 6;

										PostMessage(plogon->hwnd, WM_TOMATO_LOGONED, *p, (LPARAM)pscreen);
									}
								}
								else
								{
									result = 1;
								}
								break;
							case TOMATO_POINT_SCREEN:
								if (l >= 4)
								{
									if (*(p++) == 0)
									{
									}
									else
									{
										SendMessage(plogon->hwnd, WM_TOMATO_DRAW, l - 1, (LPARAM)p);

										command = *(p++);
										switch (command)
										{
										case 0:
											break;
										case 1:
											break;
										default:
											break;
										}
									}
								}
								break;
							default:
								break;
							}

							xypagebuffer_read(pb0, NULL, l1);
						}
						else
						{
							break;
						}
					}
				}
			}
			break;
		case XYSOCKET_TYPE_TCP1:
			break;
		default:
			break;
		}
		break;
	case XYSOCKET_SEND:
		break;
	case XYSOCKET_TIMEOUT:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP:
			break;
		case XYSOCKET_TYPE_TCP0:
			//OutputDebugString(_T("Client time out\r\n"));
			break;
		case XYSOCKET_TYPE_TCP1:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return(result);
}
//---------------------------------------------------------------------------
void tomato_initialize(struct tomato *ptomato)
{
	struct tomato_exports *pes = ptomato->pes;

	tomato_exports_load(pes);

	tomato_logon_initialize(ptomato->plogon);
	tomato_screen_initialize(ptomato->pscreen);

	xypagebuffer_initialize(ptomato->pb0, NULL, 0, 4096);
	xypagebuffer_initialize(ptomato->pb1, NULL, 0, 4096);

	ptomato->plogon->pes = pes;
	ptomato->plogon->pscreen = ptomato->pscreen;

	XYSOCKET *ps = ptomato->ps;

	XYSocketsStartup(ps, ptomato, NULL, SocketProcedure);

	XYSocketLaunchThread(ps, XYSOCKET_THREAD_CONNECT, 64);
	XYSocketLaunchThread(ps, XYSOCKET_THREAD_CLIENT, 64);

	ptomato->hinstance = GetModuleHandle(NULL);
}
void tomato_uninitialize(struct tomato *ptomato)
{
	struct tomato_exports *pes = ptomato->pes;

	XYSOCKET *ps = ptomato->ps;

	XYSocketsCleanup(ps);

	tomato_logon_uninitialize(ptomato->plogon);
	tomato_screen_uninitialize(pes, ptomato->pscreen);

	xypagebuffer_uninitialize(pes, ptomato->pb0);
	xypagebuffer_uninitialize(pes, ptomato->pb1);
}
//---------------------------------------------------------------------------