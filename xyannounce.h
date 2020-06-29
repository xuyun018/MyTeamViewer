
//---------------------------------------------------------------------------
// 程序的总接口，包括导入导出函数, 结构体之类
// 这个单元为比较基础的单元,而且指针长度会受影响
//---------------------------------------------------------------------------
#ifndef XYANNOUNCE_H
#define XYANNOUNCE_H
//---------------------------------------------------------------------------
#define _WIN32_WINNT	0x0600

//#define WIN32_LEAN_AND_MEAN
//---------------------------------------------------------------------------
#include "XYSocket.h"
#include <Ws2tcpip.h>
#include <ws2def.h>
#include <MSWSock.h>
#include <Windows.h>
#include <winternl.h>
#include <Psapi.h>
#include <tchar.h>
#include <stdint.h>

#include "xytypes.h"
//---------------------------------------------------------------------------
#pragma warning(disable : 4996)
#pragma warning(disable : 4005)
//---------------------------------------------------------------------------
// 除了转发的数据别的都不会太长, 数据格式
// 1 字节, 命令
#define TOMATO_SERVER_CONNECTED												0
// 服务端向客户端发送分配的 uid
// 客户端向服务端发送想连接的 uid
#define TOMATO_SERVER_UID													1
// 远程没登录
#define TOMATO_SERVER_OFFLINE												2
#define TOMATO_SERVER_DISCONNECTED											3
// 错误的密码
#define TOMATO_SERVER_BAD													4
// 附加长度
#define TOMATO_SERVER_TRANSMIT												5
// 已经连接成功
#define TOMATO_SERVER_LINK													6
// 相互的 IP
#define TOMATO_SERVER_P2P													7

// 直连数据包格式
#define TOMATO_POINT_PASSWORD												8
// 登录成功
#define TOMATO_POINT_LOGONED												9
// 屏幕数据(包括第一张, 下一张, 控制)
#define TOMATO_POINT_SCREEN													10
//---------------------------------------------------------------------------
#define ZLIB_LENS_COUNT														320
#define ZLIB_WORK_COUNT														288
//---------------------------------------------------------------------------
typedef void *(*t_zlib_alloc)(void* opaque, unsigned int items, unsigned int size);
typedef void (*t_zlib_free)(void* opaque, void* address);
//---------------------------------------------------------------------------
// Zlib 的基结构体
struct zlib
{
	t_zlib_alloc p_alloc;
	t_zlib_free p_free;

	void* p;
};

// Zlib 的编码(压缩)结构体
struct z_encoder
{
	t_zlib_alloc p_alloc;
	t_zlib_free p_free;

	void* p;

	// 初始化缓冲
	unsigned char buffer[2608];
};

// Zlib 的解码(解压缩)结构体
struct z_decoder
{
	t_zlib_alloc p_alloc;
	t_zlib_free p_free;

	void* p;

	// 初始化缓冲
	unsigned int fixed[544];
	unsigned char order[19];
};
//---------------------------------------------------------------------------
// 公用的动态导出函数
struct tomato_exports
{
	void *heap;

	void *(*p_alloc)(struct tomato_exports *pes, unsigned int flags, unsigned int size);
	int (*p_free)(struct tomato_exports *pes, unsigned int flags, void *data);

	// 封装的 zlib 库函数
	t_zlib_alloc p_zlib_alloc;
	t_zlib_free p_zlib_free;

	void (*p_zlib_encoder_initialize)(struct z_encoder* pze);
	void (*p_zlib_decoder_initialize)(struct z_decoder* pzd, unsigned short* lens, unsigned short* work);

	unsigned long (*p_zlib_compressbound)(unsigned long sourceLen);

	int (*p_zlib_compress)(struct z_encoder* pze, unsigned char* dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
	int (*p_zlib_uncompress)(struct z_decoder* pzd, unsigned char* dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif