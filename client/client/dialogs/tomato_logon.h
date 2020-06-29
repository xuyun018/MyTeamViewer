//---------------------------------------------------------------------------
// 内存块管理的基础类
//---------------------------------------------------------------------------
#ifndef TOMATO_LOGON_H
#define TOMATO_LOGON_H
//---------------------------------------------------------------------------
#include "../../../xyannounce.h"

#include "../../../tomato_coder.h"

#include "tomato_common.h"
#include "tomato_screen.h"

#include "../screen/screen.h"

#include <CommCtrl.h>
//---------------------------------------------------------------------------
struct tomato_connect_parameter
{
	HANDLE hevent;

	DWORD tickcount;

	SOCKET fd;
};
//---------------------------------------------------------------------------
struct tomato_logon
{
	struct tomato_exports *pes;

	struct tomato_screen *pscreen;

	HWND hwnd;

	HWND huid_local;
	HWND hpwd_local;

	// 刷新密码
	HWND hpassword;

	HWND huid_remote;
	HWND hpwd_remote;

	// 连接
	HWND hconnect;

	tomato_uid uid_local;
	tomato_uid uid_remote;

	char pwd_local[8];
	char pwd_remote[8];

	struct screen_info psi[1];

	struct tomato_connect_parameter pcp[1];

	XYSOCKET *ps;

	UINT timer;
	// 连接计数
	UINT connect_count;

	int sent_screen;
};
//---------------------------------------------------------------------------
void tomato_logon_initialize(struct tomato_logon *plogon);
void tomato_logon_uninitialize(struct tomato_logon *plogon);

HWND LogonWindowCreate(struct tomato_logon *plogon, HINSTANCE hinstance);
//---------------------------------------------------------------------------
#endif