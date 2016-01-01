/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once


//////////////////////////////////////////////////////////////////////////
/// Proto buffer classes /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*
    Proto buffers spits out a lot of warnings about 64 bit to 32 bit conversions.
    We disable these warnings just around these sections.
*/
#if defined (HEART_PLAT_WINDOWS)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#elif defined (HEART_PLAT_LINUX)
#else
#   error ("Unknown platform")
#endif

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message_lite.h"
/*
#include "debug_server_common.pb.h"
#include "package.pb.h"
#include "resource_common.pb.h"
#include "resource_shader.pb.h"
#include "resource_texture.pb.h"
#include "resource_material_fx.pb.h"
#include "resource_mesh.pb.h"
#include "resource_font.pb.h"
*/
#if defined (HEART_PLAT_WINDOWS)
#   pragma warning(pop)
#elif defined (HEART_PLAT_LINUX)
#else
#endif
