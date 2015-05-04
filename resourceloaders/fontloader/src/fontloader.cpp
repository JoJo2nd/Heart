/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "resource_common.pb.h"
#include "resource_font.pb.h"
#include <fstream>
#include <memory>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#else
#   pragma error ("Unknown platform")
#endif
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"

#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

#if defined PLATFORM_WINDOWS
#   define FB_API __cdecl
#elif PLATFORM_LINUX
#   if BUILD_64_BIT
#       define FB_API
#   else
#       define FB_API __attribute__((cdecl))
#   endif
#else
#   error
#endif

#if defined (PLATFORM_WINDOWS)
#   if defined (font_builder_EXPORTS)
#       define DLL_EXPORT __declspec(dllexport)
#   else
#       define DLL_EXPORT __declspec(dllimport)
#   endif
#else
#   define DLL_EXPORT
#endif

#define luaL_errorthrow(L, fmt, ...) \
    luaL_where(L, 1); \
    lua_pushfstring(L, fmt, ##__VA_ARGS__ ); \
    lua_concat(L, 2); \
    throw std::exception();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int fontCompile(lua_State* L) {
    using namespace Heart;
    
    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);

try {
    std::vector<char> scratchbuffer;
    scratchbuffer.reserve(1024*10);
    const char* ttf_path=lua_tostring(L, 1);
    size_t filesize=minfs_get_file_size(ttf_path);
    scratchbuffer.resize(filesize+1);
    FILE* f = fopen(ttf_path, "rt");
    fread(scratchbuffer.data(), 1, filesize, f);
    fclose(f);
    scratchbuffer[filesize] = 0;

    proto::TTFResource resource_container;
    resource_container.set_ttfdata(scratchbuffer.data(), filesize);

    //write the resource
    const char* outputpath=lua_tostring(L, 4);
    std::ofstream output;
    output.open(outputpath, std::ios_base::out|std::ios_base::binary);
    if (!output.is_open()) {
        luaL_errorthrow(L, "Unable to open output file %s", outputpath);
    }

    google::protobuf::io::OstreamOutputStream filestream(&output);
    google::protobuf::io::CodedOutputStream outputstream(&filestream);
    {
        google::protobuf::io::OstreamOutputStream filestream(&output);
        google::protobuf::io::CodedOutputStream outputstream(&filestream);
        Heart::proto::MessageContainer msgContainer;
        msgContainer.set_type_name(resource_container.GetTypeName());
        msgContainer.set_messagedata(resource_container.SerializeAsString());
        msgContainer.SerializePartialToCodedStream(&outputstream);
    }
    output.close();

    // push table of files files that where included (always empty for ttf fonts)
    lua_newtable(L); // push table of files files that where included by the shader (parse_ctx should have this info)

    return 1;
} catch (std::exception e) {
    return lua_error(L);
}
}

extern "C" {

    int FB_API version(lua_State* L) {
        lua_pushstring(L, "1.0.0");
        return 1;
    }

//Lua entry point calls
DLL_EXPORT int FB_API luaopen_font(lua_State *L) {
    static const luaL_Reg fontlib[] = {
        {"build"      , fontCompile},
        {"version", version},
        {NULL, NULL}
    };
    luaL_newlib(L, fontlib);
    return 1;
}
}

