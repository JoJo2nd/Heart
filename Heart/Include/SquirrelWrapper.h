/********************************************************************
	created:	2010/10/03
	created:	3:10:2010   11:35
	filename: 	SquirrelWrapper.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef SQUIRRELWRAPPER_H__
#define SQUIRRELWRAPPER_H__

#include "hcTypes.h"
#include "hcMemory.h"
#include "HeartSTL.h"
#define _INCLUDED_IN_SQUIRREL_WRAPPER_
#include "squirrel.h"
#pragma warning ( push )
#pragma warning ( disable : 4800 )
#pragma warning ( disable : 4244 )
#define SQBIND_CUSTOMIZE "sqbind_config.h"
#include "sqbind.h" 
#pragma warning ( pop )
#undef _INCLUDED_IN_SQUIRREL_WRAPPER_

//////////////////////////////////////////////////////////////////////////
// standard string wrapper ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template<>
class SqBind<char> 
{
public:
	struct Getter 
	{
		SQBIND_INLINE const char* get(HSQUIRRELVM v, int p_idx) 
		{
			return SqBind<char>::get(v,2);
		}
	};
	struct GetterPtr 
	{
		SQBIND_INLINE const char* get(HSQUIRRELVM v, int p_idx) 
		{
			return SqBind<char>::get(v,p_idx);
		}
	};
	static const char* get(HSQUIRRELVM v, int p_idx) 
	{
		if (sq_gettype(v,p_idx)!=OT_STRING) 
		{
			sqbind_throwerror(v,"Type is not string!");
			return NULL;
		}
		const SQChar* str;
		sq_getstring(v,p_idx,&str);
		return str;
	}

	static void push(HSQUIRRELVM v, const char* p_value) 
	{
		sq_pushstring( v, p_value, -1 );
	}
};

namespace Heart
{
namespace VM
{
	class SquirrelWrapper
	{
	public:
		SquirrelWrapper();
		virtual ~SquirrelWrapper();

		void			Initialise();
		void			Destroy();

		void			ExecuteBuffer( const hChar* buff, hUint32 size );

		void			Update();

		HSQUIRRELVM		GetVM() { return globalVM_; }

		/**
		* TransferOwerToC - transfers ownership of a object created by  the squirrel
		* VM to C++. Must be called from within a squirrel call (e.g. callback from squirrel )
		*
		* @param 	HSQUIRRELVM vm
		* @param 	SQUserPointer * ptr - the pointer to transfer to C++ ownership
		* @return   void
		*/
		static void		TransferOwerToC( SQUserPointer ptr ); 

		/**
		* ReleaseToSquirrel 
		*
		* @param 	SQUserPointer * ptr
		* @return   void
		*/
		static hBool		ReleaseToSquirrel( SQUserPointer ptr );

	private:

		struct SquirrelObject
		{
			HSQUIRRELVM		vm_;
			HSQOBJECT		obj_;
			//TODO: internal ref count

			bool operator == ( const SquirrelObject& b ) const
			{
				return vm_ == b.vm_ && obj_._unVal.raw == b.obj_._unVal.raw;
			}
		};

		typedef list< SquirrelObject > SqObjectList;

		static HSQUIRRELVM		globalVM_;//< the main squirrel virtual machine

		static SqObjectList*	ownedObjects_;
	};
}
}

#define sqbind_define_noncopyable( x ) \
template<>\
struct SqBindAllocator< x > {\
	static x *construct() { return NULL; /* make it not able to construct */ }\
	static SQBIND_INLINE x *copy_construct(const x* p_from) { return NULL; /* make it not able to copy-construct */ }\
	static SQBIND_INLINE bool assign(x* p_val, const x* p_from) { return false; /* make it not able to assign*/ }\
	static SQBIND_INLINE void destruct(x* p_instance) { /* make it ignore destruction */}\
	static SQBIND_INLINE x& get_empty() { /* if someone tries to assign, this will crash. however, this will likely never be called anyway. */ \
			static x *crashplease=NULL; return *crashplease; }\
}

#endif // SQUIRRELWRAPPER_H__