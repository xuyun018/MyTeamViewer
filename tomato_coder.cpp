#include "tomato_coder.h"

#include "zlib1211/zlib.h"
//---------------------------------------------------------------------------
unsigned int ipv4_2_buffer(const SOCKADDR_IN *psai, unsigned char *buffer)
{
	unsigned int result = 1;

	buffer[result++] = psai->sin_family;
	buffer[result++] = psai->sin_port;
	buffer[result++] = psai->sin_port >> 8;
	buffer[result++] = psai->sin_addr.S_un.S_un_b.s_b1;
	buffer[result++] = psai->sin_addr.S_un.S_un_b.s_b2;
	buffer[result++] = psai->sin_addr.S_un.S_un_b.s_b3;
	buffer[result++] = psai->sin_addr.S_un.S_un_b.s_b4;
	buffer[0] = result;

	return(result);
}
unsigned int buffer_2_ipv4(const unsigned char *buffer, SOCKADDR_IN *psai)
{
	unsigned int result = 1;

	memset(psai, 0, sizeof(SOCKADDR_IN));

	psai->sin_family = buffer[result++];
	psai->sin_port = buffer[result++];
	psai->sin_port |= buffer[result++] << 8;
	psai->sin_addr.S_un.S_un_b.s_b1 = buffer[result++];
	psai->sin_addr.S_un.S_un_b.s_b2 = buffer[result++];
	psai->sin_addr.S_un.S_un_b.s_b3 = buffer[result++];
	psai->sin_addr.S_un.S_un_b.s_b4 = buffer[result++];
	return(result);
}
unsigned int ipv6_2_buffer(const SOCKADDR_IN6 *psai, unsigned char *buffer)
{
	unsigned int i;
	unsigned int result = 1;

	// 可能不对, 还有别的没有写

	buffer[result++] = psai->sin6_family;
	buffer[result++] = psai->sin6_port;
	buffer[result++] = psai->sin6_port >> 8;
	for (i = 0; i < sizeof(psai->sin6_addr.u.Byte); i++)
	{
		buffer[result++] = psai->sin6_addr.u.Byte[i];
	}
	buffer[0] = result;
	return(result);
}
unsigned int buffer_2_ipv6(const unsigned char *buffer, SOCKADDR_IN6 *psai)
{
	unsigned int i;
	unsigned int result = 1;

	memset(psai, 0, sizeof(SOCKADDR_IN6));

	psai->sin6_family = buffer[result++];
	psai->sin6_port = buffer[result++];
	psai->sin6_port |= buffer[result++] << 8;
	for (i = 0; i < sizeof(psai->sin6_addr.u.Byte); i++)
	{
		psai->sin6_addr.u.Byte[i] = buffer[result++];
	}
	return(result);
}
//---------------------------------------------------------------------------
unsigned char *writevalue2(unsigned char *buffer, unsigned int value)
{
	(*buffer++) = (value >> 8);
	(*buffer++) = value;
	return(buffer);
}
unsigned int readvalue2(const unsigned char *buffer)
{
	unsigned int result = 0;

	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	return(result);
}
unsigned char *writevalue3(unsigned char *buffer, unsigned int value)
{
	(*buffer++) = (value >> 16);
	(*buffer++) = (value >> 8);
	(*buffer++) = value;
	return(buffer);
}
unsigned int readvalue3(const unsigned char *buffer)
{
	unsigned int result = 0;

	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	return(result);
}
unsigned char *writevalue4(unsigned char *buffer, unsigned int value)
{
	(*buffer++) = (value >> 24);
	(*buffer++) = (value >> 16);
	(*buffer++) = (value >> 8);
	(*buffer++) = value;
	return(buffer);
}
unsigned int readvalue4(const unsigned char *buffer)
{
	unsigned int result = 0;

	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	return(result);
}
unsigned char *writevalue8(unsigned char *buffer, unsigned long long value)
{
	(*buffer++) = (value >> 56);
	(*buffer++) = (value >> 48);
	(*buffer++) = (value >> 40);
	(*buffer++) = (value >> 32);
	(*buffer++) = (value >> 24);
	(*buffer++) = (value >> 16);
	(*buffer++) = (value >> 8);
	(*buffer++) = value;
	return(buffer);
}
unsigned long long readvalue8(const unsigned char *buffer)
{
	unsigned long long result = 0;

	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	result = (result << 8) + (*buffer++);
	return(result);
}

