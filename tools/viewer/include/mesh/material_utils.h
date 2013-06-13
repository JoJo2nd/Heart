/********************************************************************

    filename:   material_utils.h  
    
    Copyright (c) 19:1:2013 James Moran
    
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

#ifndef MATERIAL_UTILS_H__
#define MATERIAL_UTILS_H__

#include "heart.h"
#include "boost/filesystem.hpp"
#include <vector>
#include <string>
#include <map>

struct MeshExportResult
{
    bool        exportOK;
    std::string errors;
};
typedef std::map< std::string, std::string > MaterialRemap;

bool extractVertexInputLayoutFromShaderSource(
    const char* source, uint len, const char* profile, 
    const char* entry, const char* filepath, const char* packagepath,
    std::vector< Heart::hInputLayoutDesc >* outLayout, std::string* outErrors);
void extractVertexProgramsFromMaterial(const boost::filesystem::path& xmlpath, const char* matxml, uint len, std::vector< std::string >* vertexProgNames);

#endif // MATERIAL_UTILS_H__