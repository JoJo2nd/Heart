/********************************************************************

	filename: 	DebugRenderer.h	
	
	Copyright (c) 4:6:2011 James Moran
	
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

#ifndef DEBUGRENDERER_H__
#define DEBUGRENDERER_H__

namespace Heart
{
	class hResourceManager;
	struct hVec3;
	struct hAABB;
	struct hRect;
	struct hColour;
	class hRenderer;
	class hSceneNodeCamera;

namespace DebugRenderer
{
	void	Initialise( hResourceManager* resourceManager, hRenderer* renderer );
	void	EnableDebugDrawing( hBool val );
	void	RenderDebugText( hFloat x, hFloat y, const char* fmt, ... );
	void	RenderDebugSphere( const hVec3& centre, hFloat radius, const hColour& colour );
	void	RenderDebugAABB( const hAABB& aabb, const hColour& colour );
	void	RenderDebugScreenQuad( const hRect& rect, const hColour& colour );
	void	RenderDebugCone( const hMatrix& transform, hFloat radius, hFloat len, const hColour& colour );
	void	RenderDebugRenderCalls( hRenderer* renderer, hSceneNodeCamera* camera );
	void	Destory();
}
}

#endif // DEBUGRENDERER_H__