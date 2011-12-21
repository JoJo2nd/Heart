/********************************************************************

	filename: 	DeviceWin32OpenGL.h
	
	Copyright (c) 2011/07/26 James Moran
	
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
#ifndef _DEVICEWIN32OPENGL_H__
#define _DEVICEWIN32OPENGL_H__

//////////////////////////////////////////////////////////////////////////
// Common Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hHeartConfig.h"

#define GLEW_STATIC

//////////////////////////////////////////////////////////////////////////
// Platform Includes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "DeviceWin32.h"

//////////////////////////////////////////////////////////////////////////
// Base Heart Includes ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hTypes.h"
#include "hDebugMacros.h"
#include "hMemory.h"
#include "hEventManager.h"
#include "hRendererConstants.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "DeviceOpenGLRenderer.h"
#include "DeviceOpenGLIndexBuffer.h"
#include "DeviceOpenGLTexture.h"
#include "DeviceOpenGLVtxBuffer.h"
#include "DeviceOpenGLVtxDecl.h"
#include "DeviceOpenGLMaterial.h"

//////////////////////////////////////////////////////////////////////////
// Typedef to common names ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef Heart::hdOGLRenderer		hdRenderer;
typedef Heart::hdOGLVtxDecl			hdVtxDecl;
typedef Heart::hdOGLVtxBuffer		hdVtxBuffer;
typedef Heart::hdOGLIndexBuffer		hdIndexBuffer;
typedef Heart::hdOGLTexture			hdTexture;
typedef Heart::hdOGLMaterial		hdMaterial;

#endif // _DEVICEWIN32OPENGL_H__