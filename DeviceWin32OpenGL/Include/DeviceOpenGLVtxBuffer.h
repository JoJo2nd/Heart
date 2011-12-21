/********************************************************************

	filename: 	DeviceOpenGLVtxBuffer.h	
	
	Copyright (c) 26:7:2011 James Moran
	
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

#ifndef DEVICEOPENGLVTXBUFFER_H__
#define DEVICEOPENGLVTXBUFFER_H__

namespace Heart
{
	class hdOGLVtxBuffer
	{
	public:
		hdOGLVtxBuffer()
		{}
		~hdOGLVtxBuffer() {}

		hBool		Bound() const { return hFalse; }

		// TODO: New Style: map() & unmap()
		void*		Map() { return NULL; }
		void		Unmap( void* ptr );

	private:

		friend class hdOGLRenderer;

		hdOGLVtxDecl	decl_;
		hUint32			nVertex_;
		GLint			vtxArray_;
		GLint			vtxBufferObj_;
	};
}

#endif // DEVICEOPENGLVTXBUFFER_H__