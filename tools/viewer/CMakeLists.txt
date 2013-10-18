cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system serialization regex)
#find_package(wxWidgets REQUIRED COMPONENTS core adv aui html media xrc gl qa richtext propgrid)
find_package(wxWidgets REQUIRED COMPONENTS base core adv aui html media xrc gl qa richtext)

add_definitions(-DWXUSINGDLL)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(VIEWER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( VIEWER_INCLUDE_DIRS
    "${VIEWER_SRC_DIR}/src"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	"${EXTERNAL_SRC_ROOT}/tinyxml2"
	"${EXTERNAL_SRC_ROOT}/freeimage/Dist"
	"${assimp_INCLUDE_DIR}"
	${LIBHEART_INCLUDES}
	${nvtt_INCLUDE_DIR}
)

file(GLOB VIEWER_SRC_FILES
    "${VIEWER_SRC_DIR}/src/*.h"
    "${VIEWER_SRC_DIR}/src/*.cpp"
    "${VIEWER_SRC_DIR}/src/modulesystem.cpp"
    "${VIEWER_SRC_DIR}/src/signals.cpp"
    "${VIEWER_SRC_DIR}/src/viewermain.cpp"
)
file(GLOB TEXTURE_SRC_FILES
    "${VIEWER_SRC_DIR}/src/texture/*.h"
    "${VIEWER_SRC_DIR}/src/texture/*.cpp"
)
file(GLOB MESH_SRC_FILES
    "${VIEWER_SRC_DIR}/src/mesh/*.h"
	"${VIEWER_SRC_DIR}/src/mesh/*.cpp"
)
file(GLOB COMMON_SRC_FILES
    "${VIEWER_SRC_DIR}/src/common/*.h"
    "${VIEWER_SRC_DIR}/src/common/*.cpp"
	"${EXTERNAL_SRC_ROOT}/tinyxml2/*.h"
    "${EXTERNAL_SRC_ROOT}/tinyxml2/*.cpp"
)
set( VIEWER_PRECOMPILED_SRC_FILES
	"${VIEWER_SRC_DIR}/src/precompiled.h" 
	"${VIEWER_SRC_DIR}/src/precompiled.cpp" 
)

source_group(include FILES ${VIEWER_HDR_FILES})
source_group(source  FILES ${VIEWER_SRC_FILES})
source_group(texture_module FILES ${TEXTURE_SRC_FILES})
source_group(mesh FILES ${MESH_SRC_FILES})
source_group(common FILES ${COMMON_SRC_FILES} ${COMMON_HDR_FILES})
source_group(precompiled FILES ${VIEWER_PRECOMPILED_SRC_FILES})

set(ALL_SRC_FILES
	${VIEWER_SRC_FILES}
	${TEXTURE_SRC_FILES}
	${MESH_SRC_FILES}
    ${COMMON_SRC_FILES}
)

ADD_MSVC_PRECOMPILED_HEADER(
	"${VIEWER_SRC_DIR}/src/precompiled.h" 
	"${VIEWER_SRC_DIR}/src/precompiled.cpp" 
	VIEWER_PRECOMPILED_SRC_FILES
)

if (MSVC)
	add_definitions(/WX) # Warnings as errors
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

message("assimp lib directory ${assimp_LIB_DIR_FULL}")
include_directories(${VIEWER_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(
	${LIBHEART_LIB_DIRS}
	${assimp_LIB_DIR_FULL}
	${nvtt_LIB_DIR}
	"${EXTERNAL_SRC_ROOT}/freeimage/Dist"
)
if (BUILD_64_BIT)
	link_directories( "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64" )
	set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64")
else()
	link_directories( "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86" )
	set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86")
endif()
add_executable(viewer WIN32 
	${VIEWER_SRC_FILES} 
	${VIEWER_HDR_FILES} 
	${TEXTURE_SRC_FILES}
	${TEXTURE_HDR_FILES}
	${MESH_SRC_FILES}
    ${COMMON_SRC_FILES}
    ${COMMON_HDR_FILES}
	${VIEWER_PRECOMPILED_SRC_FILES}
)
set_property(TARGET viewer PROPERTY DEBUG_POSTFIX _d)
target_link_libraries(viewer crypto freeimage nvtt assimp ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})

ADD_CUSTOM_COMMAND(
	TARGET viewer
    POST_BUILD
	COMMAND ROBOCOPY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.exe ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${wxWidgets_LIB_DIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${freeimage_BIN_DIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${nvtt_BIN_DIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${nvtt_DEPENDENT_BIN_DIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${assimp_BIN_DIR_FULL}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)