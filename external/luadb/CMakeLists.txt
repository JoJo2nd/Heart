cmake_minimum_required(VERSION 2.8)

if (MSVC)
    add_definitions(/wd"4244") # proto buffers spits out a lot of uint64 to int conversion warnings
    add_definitions(/wd"4267") # proto buffers spits out a lot of size_t to int conversion warnings
    add_definitions(/wd"4996") # iterator copy rubbish
    add_definitions(-D_SCL_SECURE_NO_WARNINGS) # A secure MS future
endif()

set(LUADB_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( LUADB_INCLUDE_DIRS
	"${PROTOBUF_INCLUDE_DIR}"
    "${CMAKE_BINARY_DIR}/external/luadb" # for our generated headers
    "${ENET_INCLUDE_DIR}"
	"${LUA_INCLUDE_DIR}"
    "${GENERATED_PROTO_INC_DIRS}"
    "${LUADB_SRC_DIR}/src"
)
file(GLOB LUADB_SRC_FILES 
    "${LUADB_SRC_DIR}/src/ldb/*.h"
    "${LUADB_SRC_DIR}/src/ldb/*.c"
	"${LUADB_SRC_DIR}/src/ldb/*.cpp"
)

file(GLOB PROTO_SRC_FILES
    "${LUADB_SRC_DIR}/src/ldb/*.proto"
)

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()
if (WIN32)
	set(LDB_PLATFORM_LIBS Winmm)
endif()

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${PROTO_SRC_FILES})

source_group(source FILES ${LUADB_SRC_FILES})
source_group(autogen FILES ${ProtoSources} ${ProtoHeaders})
source_group(proto_files FILES ${PROTO_SRC_FILES})

include_directories(${LUADB_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_library(luadb SHARED ${LUADB_SRC_FILES} ${ProtoSources} ${ProtoHeaders} ${PROTO_SRC_FILES})
target_link_libraries(luadb lua52 proto enet ${LDB_PLATFORM_LIBS} ${Boost_LIBRARIES})
set_property(TARGET luadb PROPERTY DEBUG_POSTFIX)

ADD_CUSTOM_COMMAND(
	TARGET luadb
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" luadb "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)