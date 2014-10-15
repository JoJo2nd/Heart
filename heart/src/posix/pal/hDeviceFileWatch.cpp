/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hUTF8.h"
#include "base/hStringUtil.h"
#include "base/hMemory.h"
#include "base/hDeviceFileSystem.h"
#include "base/hDeviceFileWatch.h"
#include "pal/hMutex.h"
#include <sys/inotify.h>

namespace Heart {

    struct hdFilewatch
    {
        hdFilewatch(const hChar* path, hUint mask) 
            : watchFD_(0)
        {
            lazyInit();
            watchFD_ = inotify_add_watch(s_inotify_fd, path, mask);
        }

        ~hdFilewatch() {
            if (watchFD_) {
                inotify_rm_watch(s_inotify_fd, watchFD_);
            }
        }


        static void lazyInit() {
            s_init_mutex.Lock();
            if (s_inotify_fd == 0) {
                s_inotify_fd = inotify_init();
            }
            s_init_mutex.Unlock();
        }

        static int      s_inotify_fd;
        static hMutex   s_init_mutex;
        int             watchFD_;
    };

    int      hdFilewatch::s_inotify_fd = 0;
    hMutex   hdFilewatch::s_init_mutex;

    hdFilewatchHandle hdBeginFilewatch(const hChar* path, hUint validevents, hdFilewatchEventCallback callback) {
        hUint syslen=hdGetSystemPathSize(path)+hStrLen(path);
        hChar* syspath=(hChar*)hAlloca(syslen+1);
        hdGetSystemPath(path, syspath, syslen+1);
        hInt in_mask = 0;

        if ( validevents & hdFilewatchEvents_AddRemove ) {
            in_mask |= IN_CREATE | IN_DELETE | IN_DELETE_SELF;
        }
        if ( validevents & hdFilewatchEvents_FileModified ) {
            in_mask |= IN_MODIFY;
        }
        if ( validevents & hdFilewatchEvents_Added ) {
            in_mask |= IN_CREATE;
        }   
        if ( validevents & hdFilewatchEvents_Removed ) {
            in_mask |= IN_DELETE | IN_DELETE_SELF;
        }
        if ( validevents & hdFilewatchEvents_Modified ) {
            in_mask |= IN_MODIFY;
        }
        if ( validevents & hdFilewatchEvents_Rename ) {
            in_mask |= IN_MOVED_FROM | IN_MOVED_TO;
        }

        hdFilewatch* filewatch = new hdFilewatch(syspath, in_mask);

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

        delete filewatch; filewatch = nullptr;
    }


}