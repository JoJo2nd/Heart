/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_EVENTS_H
#define GWEN_EVENTS_H

#include "Gwen/GwenList.h"
#include "Gwen/Exports.h"
#include "Gwen/Structures.h"

namespace Gwen 
{
	namespace Controls
	{
		class Base;
	}

	namespace Event
	{

		class Caller;

		// A class must be derived from this 
		class GWEN_EXPORT Handler
		{
            GWEN_OverrideNew();
			public:

				Handler();
				virtual ~Handler();

				void RegisterCaller( Caller* );
				void UnRegisterCaller( Caller* );

			protected:

				void CleanLinks();
                list_t<Caller*>::type	m_Callers;

			public:

				typedef void (Handler::*Function)( Gwen::Controls::Base* pFromPanel );
				typedef void (Handler::*FunctionWithControl)( Gwen::Controls::Base* pFromPanel, Gwen::Controls::Base* pStoredPanel );
				typedef void (Handler::*FunctionBlank)();
				typedef void (Handler::*FunctionStr)( const Gwen::String& string );

		};



		//
		//
		//
		class GWEN_EXPORT Caller
		{
            GWEN_OverrideNew();
			public:

				Caller();
				~Caller();

				void Call( Controls::Base* pThis );

				template <typename T>
				void Add( Event::Handler* ob, T f )
				{
					AddInternal( ob, static_cast<Handler::Function>(f) );
				}

				template <typename T>
				void Add( Event::Handler* ob, T f, Controls::Base* pObj )
				{
					AddInternal( ob, static_cast<Handler::FunctionWithControl>(f), pObj );
				}

				template <typename T>
				void Add( Event::Handler* ob, void (T::*f)() )
				{
					AddInternal( ob, static_cast<Handler::FunctionBlank>(f) );
				}

				void RemoveHandler( Event::Handler* pObject );

			protected:

				void CleanLinks();
				void AddInternal( Event::Handler* pObject, Handler::Function pFunction );
				void AddInternal( Event::Handler* pObject, Handler::FunctionWithControl pFunction, Gwen::Controls::Base* pControl );
				void AddInternal( Event::Handler* pObject, Handler::FunctionBlank pFunction );

				struct handler
				{
					handler()
					{
						fnFunction = NULL;
						fnFunctionWithPanel = NULL;
						fnFunctionBlank = NULL;
						pObject = NULL;
						pControl = NULL;
					}

					Handler::Function				fnFunction;
					Handler::FunctionWithControl	fnFunctionWithPanel;
					Handler::FunctionBlank			fnFunctionBlank;

					

					Event::Handler*			pObject;

					// Available Data Slots
					Gwen::Controls::Base*	pControl;
				};

                list_t<handler>::type m_Handlers;
		};

	}

}
#endif
