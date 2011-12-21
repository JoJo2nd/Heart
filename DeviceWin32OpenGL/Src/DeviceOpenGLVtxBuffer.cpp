/********************************************************************

	filename: 	DeviceOpenGLVtxBuffer.cpp
	
	Copyright (c) 2011/08/06 James Moran
	
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

namespace Heart
{
	

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hdOGLVtxBuffer::Unmap( void* ptr )
	{
		if ( ptr )
		{
			// Update the buffer object with the data passed in ptr
			glBindBuffer( GL_ARRAY_BUFFER, vtxBufferObj_ );
			glBufferData( GL_ARRAY_BUFFER, decl_.Stride()*nVertex_, ptr, GL_STATIC_DRAW );
			for ( hUint32 i = 0; i < hrVE_MAX; ++i )
			{
				if ( decl_.ElementOffsets( i ) != hErrorCode )
				{
					glVertexAttribPointer( 3, GL_FLOAT, decl_.Stride(), decl_.ElementOffsets( i ) );
				}
			}
		}
	}

}