/********************************************************************
    
    Copyright (c) 20:4:2014 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/

#include "gal/device.h"
#include "gal/types.h"
#include "gal/utils.h"
#include "gal/platform.h"

#include <vector>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#include <windows.h> //< maybe avoid this...?

namespace gal {

struct Device {
    Device() 
        : hWnd_(0)
        , hDC_(0)
        , glCtx_(0)
        , renderThreadId_(0)
    {
        glCtxListMtx_=createMutex();
    }
    ~Device() 
    {
        destroyMutex(glCtxListMtx_);
        glCtxListMtx_=nullptr;
    }

    HWND hWnd_;
    HDC  hDC_;
    // the rendering context. Only one exists and it shouldn't swap thread very often (if ever)
    // It has to be explicitly assigned to a thread. Other gl context exist and are for creating sharing resources only
    HGLRC glCtx_;
    gal_uintptr_t renderThreadId_;

    std::vector< HGLRC > glCtxList_;
    mutex* glCtxListMtx_;

    HGLRC createGLContext() {
        HGLRC ctx;
        if (WGLEW_ARB_create_context) {
             ctx = wglCreateContextAttribsARB(hDC_, glCtx_, nullptr);
        } else {
            ctx = wglCreateContext(hDC_);
            wglShareLists(glCtx_, ctx);
        }
        MutexSentry sentry(glCtxListMtx_);
        glCtxList_.push_back(ctx);

        return ctx;
    }

    bool isRenderThread() {
        return renderThreadId_ == getThreadID();
    }

    bool aquireRenderContext() {
        gal_assert(renderThreadId_ == 0, "Cannot aquire rendering thread when is it already aquired by another thread");
        renderThreadId_ = getThreadID();
        return !!wglMakeCurrent(hDC_, glCtx_);
    }

    bool releaseRenderContext() {
        bool released=false;
        if (isRenderThread()) {
            released = !!wglMakeCurrent(nullptr, nullptr);
            renderThreadId_ = 0;
        }
        return released;
    }

    static void /*APIENTRY*/ openGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam) {
        /*
            temp so we get some output, needs to be pass a message callback from the create call
        */
        OutputDebugString(message);
        OutputDebugString("\n");
    };
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Device* createDevice(HWND hWnd) {

    GLenum glew_error;
    Device* device = new Device();

    device->hWnd_ = hWnd;
    device->hDC_ = GetDC(hWnd);

    gal_assert(wglGetCurrentContext() == nullptr, "expecting a null gl context");
    device->glCtx_ = wglCreateContext(device->hDC_);
    device->aquireRenderContext();
    glewExperimental=TRUE;//for Core context
    glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        return nullptr;
    }

    if (GLEW_ARB_debug_output) {
        glDebugMessageCallbackARB(&Device::openGLDebugMessage, device);
        glDebugMessageControlARB(GL_DONT_CARE/*source*/, GL_DONT_CARE/*type*/, GL_DONT_CARE/*severity*/, 0, nullptr, GL_TRUE);
    }
    device->releaseRenderContext();
    return device;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void releaseDevice(Device* device) {
    delete device;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool aquireRenderThread(Device* device) {
    gal_assert(device, "invalid device");
    return device->aquireRenderContext();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool releaseRenderThread(Device* device) {
    gal_assert(device, "invalid device");
    return device->releaseRenderContext();
}

}