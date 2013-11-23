cmake_minimum_required(VERSION 2.8)

set(PROFILER_SRC_DIR
    "${TOOLS_SRC_ROOT}/profiler"
)


file(GLOB_RECURSE PROFILER_SRC_FILES 
    "${PROFILER_SRC_DIR}/src/*.cpp"
    "${PROFILER_SRC_DIR}/src/*.cxx"
    "${PROFILER_SRC_DIR}/src/*.c"
    "${PROFILER_SRC_DIR}/src/*.h"
)

if (MSVC)
    add_definitions(/wd"4996") # POSIX namings
	add_definitions(/wd"4200") # Zero sized array
endif()

#set_source_files_properties(${HEART_NON_UNITY_FILES} PROPERTIES HEADER_FILE_ONLY 1)
source_group(source FILES ${PROFILER_SRC_DIR})

if (NOT ${BUILD_64_BIT} MATCHES 1)
	set(PLATTYPE x86)
else()
	set(PLATTYPE x64)
endif()


#include_directories(${ENET_INCLUDE_DIR})
#link_directories(${MATERIAL_BUILDER_LIB_DIRS})

add_executable(profiler ${PROFILER_SRC_FILES})
set_property(TARGET profiler PROPERTY DEBUG_POSTFIX _d)
target_link_libraries(profiler dbghelp)

ADD_CUSTOM_COMMAND(
	TARGET profiler
    POST_BUILD
	COMMAND ROBOCOPY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/profiler" *.exe *.pdb ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=00
)