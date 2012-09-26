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
    class hRenderable
    {
    public:

        hRenderable() 
            : indexBuffer_(NULL)
            , vertexBuffer_(NULL)
            , primType_(PRIMITIVETYPE_TRILIST)
            , nPrimatives_(0)
            , material_(NULL)
            , materialInstance_(NULL)
        {
        }
        virtual ~hRenderable() 
        {
            if (material_)
            {
                material_->DestroyMaterialInstance(materialInstance_);
                HEART_RESOURCE_SAFE_RELEASE(material_);
            }
        }

        hVertexBuffer*                          GetVertexBuffer();
        void                                    SetVertexBuffer(hVertexBuffer* vtx) {vertexBuffer_ = vtx;}
        hIndexBuffer*                           GetIndexBuffer();
        void                                    SetIndexBuffer(hIndexBuffer* idx) {indexBuffer_ = idx;}
        PrimitiveType                           GetPrimativeType() const;
        void                                    SetPrimativeType(PrimitiveType primtype) { primType_ = primtype; }
        hUint32                                 GetStartIndex() const { return startIndex_; }
        void                                    SetStartIndex(hUint32 startIdx) { startIndex_ = startIdx; }
        hUint32									GetPrimativeCount() const { return nPrimatives_; }
        void                                    SetPrimativeCount(hUint32 primCount) { nPrimatives_ = primCount; }
        void                                    SetMaterial(hMaterial* material) { material_ = material; }
        hMaterialInstance*                      GetMaterialInstance();
        hAABB						            GetAABB() const { return aabb_; }
        void									SetAABB( const Heart::hAABB& aabb ) { aabb_ = aabb; }
    
    private:

        hIndexBuffer*           indexBuffer_;
        hVertexBuffer*          vertexBuffer_;
        PrimitiveType           primType_;
        hUint32                 startIndex_;
        hUint32                 nPrimatives_;
        hMaterial*              material_;
        hMaterialInstance*      materialInstance_;
        Heart::hAABB            aabb_;
    };
}

#endif // hiMesh_h__