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

    class HEART_DLLEXPORT hDrawCallContext
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
        void SubmitDrawCall(const hDrawCall& dc)
        {
            if (calls_ == MAX_DC) Flush();
            dcs_[calls_++] = dc;
        }
        void End();

    private:

        void Flush();

        static const hUint32            MAX_DC = 32;

        hScissorRect                    scissor_;
        hRenderer*                      renderer_;
        hByte                           layerID_;
        hByte                           camID_;
        hUint32                         calls_;
        hArray< hDrawCall, MAX_DC >     dcs_;
    };
}

#endif // HDRAWCALLCONTEXT_H__