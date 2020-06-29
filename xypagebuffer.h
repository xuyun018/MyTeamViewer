//---------------------------------------------------------------------------
// 内存块管理的基础类
//---------------------------------------------------------------------------
#ifndef XYPAGEBUFFER_H
#define XYPAGEBUFFER_H
//---------------------------------------------------------------------------
#include "xyannounce.h"
//---------------------------------------------------------------------------
struct xypagebuffer
{
	unsigned char *buffer0;
	unsigned char *buffer1;
	unsigned int size0;
	unsigned int size1;
	unsigned int offset;

	unsigned int pagesize;
};
//---------------------------------------------------------------------------
void xypagebuffer_initialize(struct xypagebuffer *pb, unsigned char *buffer, unsigned int size, unsigned int pagesize);
void xypagebuffer_uninitialize(struct tomato_exports *pes, struct xypagebuffer *pb);

unsigned int xypagebuffer_write(struct tomato_exports *pes, struct xypagebuffer *pb, const unsigned char *buffer, unsigned int length);
// 需要读两次才能完成修改offset指针
unsigned int xypagebuffer_read(struct xypagebuffer *pb, unsigned char *buffer, unsigned int length);

unsigned char *xypagebuffer_convergence(struct tomato_exports *pes, struct xypagebuffer *pb);
//---------------------------------------------------------------------------
#endif