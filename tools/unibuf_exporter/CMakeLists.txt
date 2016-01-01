cmake_minimum_required(VERSION 2.8)

if (MSVC)
    add_definitions(/WX) # Warnings as errors
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

set(UB_EXPORTER_LIB_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( UB_EXPORTER_LIB_INCLUDE_DIRS
    "${UB_EXPORTER_LIB_SRC_DIR}/src"
    "${GETOPT_PORT_INCLUDE_DIR}"
    ${MINFS_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

file(GLOB UB_EXPORTER_LIB_SRC_FILES
    "${UB_EXPORTER_LIB_SRC_DIR}/src/*.h"
    "${UB_EXPORTER_LIB_SRC_DIR}/src/*.cpp"
)

source_group(source  FILES ${UB_EXPORTER_LIB_SRC_FILES})

include_directories(${UB_EXPORTER_LIB_INCLUDE_DIRS})

add_library(ub_exporter_lib ${UB_EXPORTER_LIB_SRC_FILES})
target_link_libraries(ub_exporter_lib getopt_port minfs)

file(GLOB UB_EXPORTER_SRC_FILES
    "${UB_EXPORTER_LIB_SRC_DIR}/src/prog/*.h"
    "${UB_EXPORTER_LIB_SRC_DIR}/src/prog/*.cpp"
)

source_group(source FILES ${UB_EXPORTER_SRC_FILES})
include_directories(${UB_EXPORTER_LIB_INCLUDE_DIRS})

add_executable(ub_exporter ${UB_EXPORTER_SRC_FILES})
target_link_libraries(ub_exporter ub_exporter_lib)
