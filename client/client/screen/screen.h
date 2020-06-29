#ifndef SCREEN_H
#define SCREEN_H
//---------------------------------------------------------------------------
#include "../../../xyannounce.h"
//---------------------------------------------------------------------------
struct screen_info
{
	WORD width, height;
	WORD increment, startline;

	RECT rectangle;		// change

	HDC hdc, hdcpart, hdcline;
	HBITMAP hbitmap, hline;
	unsigned char *bits;
	unsigned char *linebits;

	LPBITMAPINFO pbmi, pbmipart, pbmiline;

	unsigned char *buffer0;
	unsigned char *buffer1;
	unsigned int buffersize0;
	unsigned int buffersize1;
	unsigned int rowsize;

	LONG lastcursory;

	unsigned char bitcount;
};
//---------------------------------------------------------------------------
void screen_initialize(struct screen_info *psi);
void screen_uninitialize(struct screen_info *psi);

void screen_capture(struct screen_info *psi, unsigned int offset, unsigned char bitcount);

BITMAPINFO *screen_get_bmi(struct screen_info *psi, unsigned int *size);
unsigned int screen_get_bmisize(struct screen_info *psi);
unsigned char *screen_get_first(struct screen_info *psi, unsigned int *size);
unsigned char *screen_get_next(struct screen_info *psi, unsigned int *size, unsigned int offset);
//---------------------------------------------------------------------------
#endif // RCScreen
