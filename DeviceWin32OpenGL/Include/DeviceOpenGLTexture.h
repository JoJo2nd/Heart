/********************************************************************

	filename: 	DeviceOpenGLTexture.h	
	
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
#ifndef DEVICEOPENGLTEXTURE_H__
#define DEVICEOPENGLTEXTURE_H__

namespace Heart
{
	class hdOGLTexture
	{
	public:
		hdOGLTexture() {}
		~hdOGLTexture() {}

		hBool		Bound() const { return texture_ != 0; }
		void*		Map( hUint32 level, hUint32* pitch ) 
		{ 
			*pitch = 0; 
			return NULL; 
		}
		void		Unmap( hUint32 level, void* ptr )
		{
			glBindTexture( GL_TEXTURE_2D, texture_ );
			glTexImage2D( GL_TEXTURE_2D, 0, internalFormat_, width_ >> level, height_ >> level, 0, format_, type_, ptr );
		}

	private:

		hUint32		width_;
		hUint32		height_;
		GLint		texture_;
		GLint		internalFormat_;
		GLenum		format_;
		GLenum		type_;
	};
}

#endif // DEVICEOPENGLTEXTURE_H__