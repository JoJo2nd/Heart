cmake_minimum_required(VERSION 2.8)

ENABLE_PROFILE_IF_SET()

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}")

if (MSVC)
    add_definitions(/wd"4244") # proto buffers spits out a lot of uint64 to int conversion warnings
    add_definitions(/wd"4267") # proto buffers spits out a lot of size_t to int conversion warnings
    add_definitions(/wd"4996") # iterator copy rubbish
	add_definitions(/wd"4018") # signed unsigned compares
    add_definitions(-D_SCL_SECURE_NO_WARNINGS) # A secure MS future
endif()

if (PLATFORM_LINUX)
	add_definitions(-DO_BINARY=0)
endif()

set(PROTO_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../proto/src
)

set( PROTO_INCLUDE_DIRS
    "${PROTO_SRC_DIR}"
)

file(GLOB PROTO_SRC_FILES "${PROTO_SRC_DIR}/*.proto")

file(GLOB PROTO_SRC_FILES "${PROTO_SRC_DIR}/*.proto")
foreach(ADD_PROTO_SRC ${ADDITIONAL_PROTO_SRC_DIRS})
  file(GLOB EXTRA_PROTO_SRC_FILES ${ADD_PROTO_SRC}/*.proto)
  set(PROTO_SRC_FILES ${PROTO_SRC_FILES} ${EXTRA_PROTO_SRC_FILES})
endforeach(ADD_PROTO_SRC)

set(PROTO_LITE_SRC_FILES ${PROTO_SRC_FILES})

include_directories(${PROTOBUF_INCLUDE_DIR} "${EXTERNAL_SRC_ROOT}/lua/src" ${GENERATED_PROTO_FULL_INC_DIRS})

PROTOBUF_GENERATE_LUA_CPP_BINDINGS(ProtoSources ProtoHeaders LuaProtoSources LuaProtoHeaders ${PROTO_SRC_FILES})

source_group(proto_files FILES ${PROTO_SRC_FILES})
source_group(autogen FILES ${ProtoSources} ${ProtoHeaders} ${LiteProtoSources} ${LiteProtoHeaders} ${LuaProtoSources} ${LuaProtoHeaders})

add_library(proto_full ${PROTO_SRC_FILES} ${ProtoSources} ${ProtoHeaders})
target_link_libraries(proto_full debug ${PROTOBUF_LIBRARY_DEBUG} ${PROFILE_LIBS})
target_link_libraries(proto_full optimized ${PROTOBUF_LIBRARY} ${PROFILE_LIBS})

set(PROTO_FILES ${PROTO_SRC_FILES})

add_custom_command(
    OUTPUT "${PROTO_SRC_DIR}/proto_lua.cpp"
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/pbscan"
    ARGS -l ${PROTO_SRC_DIR}/proto_lua.cpp ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    VERBATIM
)
set_source_files_properties("${PROTO_SRC_DIR}/proto_lua.cpp" PROPERTIES GENERATED 1)

file(GLOB PROTO_LUA_SRC_FILES "${PROTO_SRC_DIR}/*.h")

add_library(proto_lua SHARED "${PROTO_SRC_DIR}/proto_lua.cpp" ${PROTO_LUA_SRC_FILES} ${PROTO_SRC_FILES} ${LuaProtoHeaders} ${LuaProtoSources})
target_link_libraries(proto_lua debug ${PROTOBUF_LIBRARY_DEBUG} ${PROFILE_LIBS} lua52 proto_full)
target_link_libraries(proto_lua optimized ${PROTOBUF_LIBRARY} ${PROFILE_LIBS} lua52 proto_full)
set_target_properties(proto_lua PROPERTIES PREFIX "")
add_dependencies(proto_lua pbscan)

if (PLATFORM_LINUX)
    ADD_CUSTOM_COMMAND(
        TARGET proto_lua
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "ln"
        ARGS -fs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/proto_lua.so" proto_lua.so
    )
endif()