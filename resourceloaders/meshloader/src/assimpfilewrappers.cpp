/********************************************************************

    filename: 	assimpfilewrappers.cpp	
    
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

#include "assimpfilewrappers.h"
#include "Heart.h"

struct FileIOUser
{
    FileIOUser(Heart::hMemoryHeapBase* heap) 
        : openHandles(heap)
    {
    }
    Heart::hResourceMemAlloc* mem;
    Heart::hIBuiltDataCache* fileaccess;
    Heart::hVector< aiFile* > openHandles;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void mlaiInitFileIO(aiFileIO* fileio, Heart::hResourceMemAlloc* mem, Heart::hIBuiltDataCache* fileaccess)
{
    FileIOUser* ud = (FileIOUser*)mem->tempHeap_->alloc(sizeof(FileIOUser));
    hPLACEMENT_NEW(ud) FileIOUser(mem->tempHeap_);
    ud->mem = mem;
    ud->fileaccess = fileaccess;
    fileio->UserData = (aiUserData)ud;
    fileio->OpenProc = mlaiFileOpenProc;
    fileio->CloseProc = mlaiFileCloseProc;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void mlaiDestroyFileIO(aiFileIO* fileio)
{
    FileIOUser* ud = (FileIOUser*)fileio->UserData;
    for (hUint i = 0; i < ud->openHandles.GetSize(); ++i) {
        if (ud->openHandles[i] != NULL) {
            ud->fileaccess->CloseFile((Heart::hIDataCacheFile*)ud->openHandles[i]->UserData);
            ud->mem->tempHeap_->release(ud->openHandles[i]);
        }
    }
    ud->~FileIOUser();
    ud->mem->tempHeap_->release(ud);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

aiFile* mlaiFileOpenProc(aiFileIO* fileio, const char* path, const char* rw)
{
    FileIOUser* ud = (FileIOUser*)fileio->UserData;
    Heart::hResourceMemAlloc* mem = ud->mem;
    Heart::hIDataCacheFile* bcfile = ud->fileaccess->OpenFile(path);
    aiFile* file = (aiFile*)mem->tempHeap_->alloc(sizeof(aiFile));
    file->ReadProc = mlaiFileReadProc;
    file->WriteProc = mlaiFileWriteProc;
    file->SeekProc = mlaiFileSeek;
    file->TellProc = mlaiFileTellProc;
    file->FileSizeProc = mlaiFileSizeProc;
    file->FlushProc = mlaiFileFlushProc;
    file->UserData = (aiUserData)bcfile;

    for (hUint i = 0; i < ud->openHandles.GetSize(); ++i) {
        if (ud->openHandles[i] == NULL) {
            ud->openHandles[i] = file;
            return file;
        }
    }

    //Not added, extend array
    ud->openHandles.PushBack(file);
    return file;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void mlaiFileCloseProc(aiFileIO* fileio, C_STRUCT aiFile* file)
{
    FileIOUser* ud = (FileIOUser*)fileio->UserData;
    Heart::hResourceMemAlloc* mem = ud->mem;
    Heart::hIDataCacheFile* bcfile = (Heart::hIDataCacheFile*)file->UserData;

    for (hUint i = 0; i < ud->openHandles.GetSize(); ++i) {
        if (ud->openHandles[i] == file) {
            ud->openHandles[i] = NULL;
            break;
        }
    }

    ud->fileaccess->CloseFile(bcfile);
    mem->tempHeap_->release(file);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

size_t mlaiFileWriteProc(aiFile*, const char*, size_t, size_t )
{
    //Does nothing
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

size_t mlaiFileReadProc(aiFile* file, char* buf, size_t size,size_t count)
{
    Heart::hIDataCacheFile* bcfile = (Heart::hIDataCacheFile*)file->UserData;    
    return bcfile->Read(buf, size*count);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

size_t mlaiFileTellProc(aiFile* file)
{
    Heart::hIDataCacheFile* bcfile = (Heart::hIDataCacheFile*)file->UserData;
    return bcfile->Tell();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

size_t mlaiFileSizeProc(aiFile* file)
{
    Heart::hIDataCacheFile* bcfile = (Heart::hIDataCacheFile*)file->UserData;
    return bcfile->Lenght();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void mlaiFileFlushProc(aiFile*)
{
    //?
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

aiReturn mlaiFileSeek(aiFile* file, size_t of, aiOrigin o)
{
    Heart::hIDataCacheFile* bcfile = (Heart::hIDataCacheFile*)file->UserData;
    Heart::hIDataCacheFile::SeekOffset ho;
    if (o == aiOrigin_SET) ho = Heart::hIDataCacheFile::BEGIN;
    if (o == aiOrigin_CUR) ho = Heart::hIDataCacheFile::CURRENT;
    if (o == aiOrigin_END) ho = Heart::hIDataCacheFile::END;

    return bcfile->Seek(of, ho) == of ? aiReturn_SUCCESS : aiReturn_FAILURE;
}
