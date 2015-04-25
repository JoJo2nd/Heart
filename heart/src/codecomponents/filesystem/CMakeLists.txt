cmake_minimum_required(VERSION 2.8)

project(HEART)

ENABLE_PROFILE_IF_SET()

add_definitions(-DHEART_SHARED_LIB_EXPORTS=1)

# set project directories
set(heart_BINARY_DIR "${HEART_SOURCE_DIR}/../bin/heart")

# heart include directorys
set( HEART_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    "${LIBHEART_INCLUDES}"
)

set(FILESYSTEM_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# heart include directorys
set( FILESYSTEM_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${HEART_SRC_ROOT}/include/base"
)

file(GLOB FILESYSTEM_SRC_FILES 
    "${HEART_SOURCE_DIR}/src/*.h"
    "${HEART_SOURCE_DIR}/src/*.c"
    "${HEART_SOURCE_DIR}/src/*.cpp"
)

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

source_group(source FILES ${FILESYSTEM_SRC_FILES})

include_directories(${FILESYSTEM_INCLUDE_DIR})
#link_directories(${HEART_LIB_DIRS})
add_library(hddfilesystem SHARED ${FILESYSTEM_SRC_FILES})
target_link_libraries(hddfilesystem heart_base)
