/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2016 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ifndef ZUTIL_H
#define ZUTIL_H

#  define ZLIB_INTERNAL

#include "zlib.h"

#if defined(STDC) && !defined(Z_SOLO)
#  if !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#endif

#ifdef Z_SOLO
   typedef long ptrdiff_t;  /* guess -- will be caught if guess is wrong */
#endif

typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;

/* (size given to avoid silly warnings with Visual C++) */

#define ERR_RETURN(strm,err) \
  return (strm->msg = ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */

#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#ifdef AMIGA
#  define OS_CODE  1
#endif

#if defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#endif

#ifdef __370__
#  if __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  else
#    define OS_CODE 8
#  endif
#endif

#if defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef __acorn
#  define OS_CODE 13
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
#  define OS_CODE  10
#endif

#ifdef _BEOS_
#  define OS_CODE  16
#endif

#ifdef __TOS_OS400__
#  define OS_CODE 18
#endif

#ifdef __APPLE__
#  define OS_CODE 19
#endif

#if defined(__BORLANDC__) && !defined(MSDOS)
  #pragma warn -8004
  #pragma warn -8008
  #pragma warn -8066
#endif

/* provide prototypes for these when building zlib without LFS */
#if !defined(_WIN32) && \
    (!defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0)
    ZEXTERN uLong ZEXPORT adler32_combine64 OF((uLong, uLong, z_off_t));
    ZEXTERN uLong ZEXPORT crc32_combine64 OF((uLong, uLong, z_off_t));
#endif

        /* common defaults */

#ifndef OS_CODE
#  define OS_CODE  3     /* assume Unix */
#endif

         /* functions */

#if defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#if defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#if defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  else
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#else
   void ZLIB_INTERNAL zmemcpy OF((Bytef* dest, const Bytef* source, uInt len));
   int ZLIB_INTERNAL zmemcmp OF((const Bytef* s1, const Bytef* s2, uInt len));
   void ZLIB_INTERNAL zmemzero OF((Bytef* dest, uInt len));
#endif

#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#endif /* ZUTIL_H */
