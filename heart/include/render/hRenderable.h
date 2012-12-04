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
            , vtxStreams_(0)
        {
        }
        virtual ~hRenderable() 
        {
        }

        hVertexBuffer*                          GetVertexBuffer(hUint32 stream) const { hcAssert(stream < hDrawCall::MAX_VERT_STREAMS); return drawItem_.vertexBuffer_[stream]; }
        hUint32                                 GetVertexStreams() const { return vtxStreams_; }
        void                                    SetVertexBuffer(hUint32 stream, hVertexBuffer* vtx) 
        {
            hcAssert(stream < hDrawCall::MAX_VERT_STREAMS); 
            vtxStreams_ = hMax(vtxStreams_,stream); 
            drawItem_.vertexBuffer_[stream] = vtx;
        }
        hIndexBuffer*                           GetIndexBuffer() const { return drawItem_.indexBuffer_; }
        void                                    SetIndexBuffer(hIndexBuffer* idx) { drawItem_.indexBuffer_ = idx; }
        PrimitiveType                           GetPrimativeType() const { return drawItem_.primType_; }
        void                                    SetPrimativeType(PrimitiveType primtype) { drawItem_.primType_ = primtype; }
        hUint32                                 GetStartIndex() const { return drawItem_.startVertex_; }
        void                                    SetStartIndex(hUint32 startIdx) { drawItem_.startVertex_ = startIdx; }
        hUint32									GetPrimativeCount() const { return drawItem_.primCount_; }
        void                                    SetPrimativeCount(hUint32 primCount) { drawItem_.primCount_ = primCount; }
        void                                    SetMaterialResourceID(hResourceID val) {materialID_ = val;}
        hResourceID                             GetMaterialResourceID() const { return materialID_; }
        void                                    SetMaterial(hMaterial* material);
        hMaterial*                              GetMaterial() const { return material_; }
        hUint32                                 GetMaterialKey() const { return materialKey_; }
        hAABB						            GetAABB() const { return aabb_; }
        void									SetAABB( const Heart::hAABB& aabb ) { aabb_ = aabb; }
    
    private:

        hResourceID             materialID_;
        hUint32                 materialKey_;
        hUint32                 vtxStreams_;
        hDrawCall               drawItem_;
        hMaterial*              material_;
        Heart::hAABB            aabb_;
    };
}

#endif // hiMesh_h__