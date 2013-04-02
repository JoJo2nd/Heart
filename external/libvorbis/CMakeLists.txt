cmake_minimum_required(VERSION 2.8)

if (MSVC)
    add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    add_definitions(/wd"4305")
endif()

set(LIBVORBIS_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LIBVORBIS_INCLUDE_DIRS
    "${LIBVORBIS_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/libogg/include"
)

set( LIBVORBIS_HDR_FILES
    "${LIBVORBIS_SRC_DIR}/include/vorbis/codec.h"
    "${LIBVORBIS_SRC_DIR}/include/vorbis/vorbisenc.h"
)

set( LIBVORBIS_SRC_FILES
    "${LIBVORBIS_SRC_DIR}/lib/analysis.c"
    "${LIBVORBIS_SRC_DIR}/lib/barkmel.c"
    "${LIBVORBIS_SRC_DIR}/lib/bitrate.c"
    "${LIBVORBIS_SRC_DIR}/lib/block.c"
    "${LIBVORBIS_SRC_DIR}/lib/codebook.c"
    "${LIBVORBIS_SRC_DIR}/lib/envelope.c"
    "${LIBVORBIS_SRC_DIR}/lib/floor0.c"
    "${LIBVORBIS_SRC_DIR}/lib/floor1.c"
    "${LIBVORBIS_SRC_DIR}/lib/info.c"
    "${LIBVORBIS_SRC_DIR}/lib/lookup.c"
    "${LIBVORBIS_SRC_DIR}/lib/lpc.c"
    "${LIBVORBIS_SRC_DIR}/lib/lsp.c"
    "${LIBVORBIS_SRC_DIR}/lib/mapping0.c"
    "${LIBVORBIS_SRC_DIR}/lib/mdct.c"
    "${LIBVORBIS_SRC_DIR}/lib/psy.c"
    "${LIBVORBIS_SRC_DIR}/lib/registry.c"
    "${LIBVORBIS_SRC_DIR}/lib/res0.c"
    "${LIBVORBIS_SRC_DIR}/lib/sharedbook.c"
    "${LIBVORBIS_SRC_DIR}/lib/smallft.c"
    "${LIBVORBIS_SRC_DIR}/lib/synthesis.c"
    "${LIBVORBIS_SRC_DIR}/lib/window.c"
)

set(LIBVORBISFILE_FILES
    "${LIBVORBIS_SRC_DIR}/include/vorbis/vorbisfile.h"
    "${LIBVORBIS_SRC_DIR}/lib/vorbisfile.c"
)

source_group(include FILES ${LIBVORBIS_HDR_FILES})
source_group(source  FILES ${LIBVORBIS_SRC_FILES})
source_group(vorbis_file FILES ${LIBVORBISFILE_FILES})

include_directories(${LIBVORBIS_INCLUDE_DIRS})
add_library(libvorbis 
    ${LIBVORBIS_SRC_FILES} 
    ${LIBVORBIS_HDR_FILES} 
    ${LIBVORBISFILE_FILES}
)
target_link_libraries(libvorbis libogg)