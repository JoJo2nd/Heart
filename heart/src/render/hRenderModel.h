/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HLODGROUP_H__
#define HLODGROUP_H__

#include "base/hTypes.h"
#include "hRenderable.h"
#include "math/hAABB.h"
#include "base/hRendererConstants.h"


namespace Heart {
    class hVertexBuffer;

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