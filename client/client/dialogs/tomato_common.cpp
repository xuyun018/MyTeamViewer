#include "tomato_common.h"
//---------------------------------------------------------------------------
unsigned int generate_key(char *text, unsigned int n, unsigned int *ra, unsigned int length)
{
	char cs[36];
	unsigned int namelength;
	unsigned int i;
	unsigned int j;
	unsigned int l;
	unsigned int result = 0;
	int a;

	a = *ra;

	for (i = 0; i < 10; i++)
	{
		cs[i] = i + '0';
	}
	for (i = 0; i < 26; i++)
	{
		cs[i + 10] = i + 'a';
	}

	if (length == 0)
	{
		length++;
	}
	namelength = length;
	namelength = namelength < n ? namelength : n;
	for (i = 0; i < namelength; i++)
	{
		text[i] = cs[(((a = a * 214013LL + 2531011LL) >> 16) & 0x7fff) % 36];
	}
	text[i] = '\0';

	result = i;

	*ra = a;
	return(result);
}

VOID WINAPI WindowSetFont(HWND hwnd, HFONT hfont)
{
	if (hfont == NULL)
	{
		hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE, 0));
}
VOID WINAPI WindowSetGUIFont(HWND hwnd)
{
	WindowSetFont(hwnd, NULL);
}
//---------------------------------------------------------------------------