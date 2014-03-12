/********************************************************************

    filename: 	hMesh.h	
    
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

#ifndef hiMesh_h__
#define hiMesh_h__

namespace Heart
{
    class hRenderer;

    //////////////////////////////////////////////////////////////////////////
    // A Renderable simply describes a collection of triangles that can be ///
    // rendered by the renderer along with a material. ///////////////////////
    //////////////////////////////////////////////////////////////////////////
    class HEART_DLLEXPORT hRenderable
    {
    public:

        hRenderable() 
            : materialKey_(0)
            , primType_(PRIMITIVETYPE_TRILIST)
            , indexBuffer_(NULL)
        {
        }
        hRenderable(hRenderable&& rhs)
            : indexBuffer_(hNullptr)
        {
            swap(this, &rhs);
        }
        hRenderable& operator = (hRenderable&& rhs) {
            swap(this, &rhs);
            return *this;
        }
        virtual ~hRenderable() {
            for (hUint i=0, n=inputLayouts_.size(); i<n; ++i) {
                if (inputLayouts_[i]) {
                    inputLayouts_[i]->Release();
                    inputLayouts_[i]=NULL;
                }
            }
        }

        hUint                                   vertexStreamCountHint(hUint val) { vertexBuffer_.reserve(val); }
        hUint                                   getVertexStreamCount() const { return (hUint)vertexBuffer_.size(); }
        hVertexBuffer*                          getVertexStream(hUint32 stream) const { return vertexBuffer_[stream]; }
        void                                    setVertexStream(hUint16 stream, hVertexBuffer* vtx) {
            if (vertexBuffer_.size() <= stream) {
                vertexBuffer_.resize(stream+1);
            }
            vertexBuffer_[stream]=vtx;
        }
        hIndexBuffer*                           getIndexBuffer() const { return indexBuffer_; }
        void                                    SetIndexBuffer(hIndexBuffer* idx) {
            indexBuffer_=idx;
        }
        PrimitiveType                           GetPrimativeType() const { return primType_; }
        void                                    SetPrimativeType(PrimitiveType primtype) { 
            hcAssert(primtype < PRIMITIVETYPE_MAX);
            primType_ = primtype;
        }
        hUint                                   GetStartIndex() const { return startVertex_; }
        void                                    SetStartIndex(hUint startIdx) { startVertex_ = startIdx; }
        hUint                                   GetPrimativeCount() const { return primCount_; }
        void                                    SetPrimativeCount(hUint primCount) { primCount_ = primCount; }
        void                                    setMaterialResourceID(hStringID val) {setMaterial(hResourceHandle(val));}
        hStringID                               getMaterialResourceID() const { return materialHandle_.getResourceID(); }
        hMaterial*                              GetMaterial() const { return materialHandle_.weakPtr<hMaterial>(); }
        hUint32                                 GetMaterialKey() const { return materialKey_; }
        hAABB   GetAABB() const { return aabb_; }
        void    SetAABB( const Heart::hAABB& aabb ) { aabb_ = aabb; }
        void initialiseRenderCommands(hRenderCommandGenerator* rcGen);
        hUint getRenderCommandOffset(hUint g, hUint t, hUint p) const {
            return cmdLookUp_.getCommand(g, t, p);
        }
    
    private:

        hRenderable(const hRenderable&);
        hRenderable operator = (const hRenderable&);
        void swap(hRenderable* lhs, hRenderable* rhs) {
            std::swap(lhs->materialKey_, rhs->materialKey_);
            std::swap(lhs->primCount_, rhs->primCount_);
            std::swap(lhs->startVertex_, rhs->startVertex_);
            std::swap(lhs->materialHandle_, rhs->materialHandle_);
            std::swap(lhs->aabb_, rhs->aabb_);
            std::swap(lhs->primType_, lhs->primType_);
            std::swap(lhs->indexBuffer_, rhs->indexBuffer_);
            std::swap(lhs->vertexBuffer_, rhs->vertexBuffer_);
            std::swap(lhs->cmdLookUp_, cmdLookUp_);
            lhs->inputLayouts_.swap(&rhs->inputLayouts_);
        }

        void  setMaterial(const hResourceHandle& material);
        hBool resourceUpdate(hStringID resourceid, hResurceEvent event, hResourceManager* resmanager, hResourceClassBase* resource);

        hUint32                     materialKey_;
        hUint                       primCount_;
        hUint                       startVertex_;
        hResourceHandle             materialHandle_;
        hAABB                       aabb_;
        PrimitiveType               primType_;
        hIndexBuffer*               indexBuffer_;
        std::vector<hVertexBuffer*> vertexBuffer_;
        hMaterialCmdLookUpHelper    cmdLookUp_;
        hVector<hdInputLayout*>     inputLayouts_;
    };
}

#endif // hiMesh_h__