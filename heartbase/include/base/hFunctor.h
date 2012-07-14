//----------------------------------------------------------------------------
//
//
//  Title		:	huFunctor.h
//  Desc		:	Function object.
//  Author		:	Neil Richardson, James Moran & Autogeneration Script
//
//  Usage       :
//
//	These are used much like function pointers, but allow classes to register
//	their methods, without any of the horrid syntax. To declare a functor, first
//	it is best to typedef huFunctor:
//
//	typedef huFunctor< void(*)( int ) >::type TMyFunctor;
//
//	This functor will take functions with a "void function(int a)" signature.
//	Finally, an example of usage:
//
//	extern void myTestFunc( int a );
//
//	TMyFunctor myFunctor;
//
//	myFunctor = TMyFunctor::bind< myTestFunc > ();		// Bind myTestFunc to the functor.
//	myFunctor( 1 );										// Call the functor, redirects to myTestFunc
//
//	class MyTestClass
//	{
//		void myTestMethod( int a )
//	};
//
//	MyTestClass ClassInstance;
//
//	// Bind myTestMethod for ClassInstance to myFunctor. This removes previous binding.
//	myFunctor = TMyFunctor::bind< MyTestClass, &MyTestClass::myTestMethod >( &ClassInstance );
//
//----------------------------------------------------------------------------
#ifndef __huFUNCTOR_H__
#define __huFUNCTOR_H__

#include "hTypes.h"

#include "hDebugMacros.h"

//////////////////////////////////////////////////////////////////////////
// huFuncTraits

template< typename _Fn >
struct huFuncTraits;

template< typename _R >
struct huFuncTraits< _R(*)() >
{
	static const int PARAMS = 0;
	typedef _R return_type;
	typedef _R(*signature_type)();
};

template< typename _R, typename _P0 >
struct huFuncTraits< _R(*)(_P0) >
{
	static const int PARAMS = 1;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _R(*signature_type)(_P0);
};

template< typename _R, typename _P0, typename _P1 >
struct huFuncTraits< _R(*)(_P0, _P1) >
{
	static const int PARAMS = 2;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _R(*signature_type)(_P0, _P1);
};

template< typename _R, typename _P0, typename _P1, typename _P2 >
struct huFuncTraits< _R(*)(_P0, _P1, _P2) >
{
	static const int PARAMS = 3;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _R(*signature_type)(_P0, _P1, _P2);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3 >
struct huFuncTraits< _R(*)(_P0, _P1, _P2, _P3) >
{
	static const int PARAMS = 4;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4 >
struct huFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4) >
{
	static const int PARAMS = 5;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
struct huFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4, _P5) >
{
	static const int PARAMS = 6;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6 >
struct huFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6) >
{
	static const int PARAMS = 7;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6, typename _P7 >
struct huFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7) >
{
	static const int PARAMS = 8;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _P7 param7_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7);
};

template< typename _Ty, typename _R >
struct huFuncTraits< _R(_Ty::*)() >
{
	static const int PARAMS = 0;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _R(*signature_type)();
};

template< typename _Ty, typename _R, typename _P0 >
struct huFuncTraits< _R(_Ty::*)(_P0) >
{
	static const int PARAMS = 1;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _R(*signature_type)(_P0);
};

template< typename _Ty, typename _R, typename _P0, typename _P1 >
struct huFuncTraits< _R(_Ty::*)(_P0, _P1) >
{
	static const int PARAMS = 2;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _R(*signature_type)(_P0, _P1);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2 >
struct huFuncTraits< _R(_Ty::*)(_P0, _P1, _P2) >
{
	static const int PARAMS = 3;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _R(*signature_type)(_P0, _P1, _P2);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3 >
struct huFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3) >
{
	static const int PARAMS = 4;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4 >
struct huFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4) >
{
	static const int PARAMS = 5;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
struct huFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4, _P5) >
{
	static const int PARAMS = 6;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6 >
struct huFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6) >
{
	static const int PARAMS = 7;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6, typename _P7 >
struct huFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7) >
{
	static const int PARAMS = 8;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _P7 param7_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7);
};

//////////////////////////////////////////////////////////////////////////
// _huFunctor

