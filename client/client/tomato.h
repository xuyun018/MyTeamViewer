//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
#ifndef TOMATO_CLIENT_H
#define TOMATO_CLIENT_H
//---------------------------------------------------------------------------
#include "../../xyannounce.h"
#include "../../tomato_coder.h"

#include "dialogs/tomato_logon.h"
#include "dialogs/tomato_screen.h"
//---------------------------------------------------------------------------
struct tomato
{
	struct tomato_exports pes[1];

	struct tomato_logon plogon[1];
	struct tomato_screen pscreen[1];

	struct xypagebuffer pb0[1];
	struct xypagebuffer pb1[1];

	XYSOCKET ps[1];

	HINSTANCE hinstance;
};
//---------------------------------------------------------------------------
void tomato_initialize(struct tomato *ptomato);
void tomato_uninitialize(struct tomato *ptomato);

//---------------------------------------------------------------------------
#endif