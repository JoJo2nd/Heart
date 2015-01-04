/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "lfds/lfds.h"
#include <GL/glew.h>    
#include <GL/gl.h>
#include <SDL.h>

namespace Heart {
namespace hRenderer {

struct hRenderDestructBase;

void* rtmp_malloc(hSize_t size, hUint alignment=16);
void rtmp_frameend();
hRenderDestructBase* dequeueRenderResourceDelete();
void hglEnsureTLSContext();
SDL_GLContext hglTLSMakeCurrent();
void hglReleaseTLSContext();
void hGLSyncFlush();

}
}