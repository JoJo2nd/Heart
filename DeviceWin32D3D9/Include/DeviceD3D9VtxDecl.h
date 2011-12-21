#ifndef DEVICED3D9VTXDECL_H__
#define DEVICED3D9VTXDECL_H__

namespace Heart
{
	class hdD3D9VtxDecl
	{
	public:
		hdD3D9VtxDecl() 
			: pVtxDecl_( NULL )
		{}
		virtual ~hdD3D9VtxDecl() {}

		hBool	Bound() const { return pVtxDecl_ != NULL; }

	private:

		friend class hdD3D9Renderer;

		hUint32								vertexFlags_;
		hUint32								stride_;
		LPDIRECT3DVERTEXDECLARATION9		pVtxDecl_;
	};
}

#endif // _DEVICED3D9VTXDECL_H__