int _ll2wcs(__int64 value, wchar_t *cs, int n, int maximum)
{
	int i = 0;
	int j = 0;
	__int64 v;

	if (value < 0)
	{
		if (maximum == 0)
		{
			if (i + 1 < n || cs != NULL && n == 0)
			{
				cs[i] = L'-';
			}
			i++;
		}

		value = -value;
	}
	v = value;
	do
	{
		v /= 10;

		j++;
	} while (v > 0);
	j += i;
	if (j < maximum)
	{
		i = maximum - j;
		if (i < n || cs != NULL && n == 0)
		{
			for (j = 0; j < i; j++)
			{
				cs[j] = L'0';
			}
		}
		j = maximum;
	}
	v = j;
	if (j < n || cs != NULL && n == 0)
	{
		cs[j] = L'\0';
	}
	while (j > i)
	{
		j--;

		if (j + 1 < n || cs != NULL && n == 0)
		{
			cs[j] = L'0' + value % 10;
		}

		value /= 10;
	}
	return((int)v);
}
__int64 _wcs2ll(const wchar_t *cs, int n, int *offset)
{
	__int64 result = 0;
	int i = 0;
	int j = 0;
	wchar_t ch;

	if (cs[i] == L'-')
	{
		j++;

		i++;
	}
	while ((n == 0 || i < n) && (ch = cs[i]) != L'\0')
	{
		if (ch >= L'0' && ch <= L'9')
		{
			result *= 10;
			result += ch - L'0';
		}
		else
		{
			break;
		}

		i++;
	}
	if (j > 0)//负数
	{
		if (j == i)//0
		{
			i = 0;
		}
		else
		{
			result = -result;
		}
	}
	if (offset != NULL)
	{
		*offset = i;
	}
	return(result);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void *xyalloc(struct tomato_exports *pes, unsigned int flags, unsigned int size)
{
	return(HeapAlloc(pes->heap, flags, size));
}
int xyfree(struct tomato_exports *pes, unsigned int flags, void *data)
{
	return(HeapFree(pes->heap, flags, data));
}

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
	struct tomato_exports *pes = (struct tomato_exports *)((struct zlib *)opaque)->p;

	return(pes->p_alloc(pes, 0, items * size));
}
void zlib_free(void *opaque, voidpf ptr)
{
	struct tomato_exports *pes = (struct tomato_exports *)((struct zlib *)opaque)->p;

	pes->p_free(pes, 0, ptr);
}

void tomato_exports_load(struct tomato_exports *pes)
{
	pes->heap = GetProcessHeap();

	pes->p_alloc = xyalloc;
	pes->p_free = xyfree;

	pes->p_zlib_alloc = zlib_alloc;
	pes->p_zlib_free = zlib_free;

	pes->p_zlib_encoder_initialize = zlib_encoder_initialize;
	pes->p_zlib_decoder_initialize = zlib_decoder_initialize;

	pes->p_zlib_compressbound = compressBound;

	pes->p_zlib_compress = compress;
	pes->p_zlib_uncompress = uncompress;
}

unsigned int encode_buffer(struct tomato_exports *pes,
	unsigned char *p, unsigned int bufferlength, unsigned int offset,
	unsigned char *buffer, unsigned int buffersize, unsigned char command,
	const unsigned char *key, unsigned int keylength, unsigned char flags)
{
	unsigned long destinationlength;
	unsigned int o = offset;
	unsigned char lastflags;

	lastflags = command << 4;

	o++;
	writevalue3(p + o, bufferlength);
	o += 3;

	if (flags&TOMATO_BUFFER_FLAG_COMPRESS)
	{
		if (bufferlength)
		{
			struct z_encoder pze[1];

			pze->p_alloc = pes->p_zlib_alloc;
			pze->p_free = pes->p_zlib_free;
			pze->p = (void *)pes;

			pes->p_zlib_encoder_initialize(pze);

			destinationlength = buffersize;
			if (pes->p_zlib_compress(pze, buffer, &destinationlength, p + o, bufferlength) == Z_OK && destinationlength > 0 && destinationlength + 4 < bufferlength)
			{
				writevalue3(p + o, destinationlength);
				o += 4;
				memcpy(p + o, buffer, destinationlength);
				bufferlength = destinationlength;

				lastflags |= TOMATO_BUFFER_FLAG_COMPRESS;
			}
		}
	}

	p[offset] = lastflags;

	o += bufferlength;
	o -= offset;

	return(o);
}
unsigned char *decode_buffer(struct tomato_exports *pes, struct xypagebuffer *pb,
	unsigned char *p, unsigned int bufferlength, unsigned int *used, unsigned int *length,
	const unsigned char *key, unsigned int keylength, unsigned char *command)
{
	unsigned char *result = NULL;
	unsigned long destinationlength;
	unsigned short lens[ZLIB_LENS_COUNT];
	unsigned short work[ZLIB_WORK_COUNT];
	unsigned int o = 0;
	unsigned int l0;
	unsigned int l1;
	unsigned char lastflags;

	*used = 0;

	if (o + 4 <= bufferlength)
	{
		lastflags = p[o++];
		l0 = readvalue3(p + o);
		o += 3;

		if (lastflags&TOMATO_BUFFER_FLAG_COMPRESS)
		{
			// 绝对有长度
			if (o + 4 < bufferlength)
			{
				l1 = readvalue3(p + o);
				o += 4;

				if (o + l1 <= bufferlength)
				{
					struct z_decoder pzd[1];

					pzd->p_alloc = pes->p_zlib_alloc;
					pzd->p_free = pes->p_zlib_free;
					pzd->p = (void *)pes;

					pes->p_zlib_decoder_initialize(pzd, lens, work);

					//
					pb->offset = 0;
					xypagebuffer_write(pes, pb, NULL, l0);
					//

					if (pb->buffer1)
					{
						destinationlength = l0;
						if (pes->p_zlib_uncompress(pzd, pb->buffer1, &destinationlength, p + o, l1) == Z_OK  && destinationlength == l0)
						{
							result = pb->buffer1;

							*length = l0;

							*command = lastflags >> 4;
						}
					}

					*used = o + l1;
				}
			}
		}
		else
		{
			if (o + l0 <= bufferlength)
			{
				result = p + o;

				*length = l0;

				*command = lastflags >> 4;

				*used = o + l0;
			}
		}
	}

	return(result);
}
//---------------------------------------------------------------------------