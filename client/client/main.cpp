#include "tomato.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	WSADATA wsad;

	WSAStartup(MAKEWORD(2, 2), &wsad);

	struct tomato ptomato[1];
	struct tomato_logon *plogon = ptomato->plogon;
	HINSTANCE hinstance;
	MSG msg;

	tomato_initialize(ptomato);

	plogon->ps = ptomato->ps;

	hinstance = GetModuleHandle(NULL);

	LogonWindowCreate(plogon, hinstance);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	tomato_uninitialize(ptomato);

	WSACleanup();

	return(0);
}