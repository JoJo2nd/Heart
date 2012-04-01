/********************************************************************

	filename: 	hComponent.h	
	
	Copyright (c) 22:1:2012 James Moran
	
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
#ifndef HCOMPONENT_H__
#define HCOMPONENT_H__

namespace Heart
{
    class hEntity;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    enum hComponentPropertyType
    {
        eComponentPropertyType_Bool,
        eComponentPropertyType_Int,
        eComponentPropertyType_UInt,
        eComponentPropertyType_Float,
        eComponentPropertyType_String,
        eComponentPropertyType_ResourceAsset,
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    extern const hChar* ComponentPropertyName[];

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    struct hComponentProperty
    {
        const hChar*            name_;
        const hChar*            doc_;
        hComponentPropertyType  type_;
        const hChar*            typeStr_;
        hUint32                 offset_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hComponent
    {
    public:
        hComponent( hEntity* entity )
            : entity_(entity)
        {

        }
        ~hComponent()
        {

        }

        hEntity*                            GetEntity() const { return entity_; }

    private:

        hEntity* entity_;
    };

#define HEART_COMPONENT_TYPE() \
    public: \
        static const hChar*                 GetComponentName() { return componentName_; }\
        static hUint32                      GetComponentID() { return componentID_; }\
        static hUint32*                     GetComponentIDAddress() { return &componentID_; }\
        static hUint32                      GetComponentHash() { return componentHash_; }\
        static hUint32                      GetPropertyCount();\
        static const hComponentProperty*    GetPropertyArray() { return propertyArray_; }\
    private: \
        static const hChar                  componentName_[];\
        static const hChar                  componentDoc_[];\
        static const hUint32                componentHash_;\
        static hUint32                      componentID_;\
        static const hComponentProperty     propertyArray_[];\

#define HEART_DEFINE_COMPONENT_TYPE( klass, name, doc )\
    const hUint32   klass::componentHash_   = hCRC32::StringCRC( name );\
    hUint32         klass::componentID_     = ~0U;\
    const hChar     klass::componentName_[] = { name };\
    const hChar     klass::componentDoc_[]  = { doc };\

#define HEART_COMPONET_PROPERTIES_BEGIN( klass )\
    const hComponentProperty klass::propertyArray_[] = {

#define HEART_COMPONET_PROPERTIES_END( klass ) \
    };\
    hUint32         klass::GetPropertyCount() { return hStaticArraySize(propertyArray_); }

#define HEART_COMPONENT_PROPERTY( klass, name, var, type, doc )\
    { name, doc, eComponentPropertyType_##type, ComponentPropertyName[eComponentPropertyType_##type], hOffsetOf( klass, var ) },
}

#endif // HCOMPONENT_H__