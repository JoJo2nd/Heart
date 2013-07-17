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
            : materialID_(0)
            , materialKey_(0)
            , material_(NULL)
            , primType_(PRIMITIVETYPE_TRILIST)
            , indexBuffer_(NULL)
        {
        }
        virtual ~hRenderable() 
        {
        }

        hUint                                   getVertexBufferCount() const { return vertexBuffer_.GetSize(); }
        hVertexBuffer*                          GetVertexBuffer(hUint32 stream) const { return vertexBuffer_[stream]; }
        void                                    SetVertexBuffer(hUint16 stream, hVertexBuffer* vtx) {
            if (vertexBuffer_.GetSize() <= stream) {
                vertexBuffer_.Resize(stream+1);
            }
            vertexBuffer_[stream]=vtx;
        }
        hIndexBuffer*                           GetIndexBuffer() const { return indexBuffer_; }
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
        hUint  									GetPrimativeCount() const { return primCount_; }
        void                                    SetPrimativeCount(hUint primCount) { primCount_ = primCount; }
        void                                    SetMaterialResourceID(hResourceID val) {materialID_ = val;}
        hResourceID                             GetMaterialResourceID() const { return materialID_; }
        void                                    SetMaterial(hMaterial* material);
        hMaterial*                              GetMaterial() const { return material_; }
        hUint32                                 GetMaterialKey() const { return materialKey_; }
        hAABB						            GetAABB() const { return aabb_; }
        void									SetAABB( const Heart::hAABB& aabb ) { aabb_ = aabb; }
        void                                    bind() { 
            //matInstance_->bindInputStreams(primType_, indexBuffer_, vertexBuffer_.GetBuffer(), vertexBuffer_.GetSize()); 
        }
        void initialiseRenderCommands(hRenderCommandGenerator* rcGen);
        hUint getRenderCommandOffset(hUint g, hUint t, hUint p) const {
            return cmdLookUp_.getCommand(g, t, p);
        }
    
    private:

        hResourceID              materialID_;
        hUint32                  materialKey_;
        hUint                    primCount_;
        hUint                    startVertex_;
        hMaterial*               material_;
        hAABB                    aabb_;
        PrimitiveType            primType_;
        hIndexBuffer*            indexBuffer_;
        hVector<hVertexBuffer*>  vertexBuffer_;
        hMaterialCmdLookUpHelper cmdLookUp_;
        hVector<hdInputLayout*>  inputLayouts_;
    };
}

#endif // hiMesh_h__