/********************************************************************
    
    Copyright (c) 9:4:2014 James Moran
    
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

#include <string>

namespace build {
namespace ErrorCode {
enum Type {
    LuaCompileError = -1,
    BuildError = -2,
    AlreadyBuilding = -3,
    NoReportAvailable = -4, // available 
    
    OK = 0,
    AlreadyInit = 1,
};
}

typedef ErrorCode::Type ErrorCodeEnum;

struct Report
{
    std::string buildOutput;
};

typedef void (*BuildLogHandler)(const char* str, uint str_len, void* ptr);

ErrorCodeEnum beginDataBuild(const std::string& build_path, const std::string& output_path, BuildLogHandler msg_handler, void* user_ptr);
bool      isBuildingData();
bool      dataBuildComplete();
ErrorCodeEnum getLastBuildReport(Report* out_report);

}