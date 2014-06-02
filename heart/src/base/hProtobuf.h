/********************************************************************

    Copyright (c) James Moran
    
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
#ifndef __hPROTOBUF_H__
#define __hPROTOBUF_H__

//////////////////////////////////////////////////////////////////////////
/// Proto buffer classes /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*
    Proto buffers spits out a lot of warnings about 64 bit to 32 bit conversions.
    We disable these warnings just around these sections.
*/
#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#else
#   pragma error ("Unknown platform")
#endif

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

#include "debug_server_common.pb.h"
#include "package.pb.h"
#include "resource_common.pb.h"
#include "resource_shader.pb.h"
#include "resource_texture.pb.h"
#include "resource_material_fx.pb.h"
#include "resource_mesh.pb.h"

#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

#endif