template< typename _Fn, int >class _huFunctor;template< typename _Fn >
class _huFunctor< _Fn, 0 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()()
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 0 > bind()
	{
		_huFunctor< _Fn, 0 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)() >
	static _huFunctor< _Fn, 0 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 0 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void* )
	{
		return (*func)(  );
	}

	template< class _Ty, return_type (_Ty::*meth)() >
	static return_type method_stub( void* pObj )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)(  );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 1 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 1 > bind()
	{
		_huFunctor< _Fn, 1 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type) >
	static _huFunctor< _Fn, 1 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 1 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0 )
	{
		return (*func)( P0 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 2 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::param1_type param1_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0, P1);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 2 > bind()
	{
		_huFunctor< _Fn, 2 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type) >
	static _huFunctor< _Fn, 2 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 2 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1 )
	{
		return (*func)( P0, P1 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 3 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::param1_type param1_type;
	typedef typename huFuncTraits< _Fn >::param2_type param2_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0, P1, P2);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 3 > bind()
	{
		_huFunctor< _Fn, 3 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type) >
	static _huFunctor< _Fn, 3 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 3 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2 )
	{
		return (*func)( P0, P1, P2 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 4 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::param1_type param1_type;
	typedef typename huFuncTraits< _Fn >::param2_type param2_type;
	typedef typename huFuncTraits< _Fn >::param3_type param3_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0, P1, P2, P3);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 4 > bind()
	{
		_huFunctor< _Fn, 4 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type) >
	static _huFunctor< _Fn, 4 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 4 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3 )
	{
		return (*func)( P0, P1, P2, P3 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 5 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::param1_type param1_type;
	typedef typename huFuncTraits< _Fn >::param2_type param2_type;
	typedef typename huFuncTraits< _Fn >::param3_type param3_type;
	typedef typename huFuncTraits< _Fn >::param4_type param4_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0, P1, P2, P3, P4);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 5 > bind()
	{
		_huFunctor< _Fn, 5 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type) >
	static _huFunctor< _Fn, 5 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 5 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4 )
	{
		return (*func)( P0, P1, P2, P3, P4 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 6 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::param1_type param1_type;
	typedef typename huFuncTraits< _Fn >::param2_type param2_type;
	typedef typename huFuncTraits< _Fn >::param3_type param3_type;
	typedef typename huFuncTraits< _Fn >::param4_type param4_type;
	typedef typename huFuncTraits< _Fn >::param5_type param5_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0, P1, P2, P3, P4, P5);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 6 > bind()
	{
		_huFunctor< _Fn, 6 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type) >
	static _huFunctor< _Fn, 6 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 6 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5 )
	{
		return (*func)( P0, P1, P2, P3, P4, P5 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4, P5 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 7 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::param1_type param1_type;
	typedef typename huFuncTraits< _Fn >::param2_type param2_type;
	typedef typename huFuncTraits< _Fn >::param3_type param3_type;
	typedef typename huFuncTraits< _Fn >::param4_type param4_type;
	typedef typename huFuncTraits< _Fn >::param5_type param5_type;
	typedef typename huFuncTraits< _Fn >::param6_type param6_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type, typename huFuncTraits< _Fn >::param6_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5, typename huFuncTraits< _Fn >::param6_type P6)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0, P1, P2, P3, P4, P5, P6);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 7 > bind()
	{
		_huFunctor< _Fn, 7 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type, typename huFuncTraits< _Fn >::param6_type) >
	static _huFunctor< _Fn, 7 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 7 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5, typename huFuncTraits< _Fn >::param6_type P6 )
	{
		return (*func)( P0, P1, P2, P3, P4, P5, P6 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type, typename huFuncTraits< _Fn >::param6_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5, typename huFuncTraits< _Fn >::param6_type P6 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4, P5, P6 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _huFunctor< _Fn, 8 >
{
public:
	typedef typename huFuncTraits< _Fn >::return_type return_type;
	typedef typename huFuncTraits< _Fn >::param0_type param0_type;
	typedef typename huFuncTraits< _Fn >::param1_type param1_type;
	typedef typename huFuncTraits< _Fn >::param2_type param2_type;
	typedef typename huFuncTraits< _Fn >::param3_type param3_type;
	typedef typename huFuncTraits< _Fn >::param4_type param4_type;
	typedef typename huFuncTraits< _Fn >::param5_type param5_type;
	typedef typename huFuncTraits< _Fn >::param6_type param6_type;
	typedef typename huFuncTraits< _Fn >::param7_type param7_type;
	typedef typename huFuncTraits< _Fn >::return_type(*stub_func)(void*,typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type, typename huFuncTraits< _Fn >::param6_type, typename huFuncTraits< _Fn >::param7_type);
public:
	inline _huFunctor(): pThis_( NULL ), stubFunc_( NULL ){};
	inline return_type operator()(typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5, typename huFuncTraits< _Fn >::param6_type P6, typename huFuncTraits< _Fn >::param7_type P7)
	{
		hcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pThis_, P0, P1, P2, P3, P4, P5, P6, P7);
	}

	inline hBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	template< _Fn _func >
	static _huFunctor< _Fn, 8 > bind()
	{
		_huFunctor< _Fn, 8 > Func;
		Func.pThis_ = NULL;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type, typename huFuncTraits< _Fn >::param6_type, typename huFuncTraits< _Fn >::param7_type) >
	static _huFunctor< _Fn, 8 > bind( _Ty* pThis )
	{
		_huFunctor< _Fn, 8 > Func;
		Func.pThis_ = pThis;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		hcAssert( pThis != NULL );
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5, typename huFuncTraits< _Fn >::param6_type P6, typename huFuncTraits< _Fn >::param7_type P7 )
	{
		return (*func)( P0, P1, P2, P3, P4, P5, P6, P7 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename huFuncTraits< _Fn >::param0_type, typename huFuncTraits< _Fn >::param1_type, typename huFuncTraits< _Fn >::param2_type, typename huFuncTraits< _Fn >::param3_type, typename huFuncTraits< _Fn >::param4_type, typename huFuncTraits< _Fn >::param5_type, typename huFuncTraits< _Fn >::param6_type, typename huFuncTraits< _Fn >::param7_type) >
	static return_type method_stub( void* pObj, typename huFuncTraits< _Fn >::param0_type P0, typename huFuncTraits< _Fn >::param1_type P1, typename huFuncTraits< _Fn >::param2_type P2, typename huFuncTraits< _Fn >::param3_type P3, typename huFuncTraits< _Fn >::param4_type P4, typename huFuncTraits< _Fn >::param5_type P5, typename huFuncTraits< _Fn >::param6_type P6, typename huFuncTraits< _Fn >::param7_type P7 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4, P5, P6, P7 );
	}

private:
	void* pThis_;
	stub_func stubFunc_;
};

template< typename _Fn >
struct huFunctor
{
	typedef _huFunctor< _Fn, huFuncTraits< _Fn >::PARAMS > type;
};
#endif
