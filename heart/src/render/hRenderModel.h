/********************************************************************

	filename: 	hRenderModel.h	
	
	Copyright (c) 13:5:2012 James Moran
	
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

#ifndef HLODGROUP_H__
#define HLODGROUP_H__

#include "base/hTypes.h"
#include "hRenderable.h"
#include "math/hAABB.h"
#include "base/hRendererConstants.h"


namespace Heart
{

    class  hRenderModel
    {
    public:

        hRenderModel() {}
        ~hRenderModel() {}
    
        hAABB           getBounds() const { return totalBounds_; }
        hBool           bindVertexStream(hUint inputSlot, hVertexBuffer* vtxBuf);
        hRCmd*          getRenderCommands(hUint offset) {
            return renderCommands_.getCommandAtOffset(offset);
        }
        void            setRenderableCountHint(hUint val) {
            renderables_.reserve(val);
        }
        hRenderable*    appendRenderable() {
            renderables_.push_back(hRenderable());
            return &(*renderables_.rbegin());
        }
        hUint           getRenderableCount() const {
            return (hUint)renderables_.size();
        }
        hRenderable*    getRenderable(hUint i) {
            return &renderables_[i];
        }
        const hRenderable*    getRenderable(hUint i) const {
            return &renderables_[i];
        }
        void calculateBounds();
        void listenForResourceEvents();
        void stopListeningForResourceEvents();
        void cleanUp();

    private:

        hBool resourceUpdate(hStringID resourceid, hResurceEvent event);

        hBool attemptResourceInsert();

        void  initialiseRenderCommands();
        void  destroyRenderCommands();

        std::vector< hRenderable >                  renderables_;
        hAABB                                       totalBounds_;
        hRenderCommands                             renderCommands_;
    };

}

#endif // HLODGROUP_H__