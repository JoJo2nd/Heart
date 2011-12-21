/********************************************************************
	created:	2008/11/23
	created:	23:11:2008   11:42
	filename: 	hrVertexDeclarations.h
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRVERTEXDECLARATIONS_H__
#define HRVERTEXDECLARATIONS_H__

#include "hRendererConstants.h"
#include "HeartSTL.h"

namespace Heart
{
	class hVertexDeclaration : public pimpl< hdVtxDecl >
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