/********************************************************************

	filename: 	hVertexDeclarations.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef HRVERTEXDECLARATIONS_H__
#define HRVERTEXDECLARATIONS_H__

namespace Heart
{
	class hVertexDeclaration : public hPtrImpl< hdVtxDecl >
	{
	public:
		
		hVertexDeclaration()
		{}

		virtual ~hVertexDeclaration()
		{}

		hUint32			Stride() const { return stride_; }
		hUint32			GetElementOffset( VertexElement element ) const 
		{ 
			hcAssertMsg( elementOffsets_[ element ] != hErrorCode, "accessing element not defined in vertex declaration" ); 
			return elementOffsets_[ element ];
		}

	private:

		friend class hRenderer;

		hUint32			vtxFlags_;
		//both these are set by the implementation, here for speed
		hUint32			stride_;
		hUint32			elementOffsets_[ hrVE_MAX ];

	};
}

#endif // HRVERTEXDECLARATIONS_H__