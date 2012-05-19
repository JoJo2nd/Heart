/********************************************************************

	filename: 	libtga.h	
	
	Copyright (c) 11:5:2012 James Moran
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	
	3. This notice may not be removed or altered from any source
	distribution.

*********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif//

#include <stdlib.h>
#include <stdio.h>

typedef unsigned char   tga_byte;
typedef short           tga_int16;

#define LTGA_SEEK_SET (SEEK_SET)
#define LTGA_SEEK_CUR (SEEK_CUR)
#define LTGA_SEEK_END (SEEK_END)

#define LTGA_CALLBACK   __cdecl
#define LTGA_API        __cdecl

//Error codes
#define LTGA_ERR_NOMEM          (-1)
#define LTGA_ERR_NOSUPPORT      (-2)

typedef size_t (LTGA_CALLBACK *TGAWriteBytes_t)(void* src, size_t write, void* user);
typedef size_t (LTGA_CALLBACK *TGAReadBytes_t)(void* dst, size_t read, void* user);
typedef size_t (LTGA_CALLBACK *TGASeekBytes_t)(size_t offset, int from, void* user);
typedef void* (LTGA_CALLBACK *TGAmalloc_t)(size_t size, void* user);
typedef void (LTGA_CALLBACK *TGAfree_t)(void* ptr, void* user);

typedef struct TGADataReaderFuncs
{
    TGAWriteBytes_t writer_;
    TGAReadBytes_t  reader_;
    TGASeekBytes_t  seek_;
    TGAmalloc_t     mallocFunc_;
    TGAfree_t       freeFunc_;
} TGADataReaderFuncs_t;

typedef struct TGAImageInfo
{
    tga_int16   width_;
    tga_int16   height_;
    tga_byte    bbp_;
    int         rleCompressed_;
    tga_byte    colourWidth_;
} TGAImageInfo_t;

typedef struct TGAImage TGAImage_t;

TGAImage_t* LTGA_API                TGACreateImage(TGADataReaderFuncs_t funcs, void* user);
void        LTGA_API                TGADestroyImage(TGAImage_t* image);
void        LTGA_API                TGAGetImageInfo(TGAImage_t* image, TGAImageInfo_t* info);
int         LTGA_API                TGASetImageInfo(TGAImage_t* image, const TGAImageInfo_t* info);
int         LTGA_API                TGASetImageData(tga_byte* src, size_t size);
int         LTGA_API                TGAReadImage(TGAImage_t* image);
tga_byte*   LTGA_API                TGAGetImageData(TGAImage_t* image);

#ifdef __cplusplus
};//extern "C" 
#endif//
