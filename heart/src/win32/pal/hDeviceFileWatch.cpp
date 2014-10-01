/********************************************************************

    filename:   hDeviceFileWatch.cpp  
    
    Copyright (c) 16:10:2013 James Moran
    
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

#include "base/hDeviceFilewatch.h"
#include <winsock2.h>
#include <windows.h>
#include "base/hUTF8.h"
#include "base/hDeviceFileSystem.h"
#include "base/hStringUtil.h"
#include "base/hMemory.h"

namespace Heart
{
    struct hdFilewatch
    {
        hdFilewatch() 
            : watchHandle_(INVALID_HANDLE_VALUE)
            , waitHandle_(INVALID_HANDLE_VALUE)
            , dirHandle_(INVALID_HANDLE_VALUE)
            , pathLen_(0)
            , path_(nullptr)
        {}

        HANDLE                   watchHandle_;     
        HANDLE                   waitHandle_; // for callback from win32
        HANDLE                   dirHandle_;
        hUint                    watchFlags_;
        hdFilewatchEventCallback callback_;
        hUint                    pathLen_;
        hChar*                   path_;
        hUint                    nonSysPathLen_;
        hChar*                   nonSysPath_;
    };

    DWORD hdBuildFilter(hUint validevents) {
        DWORD flags=0;
        if (validevents&hdFilewatchEvents_AddRemove) {
            flags |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
        }
        if (validevents&hdFilewatchEvents_FileModified) {
            flags |= FILE_NOTIFY_CHANGE_LAST_WRITE;
        }
        return flags;
    }

    hdFilewatchEvents hdBuildEvent(DWORD action) {
        switch(action) {
        case FILE_ACTION_ADDED           : return hdFilewatchEvents_Added;
        case FILE_ACTION_REMOVED         : return hdFilewatchEvents_Removed;
        case FILE_ACTION_MODIFIED        : return hdFilewatchEvents_Modified;
        case FILE_ACTION_RENAMED_OLD_NAME: return hdFilewatchEvents_Rename;
        case FILE_ACTION_RENAMED_NEW_NAME: return hdFilewatchEvents_Rename;
        }
        return hdFilewatchEvents_Unknown;
    }

    VOID CALLBACK hdPrivateFilewatchCallback(PVOID user, BOOLEAN /*timerorwaitfired*/) {
        hdFilewatch* filewatch=(hdFilewatch*)user;
        hByte workBuffer[1024*16];
        hChar filepathbuffer[1024];
        hBool subtreescan=FALSE;//(filewatch->watchFlags_&hdFilewatchEvents_RecursiveWatch)==hdFilewatchEvents_RecursiveWatch;
        DWORD returnedbytes;
        DWORD filter=hdBuildFilter(filewatch->watchFlags_);
        filter|=FILE_NOTIFY_CHANGE_CREATION|FILE_NOTIFY_CHANGE_SIZE;
        filter|=FILE_NOTIFY_CHANGE_FILE_NAME  ;
        filter|=FILE_NOTIFY_CHANGE_DIR_NAME   ;
        filter|=FILE_NOTIFY_CHANGE_ATTRIBUTES ;
        filter|=FILE_NOTIFY_CHANGE_SIZE       ;
        filter|=FILE_NOTIFY_CHANGE_LAST_WRITE ;
        filter|=FILE_NOTIFY_CHANGE_LAST_ACCESS;
        filter|=FILE_NOTIFY_CHANGE_CREATION   ;
        filter|=FILE_NOTIFY_CHANGE_SECURITY   ;
        //synchronous call but should be on a separate thread so doesn't matter too much
        BOOL ok=ReadDirectoryChangesW(filewatch->watchHandle_, workBuffer, sizeof(workBuffer), subtreescan, filter, &returnedbytes, hNullptr, hNullptr);
        if (ok && returnedbytes > 0) { // How to handle this? Do we restart the search? For the time being we just ignore the problem!
            FILE_NOTIFY_INFORMATION* info=(FILE_NOTIFY_INFORMATION*)workBuffer;
            for (;;) {
                hUint written=hUTF8::encodeFromUnicodeString((hUTF8::Unicode*)info->FileName, info->FileNameLength/sizeof(hUTF8::Unicode), filepathbuffer, sizeof(filepathbuffer)-1);
                filepathbuffer[written]=0;
                filewatch->callback_(filewatch->nonSysPath_, filepathbuffer, hdBuildEvent(info->Action));
                if (info->NextEntryOffset == 0) {
                    break;
                }
                info=(FILE_NOTIFY_INFORMATION*)(((hByte*)info)+info->NextEntryOffset);
            }
        }
        //reissue the search
        ok = FindNextChangeNotification(filewatch->watchHandle_);
        if (!ok) { // can fail on exit
            return;
        }
        ok=RegisterWaitForSingleObject(&filewatch->waitHandle_, filewatch->watchHandle_, hdPrivateFilewatchCallback, filewatch, INFINITE, WT_EXECUTEDEFAULT|WT_EXECUTEONLYONCE);
        if (!ok) { // can fail on exit
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdFilewatchHandle hdBeginFilewatch(const hChar* path, hUint validevents, hdFilewatchEventCallback callback) {
        hUint syslen=hdGetSystemPathSize(path)+hStrLen(path);
        hChar* syspath=(hChar*)hAlloca(syslen+1);
        hdGetSystemPath(path, syspath, syslen+1);
        hUint pathlen=hStrLen(path);
        DWORD flags=hdBuildFilter(validevents);
        hBool subtreescan=FALSE;//(validevents&hdFilewatchEvents_RecursiveWatch)==hdFilewatchEvents_RecursiveWatch;
        HANDLE handle=FindFirstChangeNotification(syspath, subtreescan, flags);
        if (handle==INVALID_HANDLE_VALUE) {
            return (hdFilewatchHandle)hNullptr;
        }

        //this handle is needed to read the changes, the prev handle just wake the listening thread.
//         HANDLE dirhandle=CreateFile(
//             syspath, 
//             FILE_LIST_DIRECTORY|FILE_READ_ATTRIBUTES|FILE_READ_DATA|FILE_TRAVERSE, 
//             FILE_SHARE_READ,
//             hNullptr,
//             OPEN_EXISTING,
//             FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS,
//             hNullptr);
//         if (dirhandle==INVALID_HANDLE_VALUE) {
//             return (hdFilewatchHandle)hNullptr;
//         }

        hdFilewatch* filewatch=new (hMalloc(sizeof(hdFilewatch)+syslen+pathlen+2)) hdFilewatch(); //+1 for NULL string term
        filewatch->path_=(hChar*)(filewatch+1);
        filewatch->pathLen_=syslen;
        filewatch->nonSysPath_=filewatch->path_+filewatch->pathLen_+1;
        filewatch->nonSysPathLen_=pathlen;
        filewatch->watchHandle_=handle;
//        filewatch->dirHandle_=dirhandle;
        filewatch->callback_=callback;
        filewatch->watchFlags_=validevents;
        hStrCopy(filewatch->path_, filewatch->pathLen_+1, syspath);
        hStrCopy(filewatch->nonSysPath_, filewatch->nonSysPathLen_+1, path);

        //setup the callback wait.
        if (RegisterWaitForSingleObject(&filewatch->waitHandle_, filewatch->watchHandle_, hdPrivateFilewatchCallback, filewatch, INFINITE, WT_EXECUTEDEFAULT|WT_EXECUTEONLYONCE) == FALSE) {
            hdEndFilewatch((hdFilewatchHandle)filewatch);
            return (hdFilewatchHandle)hNullptr;
        }

        return (hdFilewatchHandle)filewatch;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdEndFilewatch(hdFilewatchHandle filewatchhandle) {
        hdFilewatch* filewatch=(hdFilewatch*)filewatchhandle;
        if (!filewatch) {
            return;
        }
        hcAssert(filewatch);
        if (filewatch->watchHandle_!=INVALID_HANDLE_VALUE) {
            CloseHandle(filewatch->watchHandle_);
            filewatch->watchHandle_=INVALID_HANDLE_VALUE;
        }
        if (filewatch->dirHandle_) {
            CloseHandle(filewatch->dirHandle_);
            filewatch->dirHandle_=INVALID_HANDLE_VALUE;
        }
        delete filewatch; filewatch = nullptr;
    }


}