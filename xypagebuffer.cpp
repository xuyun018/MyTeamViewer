#include "xypagebuffer.h"
//---------------------------------------------------------------------------
void xypagebuffer_initialize(struct xypagebuffer *pb, unsigned char *buffer, unsigned int size, unsigned int pagesize)
{
	pb->buffer0 = buffer;
	pb->buffer1 = buffer;
	pb->size0 = size;
	pb->size1 = size;
	pb->offset = 0;

	pb->pagesize = pagesize;
}
void xypagebuffer_uninitialize(struct tomato_exports *pes, struct xypagebuffer *pb)
{
	if (pb->buffer1 != pb->buffer0)
	{
		if (pb->buffer1 != NULL)
		{
			pes->p_free(pes, 0, pb->buffer1);
		}
	}
	// 
	pb->buffer1 = pb->buffer0;
	pb->size1 = pb->size0;
	pb->offset = 0;
}

unsigned int xypagebuffer_write(struct tomato_exports *pes, struct xypagebuffer *pb, const unsigned char *buffer, unsigned int length)
{
	unsigned char *newbuffer;

	if (pb->offset + length > pb->size1)
	{
		pb->size1 = (pb->offset + length + pb->pagesize - 1) / pb->pagesize;
		pb->size1 *= pb->pagesize;
		newbuffer = (unsigned char *)pes->p_alloc(pes, 0, pb->size1);
		if (newbuffer)
		{
			if (pb->offset > 0)
			{
				//psis->p_
				memcpy(newbuffer, pb->buffer1, pb->offset);
			}
		}
		if (pb->buffer1 != pb->buffer0)
		{
			pes->p_free(pes, 0, pb->buffer1);
		}
		pb->buffer1 = newbuffer;
	}
	if (pb->buffer1)
	{
		if (buffer != NULL && length > 0)
		{
			memcpy(pb->buffer1 + pb->offset, buffer, length);
			pb->offset += length;
		}
	}
	else
	{
		pb->offset = 0;
		pb->size1 = 0;

		length = 0;
	}

	return(length);
}
unsigned int xypagebuffer_read(struct xypagebuffer *pb, unsigned char *buffer, unsigned int length)
{
	if (length > pb->offset)
	{
		length = pb->offset;
	}
	if (length > 0)
	{
		if (buffer != NULL)
		{
			//psis->p_
			memcpy(buffer, pb->buffer1, length);
		}
		else
		{
			pb->offset -= length;
			if (pb->offset > 0)
			{
				//psis->p_
				memmove(pb->buffer1, pb->buffer1 + length, pb->offset);
			}
		}
	}
	return(length);
}

unsigned char *xypagebuffer_convergence(struct tomato_exports *pes, struct xypagebuffer *pb)
{
	unsigned char *buffer = NULL;
	unsigned int length;

	length = pb->offset;
	if (length > 0)
	{
		buffer = (unsigned char *)pes->p_alloc(pes, 0, length);
		if (buffer != NULL)
		{
			memcpy(buffer, pb->buffer1, length);

			if (pb->buffer1 != pb->buffer0)
			{
				pes->p_free(pes, 0, pb->buffer1);
			}
			pb->buffer1 = buffer;
		}
	}
	return(buffer);
}
//---------------------------------------------------------------------------