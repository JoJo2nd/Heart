cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)
find_package(Protobuf REQUIRED)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(TEXTURE_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( TEXTURE_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${TEXTURE_BUILDER_SRC_DIR}/src"
)
file(GLOB TEXTURE_BUILDER_SRC_FILES 
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.h"
    "${TEXTURE_BUILDER_SRC_DIR}/src/*.cpp"
)
set(TEXTURE_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)

source_group(source  FILES ${TEXTURE_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${TEXTURE_BUILDER_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${TEXTURE_BUILDER_LIB_DIRS})
add_library(texture_builder SHARED ${TEXTURE_BUILDER_SRC_FILES})
target_link_libraries(texture_builder lua crypto proto ${Boost_LIBRARIES} ${PLATFORM_LIBS})

set_target_properties(texture_builder PROPERTIES OUTPUT_NAME texture)

ADD_CUSTOM_COMMAND(
	TARGET texture_builder
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" texture "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder/texture"
)