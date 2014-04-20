cmake_minimum_required(VERSION 2.8)

ENABLE_PROFILE_IF_SET()

if (MSVC)
    add_definitions(/wd"4244") # proto buffers spits out a lot of uint64 to int conversion warnings
    add_definitions(/wd"4267") # proto buffers spits out a lot of size_t to int conversion warnings
    add_definitions(/wd"4996") # iterator copy rubbish
    add_definitions(-D_SCL_SECURE_NO_WARNINGS) # A secure MS future
endif()

set(PROTO_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../proto/src
)

set( PROTO_INCLUDE_DIRS
    "${PROTO_SRC_DIR}"
)

file(GLOB PROTO_SRC_FILES "${PROTO_SRC_DIR}/*.proto")

set(PROTO_LITE_SRC_FILES ${PROTO_SRC_FILES})

include_directories(${PROTOBUF_INCLUDE_DIR} "${EXTERNAL_SRC_ROOT}/lua/src" ${GENERATED_PROTO_FULL_INC_DIRS})

PROTOBUF_GENERATE_LUA_CPP_BINDINGS(ProtoSources ProtoHeaders LuaProtoSources LuaProtoHeaders ${PROTO_SRC_FILES})

source_group(proto_files FILES ${PROTO_SRC_FILES})
source_group(autogen FILES ${ProtoSources} ${ProtoHeaders} ${LiteProtoSources} ${LiteProtoHeaders} ${LuaProtoSources} ${LuaProtoHeaders})

add_library(proto_full ${PROTO_SRC_FILES} ${ProtoSources} ${ProtoHeaders})
target_link_libraries(proto_full debug ${PROTOBUF_LIBRARY_DEBUG} ${PROFILE_LIBS})
target_link_libraries(proto_full optimized ${PROTOBUF_LIBRARY} ${PROFILE_LIBS})
set_property(TARGET proto_full PROPERTY DEBUG_POSTFIX)

file(GLOB PROTO_LUA_SRC_FILES "${PROTO_SRC_DIR}/proto_lua*.cpp" "${PROTO_SRC_DIR}/proto_lua*.h")
add_library(proto_lua SHARED ${PROTO_LUA_SRC_FILES} ${PROTO_SRC_FILES} ${LuaProtoHeaders} ${LuaProtoSources})
target_link_libraries(proto_lua debug ${PROTOBUF_LIBRARY_DEBUG} ${PROFILE_LIBS} lua52 proto_full)
target_link_libraries(proto_lua optimized ${PROTOBUF_LIBRARY} ${PROFILE_LIBS} lua52 proto_full)
set_property(TARGET proto_lua PROPERTY DEBUG_POSTFIX)

ADD_CUSTOM_COMMAND(
	TARGET proto_lua
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" proto_lua "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)