cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)
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
	"${TOOLS_SRC_ROOT}/viewer_api/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	${LIBHEART_INCLUDES}
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
ADD_MSVC_PRECOMPILED_HEADER(
	"${VIEWER_SRC_DIR}/include/precompiled.h" 
	"${VIEWER_SRC_DIR}/src/precompiled.cpp" 
	VIEWER_PRECOMPILED_SRC_FILES
)

source_group(include FILES ${VIEWER_HDR_FILES})
source_group(source  FILES ${VIEWER_SRC_FILES})
source_group(precompiled FILES ${VIEWER_PRECOMPILED_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

include_directories(${VIEWER_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${LIBHEART_LIB_DIRS})
add_executable(viewer WIN32 ${VIEWER_SRC_FILES} ${VIEWER_HDR_FILES} ${VIEWER_PRECOMPILED_SRC_FILES})
set_property(TARGET viewer PROPERTY DEBUG_POSTFIX _d)
target_link_libraries(viewer heart viewer_api ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})

ADD_CUSTOM_COMMAND(
	TARGET viewer
    POST_BUILD
	COMMAND ROBOCOPY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.exe ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${wxWidgets_LIB_DIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)