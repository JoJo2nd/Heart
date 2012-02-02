/********************************************************************

	filename: 	EntityLib.h	
	
	Copyright (c) 28:1:2012 James Moran
	
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
#ifndef ENTITYLIB_H__
#define ENTITYLIB_H__

#include <string>

#define ENTITYLIB_DLL_API   __declspec( dllexport )
#define ENTITYLIB_API       __cdecl

typedef unsigned int uint;
typedef unsigned long uint32;

namespace Entity
{
    enum PropertyValueType
    {
        PropertyValueType_Bool,
        PropertyValueType_Int,
        PropertyValueType_Uint,
        PropertyValueType_Float,
        PropertyValueType_String,
        PropertyValueType_ResourceAssetID,
        PropertyValueType_None
    };

    class ENTITYLIB_DLL_API IErrorObject
    {
    public:
        IErrorObject( bool ok, const char* error, const char* loc )
            : ok_(ok)
            , error_(error)
        {
            error_ += loc;
        }
        virtual const char* GetErrorString() const { return error_.c_str(); }
        virtual bool        GetSuccess() const { return ok_; } 
        operator bool () const { return GetSuccess(); }


    private:

        std::string error_;
        bool        ok_;
    };

    class ENTITYLIB_DLL_API IComponentProperty
    {
    public:
        virtual ~IComponentProperty() {}

        virtual const char*         GetName() const = 0;
        virtual const char*         GetDoc() const = 0;
        virtual PropertyValueType   GetValueType() const = 0;
        virtual bool                GetValueBool() const = 0;
        virtual int                 GetValueInt() const = 0;
        virtual uint                GetValueUint() const = 0;
        virtual const char*         GetValueString() const = 0;
        virtual float               GetValueFloat() const = 0;
        virtual void                SetValueBool( bool v ) = 0;
        virtual void                SetValueInt( int v ) = 0;
        virtual void                SetValueUint( uint v ) = 0;
        virtual void                SetValueString( const char* v ) = 0;
        virtual void                SetValueFloat( float v ) = 0;

    private:

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version ) {}
    };

    class ENTITYLIB_DLL_API IComponentView
    {
    public:
        virtual ~IComponentView() {}
        
        virtual const char*                 GetName() const = 0;
        virtual uint                        GetDefaultPropertyCount() const = 0;
        virtual const IComponentProperty*   GetDefaultProperty( uint propIdx ) const = 0;

    private:

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version ) {}
    };

    class ENTITYLIB_DLL_API IEntityDefinitionView
    {
    public:
        virtual ~IEntityDefinitionView() {}

        virtual const char*             GetName() const = 0;
        virtual uint                    GetComponentCount() const = 0;
        virtual const IComponentView*   GetComponent( uint compIdx ) const = 0;
        virtual uint                    GetPropertyCount() const = 0;
        virtual IComponentProperty*     GetProperty( uint propIdx ) = 0;
        //TODO:
        // Add Component
        // Remove Component
    private:

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version ) {}
    };

    class ENTITYLIB_DLL_API ILibraryView
    {
    public:
        virtual ~ILibraryView() {}

        virtual const IErrorObject      LoadLibrary( const char* path ) = 0;
        virtual const IErrorObject      ImportComponetsFromXML( const char* path ) = 0;
        virtual const IErrorObject      SaveLibrary( const char* path ) = 0;
        virtual uint                    GetComponentCount() const = 0;
        virtual const IComponentView*   GetComponentView( uint componentIdx ) const = 0;
        virtual const IComponentView*   GetComponentViewByName( const char* name ) const = 0; 
        virtual uint                    GetEntityDefinitionCount() const = 0;
        virtual IEntityDefinitionView*  GetEntityDefinition( uint definitionIdx ) const = 0;
        virtual IEntityDefinitionView*  GetEntityDefinitionByName( const char* name ) const = 0;
        virtual const IErrorObject      AddEntity( const char* name ) = 0;

    private:

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version ) {}
    };
}

extern "C"
{
    ENTITYLIB_DLL_API Entity::ILibraryView*   ENTITYLIB_API     CreateEntityLibraryView();
    ENTITYLIB_DLL_API void                    ENTITYLIB_API     DestroyEntityLibraryView( Entity::ILibraryView* );
}

#endif // ENTITYLIB_H__