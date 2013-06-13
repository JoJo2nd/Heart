cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system serialization)
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
    "${VIEWER_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	"${EXTERNAL_SRC_ROOT}/tinyxml2"
	"${EXTERNAL_SRC_ROOT}/freeimage/Dist"
	"${assimp_INCLUDE_DIR}"
	${LIBHEART_INCLUDES}
	${nvtt_INCLUDE_DIR}
)

set( VIEWER_HDR_FILES 
    "${VIEWER_SRC_DIR}/include/consolelog.h"
    "${VIEWER_SRC_DIR}/include/menuidprovider.h"
    "${VIEWER_SRC_DIR}/include/modulesystem.h"
    "${VIEWER_SRC_DIR}/include/uieventsids.h"
    "${VIEWER_SRC_DIR}/include/viewermain.h"
)
set( VIEWER_SRC_FILES
    "${VIEWER_SRC_DIR}/src/consolelog.cpp"
    "${VIEWER_SRC_DIR}/src/modulesystem.cpp"
    "${VIEWER_SRC_DIR}/src/signals.cpp"
    "${VIEWER_SRC_DIR}/src/viewermain.cpp"
)
set(TEXTURE_SRC_FILES
    "${VIEWER_SRC_DIR}/include/texture/texture_data_store.h"
    "${VIEWER_SRC_DIR}/include/texture/texture_management_window.h"
	"${VIEWER_SRC_DIR}/include/texture/texture_data_store_export_window.h"
    "${VIEWER_SRC_DIR}/include/texture/texture_module.h"
	"${VIEWER_SRC_DIR}/src/texture/texture_data_store.cpp"
	"${VIEWER_SRC_DIR}/src/texture/texture_management_window.cpp"
	"${VIEWER_SRC_DIR}/src/texture/texture_data_store_export_window.cpp"
	"${VIEWER_SRC_DIR}/src/texture/texture_module.cpp"
)
set(MESH_SRC_FILES
    "${VIEWER_SRC_DIR}/include/mesh/mesh_container.h"
	"${VIEWER_SRC_DIR}/include/mesh/mesh_explorer_window.h"
    "${VIEWER_SRC_DIR}/include/mesh/mesh_lod_level.h"
	"${VIEWER_SRC_DIR}/include/mesh/mesh_module.h"
	"${VIEWER_SRC_DIR}/include/mesh/plugin_defines.h"
	"${VIEWER_SRC_DIR}/include/mesh/xml_helpers.h"
	"${VIEWER_SRC_DIR}/src/mesh/mesh_container.cpp"
	"${VIEWER_SRC_DIR}/src/mesh/mesh_explorer_window.cpp"
	"${VIEWER_SRC_DIR}/src/mesh/mesh_lod_level.cpp"
	"${VIEWER_SRC_DIR}/src/mesh/mesh_module.cpp"
)
set(COMMON_HDR_FILES
    "${VIEWER_SRC_DIR}/include/common/action_stack.h"
    "${VIEWER_SRC_DIR}/include/common/module.h"
    "${VIEWER_SRC_DIR}/include/common/status_codes.h"
	"${EXTERNAL_SRC_ROOT}/tinyxml2/tinyxml2.h"
)
set(COMMON_SRC_FILES
    "${VIEWER_SRC_DIR}/src/common/action_stack.cpp"
	"${EXTERNAL_SRC_ROOT}/tinyxml2/tinyxml2.cpp"
)
set( VIEWER_PRECOMPILED_SRC_FILES
	"${VIEWER_SRC_DIR}/include/precompiled.h" 
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
	"${VIEWER_SRC_DIR}/include/precompiled.h" 
	"${VIEWER_SRC_DIR}/src/precompiled.cpp" 
	"${ALL_SRC_FILES}"
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
target_link_libraries(viewer heart freeimage nvtt assimp ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})

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