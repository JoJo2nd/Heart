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

set(MEMTRACK_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( MEMTRACK_INCLUDE_DIRS
    "${MEMTRACK_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
    "${EXTERNAL_SRC_ROOT}/crypto/include"
    "${ENET_INCLUDE_DIR}"
    ${LIBHEART_INCLUDES}
)

file(GLOB MEMTRACK_HDR_FILES 
    "${MEMTRACK_SRC_DIR}/src/*.h"
)

file(GLOB MEMTRACK_SRC_FILES
    "${MEMTRACK_SRC_DIR}/src/*.cpp"
)
ADD_MSVC_PRECOMPILED_HEADER(
	"${MEMTRACK_SRC_DIR}/src/precompiled.h" 
	"${MEMTRACK_SRC_DIR}/src/precompiled.cpp" 
	MEMTRACK_PRECOMPILED_SRC_FILES
)

source_group(include FILES ${MEMTRACK_HDR_FILES})
source_group(source  FILES ${MEMTRACK_SRC_FILES})
source_group(precompiled FILES ${MEMTRACK_PRECOMPILED_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${MEMTRACK_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_executable(mem_track WIN32 ${MEMTRACK_SRC_FILES} ${MEMTRACK_HDR_FILES} ${MEMTRACK_PRECOMPILED_SRC_FILES})
set_property(TARGET mem_track PROPERTY DEBUG_POSTFIX _d)
target_link_libraries(mem_track enet crypto proto ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})

ADD_CUSTOM_COMMAND(
	TARGET mem_track
    POST_BUILD
	COMMAND ROBOCOPY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/mem_tracker" *.exe ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${wxWidgets_LIB_DIR}" "${CMAKE_SOURCE_DIR}/deploy/mem_tracker" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)