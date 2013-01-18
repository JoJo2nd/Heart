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
            , matInstance_(NULL)
            , primType_(PRIMITIVETYPE_TRILIST)
            , indexBuffer_(NULL)
        {
            hZeroMem(vertexBuffer_, sizeof(vertexBuffer_));
        }
        virtual ~hRenderable() 
        {
        }

        hVertexBuffer*                          GetVertexBuffer(hUint32 stream) const {
            hcAssert(stream < HEART_MAX_INPUT_STREAMS); 
            return vertexBuffer_[stream]; 
        }
        void                                    SetVertexBuffer(hUint16 stream, hVertexBuffer* vtx) {
            hcAssert(stream < HEART_MAX_INPUT_STREAMS);
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
        hUint16                                 GetStartIndex() const { return drawItem_.startVertex_; }
        void                                    SetStartIndex(hUint16 startIdx) { drawItem_.startVertex_ = startIdx; }
        hUint16									GetPrimativeCount() const { return drawItem_.primCount_; }
        void                                    SetPrimativeCount(hUint16 primCount) { drawItem_.primCount_ = primCount; }
        void                                    SetMaterialResourceID(hResourceID val) {materialID_ = val;}
        hResourceID                             GetMaterialResourceID() const { return materialID_; }
        void                                    SetMaterial(hMaterialInstance* material);
        hMaterialInstance*                      GetMaterial() const { return matInstance_; }
        hUint32                                 GetMaterialKey() const { return materialKey_; }
        hAABB						            GetAABB() const { return aabb_; }
        void									SetAABB( const Heart::hAABB& aabb ) { aabb_ = aabb; }
        void                                    bind() { 
            matInstance_->bindInputStreams(primType_, indexBuffer_, vertexBuffer_, HEART_MAX_INPUT_STREAMS); 
        }
    
    private:

        hResourceID             materialID_;
        hUint32                 materialKey_;
        hDrawCall               drawItem_;
        hMaterialInstance*      matInstance_;
        hAABB                   aabb_;
        PrimitiveType           primType_;
        hIndexBuffer*           indexBuffer_;
        hVertexBuffer*          vertexBuffer_[HEART_MAX_INPUT_STREAMS];
        hdRenderStreamsObject   vertexStreams_;
    };
}

#endif // hiMesh_h__