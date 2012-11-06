/********************************************************************

	filename: 	assimpfilewrappers.h	
	
	Copyright (c) 13:10:2012 James Moran
	
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

#pragma once

#ifndef ASSIMPFILEWRAPPERS_H__
#define ASSIMPFILEWRAPPERS_H__

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/cfileio.h"

namespace Heart
{
    struct hResourceMemAlloc;
    class hIBuiltDataCache;
}

// aiFile callbacks
size_t   mlaiFileWriteProc (aiFile*,   const char*, size_t, size_t);
size_t   mlaiFileReadProc  (aiFile*,   char*, size_t,size_t);
size_t   mlaiFileTellProc  (aiFile*);
size_t   mlaiFileSizeProc  (aiFile*);
void     mlaiFileFlushProc (aiFile*);
aiReturn mlaiFileSeek(aiFile*, size_t, aiOrigin);

// aiFileIO callbacks
aiFile* mlaiFileOpenProc  (aiFileIO*, const char*, const char*);
void    mlaiFileCloseProc(aiFileIO*, C_STRUCT aiFile*);

void    mlaiInitFileIO(aiFileIO*, Heart::hResourceMemAlloc*, Heart::hIBuiltDataCache*);
void    mlaiDestroyFileIO(aiFileIO*);

#endif // ASSIMPFILEWRAPPERS_H__