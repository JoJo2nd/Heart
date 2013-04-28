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

set(VIEWER_API_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( VIEWER_API_INCLUDE_DIRS
    "${VIEWER_API_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	${LIBHEART_INCLUDES}
)

set( VIEWER_API_HDR_FILES 
    "${VIEWER_API_SRC_DIR}/include/action_stack.h"
    "${VIEWER_API_SRC_DIR}/include/module.h"
    "${VIEWER_API_SRC_DIR}/include/package_system.h"
    "${VIEWER_API_SRC_DIR}/include/status_codes.h"
    "${VIEWER_API_SRC_DIR}/include/viewer_api.h"
    "${VIEWER_API_SRC_DIR}/include/viewer_api_config.h"
)
set( VIEWER_API_SRC_FILES
    "${VIEWER_API_SRC_DIR}/src/package_system.cpp"
)
set( VIEWER_API_PRIVATE_FILES
    "${VIEWER_API_SRC_DIR}/include/impl/package_impl.h"
    "${VIEWER_API_SRC_DIR}/include/impl/package_system_impl.h"
    "${VIEWER_API_SRC_DIR}/include/impl/resource_impl.h"
    "${VIEWER_API_SRC_DIR}/include/impl/xml_helpers.h"
    "${VIEWER_API_SRC_DIR}/src/impl/action_stack_impl.cpp"
    "${VIEWER_API_SRC_DIR}/src/impl/package_impl.cpp"
    "${VIEWER_API_SRC_DIR}/src/impl/package_system_impl.cpp"
    "${VIEWER_API_SRC_DIR}/src/impl/resource_impl.cpp"
)
#ADD_MSVC_PRECOMPILED_HEADER(
#	"${VIEWER_API_SRC_DIR}/include/precompiled.h" 
#	"${VIEWER_API_SRC_DIR}/src/precompiled.cpp" 
#	VIEWER_API_PRECOMPILED_SRC_FILES
#)

source_group(include FILES ${VIEWER_API_HDR_FILES})
source_group(source  FILES ${VIEWER_API_SRC_FILES})
#source_group(precompiled FILES ${VIEWER_API_PRECOMPILED_SRC_FILES})
source_group(private FILES ${VIEWER_API_PRIVATE_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${VIEWER_API_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_library(viewer_api ${VIEWER_API_SRC_FILES} ${VIEWER_API_HDR_FILES} ${VIEWER_API_PRIVATE_FILES})
target_link_libraries(viewer_api ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})

ADD_CUSTOM_COMMAND(
	TARGET viewer
    POST_BUILD
	COMMAND ROBOCOPY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.exe ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${wxWidgets_LIB_DIR}" "${CMAKE_SOURCE_DIR}/deploy/viewer" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)