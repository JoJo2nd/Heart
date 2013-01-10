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
    class hComponentFactory;
    class hHeartEngine;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    enum hComponentPropertyType
    {
        eComponentPropertyType_None,
        eComponentPropertyType_Bool,
        eComponentPropertyType_Int,
        eComponentPropertyType_UInt,
        eComponentPropertyType_Float,
        eComponentPropertyType_String,
        eComponentPropertyType_ResourceAsset,
    };

    enum hComponentTypeFlag
    {
        eComponentTypeFlag_Bool        = 1,
        eComponentTypeFlag_Int         = 1 << 1,
        eComponentTypeFlag_Float       = 1 << 2,
        eComponentTypeFlag_Array       = 1 << 3,
        eComponentTypeFlag_String      = 1 << 4,
        eComponentTypeFlag_Signed      = 1 << 5,
        eComponentTypeFlag_ResID       = 1 << 6,
        eComponentTypeFlag_Ptr         = 1 << 7,    
        //eComponentTypeFlag_Vector2
        //eComponentTypeFlag_Vector3
        //eComponentTypeFlag_Vector4
        //eComponentTypeFlag_Matrix
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template< typename t_ty >
    inline hUint32 hGetComponentTypeFlags()
    {
        hUint32 flags=0;
        if (hIs_same<hRemove_const<t_ty>::type, hBool>::value) flags|=eComponentTypeFlag_Bool;
        if (hIs_same<hRemove_pointer<t_ty>::type, hChar>::value) flags|=eComponentTypeFlag_String;
        if (hIs_same<hRemove_extent<t_ty>::type, hChar>::value) flags|=eComponentTypeFlag_String;
        if (hIs_same<t_ty, hResourceID>::value) flags|=eComponentTypeFlag_ResID;// TODO: ATM, will detect all uint64 as resource ids
        if (hIs_integral<t_ty>::value) flags|=eComponentTypeFlag_Int;
        if (hIs_floating_point<t_ty>::value) flags|=eComponentTypeFlag_Float;
        if (hIs_signed<t_ty>::value) flags|=eComponentTypeFlag_Signed;
        if (hIs_array<t_ty>::value) flags|=eComponentTypeFlag_Array;
        if (hIs_pointer<t_ty>::value) flags|=eComponentTypeFlag_Ptr;
        return flags;
    }

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
        hUint32                 flags_;
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
        hComponent( hEntity* entity, hUint32 id )
            : entity_(entity)
            , id_(id)
        {

        }
        virtual ~hComponent()
        {

        }

        hEntity*                            GetEntity() const { return entity_; }
        hUint32                             GetID() const { return id_; }
        virtual void                        OnCreate(hHeartEngine* engine) {};
        virtual void                        OnActivate() {};
        virtual void                        OnDeactivate() {};
        virtual void                        OnDestroy() {};
        void                                SetFactory(hComponentFactory* factory) { factory_ = factory; }
        hComponentFactory*                  GetFactory() const { return factory_; }

    protected:
        const hUint32           id_;

    private:

        hEntity*                entity_;
        hComponentFactory*      factory_;

    };

#define HEART_COMPONENT_TYPE() \
    public: \
        static const hChar*                 GetComponentName() { return componentName_; }\
        static hUint32                      GetComponentID() { return (hUint32)&componentID_; }\
        static hUint32*                     GetComponentIDAddress() { return &componentID_; }\
        static hUint32                      GetComponentHash() { return componentHash_; }\
        static hUint32                      GetPropertyCount();\
        static const Heart::hComponentProperty*    GetPropertyArray() { return propertyArray_; }\
    private: \
        static const hChar                  componentName_[];\
        static const hChar                  componentDoc_[];\
        static const hUint32                componentHash_;\
        static hUint32                      componentID_; /*always componentID_ == thisID_*/\
        static const Heart::hComponentProperty     propertyArray_[];\

#define HEART_DEFINE_COMPONENT_TYPE( klass, name, doc )\
    const hUint32   klass::componentHash_   = Heart::hCRC32::StringCRC( name );\
    hUint32         klass::componentID_     = ~0U;\
    const hChar     klass::componentName_[] = { name };\
    const hChar     klass::componentDoc_[]  = { doc };\

#define HEART_COMPONET_PROPERTIES_BEGIN( klass )\
    const Heart::hComponentProperty klass::propertyArray_[] = { \
        HEART_COMPONENT_PROPERTY(klass, "__id__", hUint32, id_, "Component Type ID")

#define HEART_COMPONET_PROPERTIES_END( klass ) \
    };\
    hUint32         klass::GetPropertyCount() { return hStaticArraySize(propertyArray_); }

#define HEART_COMPONENT_PROPERTY( klass, name, type, var, doc )\
    { name, doc, Heart::hGetComponentTypeFlags<type>(), Heart::eComponentPropertyType_None, Heart::ComponentPropertyName[Heart::eComponentPropertyType_None], hOffsetOf( klass, var ) },
}

#endif // HCOMPONENT_H__