//---------------------------------------------------------------------------
// 内存块管理的基础类
//---------------------------------------------------------------------------
#ifndef TOMATO_CODER_H
#define TOMATO_CODER_H
//---------------------------------------------------------------------------
#include "xyannounce.h"

#include "xypagebuffer.h"
//---------------------------------------------------------------------------
#define TOMATO_BUFFER_FLAG_ENCODE											0x01
#define TOMATO_BUFFER_FLAG_COMPRESS											0x02
//---------------------------------------------------------------------------
unsigned int ipv4_2_buffer(const SOCKADDR_IN *psai, unsigned char *buffer);
unsigned int buffer_2_ipv4(const unsigned char *buffer, SOCKADDR_IN *psai);
unsigned int ipv6_2_buffer(const SOCKADDR_IN6 *psai, unsigned char *buffer);
unsigned int buffer_2_ipv6(const unsigned char *buffer, SOCKADDR_IN6 *psai);

unsigned char *writevalue2(unsigned char *buffer, unsigned int value);
unsigned int readvalue2(const unsigned char *buffer);
unsigned char *writevalue3(unsigned char *buffer, unsigned int value);
unsigned int readvalue3(const unsigned char *buffer);
unsigned char *writevalue4(unsigned char *buffer, unsigned int value);
unsigned int readvalue4(const unsigned char *buffer);
unsigned char *writevalue8(unsigned char *buffer, unsigned long long value);
unsigned long long readvalue8(const unsigned char *buffer);

int _ll2wcs(__int64 value, wchar_t *cs, int n, int maximum);
__int64 _wcs2ll(const wchar_t *cs, int n, int *offset);

void tomato_exports_load(struct tomato_exports *pes);

unsigned int encode_buffer(struct tomato_exports *pes,
	unsigned char *p, unsigned int bufferlength, unsigned int offset,
	unsigned char *buffer, unsigned int buffersize, unsigned char command,
	const unsigned char *key, unsigned int keylength, unsigned char flags);
unsigned char *decode_buffer(struct tomato_exports *pes, struct xypagebuffer *pb,
	unsigned char *p, unsigned int bufferlength, unsigned int *used, unsigned int *length,
	const unsigned char *key, unsigned int keylength, unsigned char *command);
//---------------------------------------------------------------------------
#endif