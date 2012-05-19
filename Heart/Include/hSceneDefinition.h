/********************************************************************

	filename: 	hSceneDefinition.h	
	
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

#ifndef HSCENEDEFINITION_H__
#define HSCENEDEFINITION_H__

namespace Heart
{
    class hSceneNodeComponent : public hComponent
    {
        HEART_COMPONENT_TYPE();
    public: 
        hSceneNodeComponent();
        hSceneNodeComponent(hEntity* entity)
            : hComponent(entity, GetComponentID());
        {

        }
    private:

        hMatrix         nextLocalMatrix_;
        hMatrix         localMatrix_;

    };

    class hRenderModelComponent 
    {
        hLODGroup       lodGroup_;
        hMatrix         transform_;
    };

    class hPhysicsComponent
    {

    };

    class hLightComponent
    {

    };

    struct hSharedBufferData
    {
        void* index_;
        void* vertex_;
        hUint32 vertexFormat_;//etc,etc
    };

    class hSceneDefinition
    {
    public:
        hSceneDefinition() {}
        ~hSceneDefinition() {}
    private:

        // EntityMap 
        /*
         struct 
         {
            name
            components(xN) (index into arrays)
         }
        */

        hVector< hEntity* >                 sceneEntities_;
        hVector< hSceneNodeComponent >      sceneNodeComponents_;
        //hLinearOctree...?
        hVector< hRenderModelComponent >    renderModels_;
        hVector< hIndexBuffer* >            sharedIndexBuffers_;
        hVector< hVertexBuffer* >           sharedVertexBuffers_;
        hVector< hSharedBufferData >        sharedBufferInitData_;
        hVector< hMaterial* >               materials_;
    };

    /*
     * Parsing and building scenes is done via a single DAE/collada scene and a object properties xml.
     * The object  properties links game like props (e.g. material names, physics mass, etc) to 
     * names in the collada file. 
     * Physics objects are identified by being under a Node called "physics" in the collada file.
     * Render objects are identified by being under a Node called "render" in the collada file.
     * New layers may be added later, (May be possible to xref in?) but all layers must be just under the
     * root node in the scene.
     * Components are built off-line and properties filled in as needed from the properties file.
     * The build will fail if an object is missing an important property (like render object with no material)
     * The build will combine vertex buffers that share the same vertex format into a single buffer (except when
     * the vertex buffer limit is blown, then a new buffer is made) to reduce state swaps.
     * Entities for a scene are created at runtime but components are built off-line and added to runtime entities.
     * This will require some name-to-node list/map which should be discarded after load.
     * Physics objects can be linked to render objects by the property file OR by sharing the name of the object 
     * but with a 'p_' prefix (e.g. render_dude --> p_render_dude). In this case the render model's position will
     * override the physics position.
     * LOD's are done by grouping a collection of drawable objects under a LOD_[render_name] node and prefixing
     * the name of each object in that node with the level (up to 9) like so...
     *      + LOD_render_dude (NOTE: would link to p_render_dude physics object)
     *          + 0_whatever1 <--two objects in layer one
     *          + 0_whatever2 <--
     *          + 1_whatever1 <--two objects in layer two
     *          + 1_whatever2
     *          + 2_whateverCombined <-- combined object for both objects in layer 3 :)
     *          + allways_highest_lod <-- with no prefix, this is dropped in the level zero and always render, no matter what distance
     * LOD ranges are defined in the object properties xml.
     * Name of objects in the object properties xml are matched but without any prefixes, so
     * LOD_render_dude & p_render_dude are matched to "render_dude" in the objs props xml.
     ****************************************************************************************************************************
     * At runtime, the job chains can be used to walk the renderables, lights, scene objects and update props as needed.
     * Arrays would just need to be split in to small chunks
     **/
}

#endif // HSCENEDEFINITION_H__