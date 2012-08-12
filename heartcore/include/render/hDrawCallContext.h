/********************************************************************

	filename: 	hDrawCallContext.h	
	
	Copyright (c) 6:8:2012 James Moran
	
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
#ifndef HDRAWCALLCONTEXT_H__
#define HDRAWCALLCONTEXT_H__

namespace Heart
{
    class hVertexBuffer;
    class hIndexBuffer;

    class HEARTCORE_SLIBEXPORT hDrawCallContext
    {
    public:
        hDrawCallContext()
            : renderer_(NULL)
            , calls_(0)
        {

        }
        ~hDrawCallContext()
        {}

        void Begin(hRenderer* renderer);
        void SetRenderLayer(hByte val) { layerID_ = val; }
        void SetCameraID(hByte val) { camID_ = val; }
        void SubmitDrawCall( hMaterialInstance* mat, 
                            hIndexBuffer* ib, 
                            hVertexBuffer* vb, 
                            hUint32 primCount, 
                            hBool transparent = hFalse,
                            hFloat vsDepth = 0.f,
                            hUint32 startVtx = 0, 
                            PrimitiveType type = PRIMITIVETYPE_TRILIST);
        void SubmitDrawCallInline(hMaterialInstance* mat, 
                                  hByte* ibOut, hUint32 ibSize,
                                  hByte* vbOut, hUint32 vbSize,
                                  hUint16 primCount, 
                                  hUint16 stride,
                                  hBool transparent = hFalse,
                                  hFloat vsDepth = 0.f,
                                  PrimitiveType type = PRIMITIVETYPE_TRILIST);
        void SubmitDrawCall(const hDrawCall& dc);
        void SubmitCommand(hUint32 cmd, void* data, hUint32 size);
        void End();

    private:

        void Submit();

        static const hUint32            MAX_DC = 32;
        hRenderer*                      renderer_;
        hByte                           layerID_;
        hByte                           camID_;
        hUint32                         calls_;
        hArray< hDrawCall, MAX_DC >     dcs_;
    };
}

#endif // HDRAWCALLCONTEXT_H__