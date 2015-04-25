/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hSysCalls.h"
#include "core/hIFileSystem.h"

namespace Heart {

hBool loadFileSystemInterface(const char* component_name, hFileSystemInterface* out) {
    auto lib = hSysCall::openSharedLib(component_name);
    auto success = true;
    success &= hSysCall::getFunctionAddress(lib, "hrt_initialise_filesystem", out->hrt_initialise_filesystem) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_fileOpComplete", out->hrt_fileOpComplete) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_fileOpWait", out->hrt_fileOpWait) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_fileOpClose", out->hrt_fileOpClose) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_openFile", out->hrt_openFile) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_closeFile", out->hrt_closeFile) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_openDir", out->hrt_openDir) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_readDir", out->hrt_readDir) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_closeDir", out->hrt_closeDir) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_freadAsync", out->hrt_freadAsync) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_fwriteAsync", out->hrt_fwriteAsync) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_fstatAsync", out->hrt_fstatAsync) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_mountPoint", out->hrt_mountPoint) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_unmountPoint", out->hrt_unmountPoint) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_isAbsolutePath", out->hrt_isAbsolutePath) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_getCurrentWorkingDir", out->hrt_getCurrentWorkingDir) != nullptr;
    success &= hSysCall::getFunctionAddress(lib, "hrt_getProcessDirectory", out->hrt_getProcessDirectory) != nullptr;
    if (success) {
        out->hrt_initialise_filesystem();
    }
    return success;
}

}