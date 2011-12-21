/********************************************************************

	filename: 	EventDispathcer.h	
	
	Copyright (c) 28:10:2011 James Moran
	
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

#ifndef EVENTDISPATHCER_H__
#define EVENTDISPATHCER_H__

struct Event
{
public:
    Event( const pChar* name, const void* data, pUint32 size ) 
        : user_( data )
        , size_( size )
    {
        strcpy_s( evtName_, 16, name );
    }
    Event( const Event& rhs, const void* data, pUint32 size ) 
        : user_( data )
        , size_( size )
    {
        id_[0] = rhs.id_[0];
        id_[1] = rhs.id_[1];
    }
    const pChar*    GetEventName() const { return evtName_; };
    const void*     GetEventData() const { return user_; }

    pBool operator == ( const Event& rhs ) const
    {
        return id_[0] == rhs.id_[0] && id_[1] == rhs.id_[1];
    }

private:
    union 
    {
        pUint64     id_[2];
        pChar       evtName_[16];
    };
    pUint32         size_; 
    const void*     user_;
};

#define EVT_DEFINE_EVENT( x ) static Event EVT_##x ( #x, NULL, 0 )
//#define EVT_DECLARE_EVENT( x ) Event EVT_##x ( x, NULL, 0 )
#define EVT_EVENT( x ) (EVT_##x)

class EventListener;

class EventDispatcher
{
public:
    EventDispatcher()
    {

    }
    ~EventDispatcher()
    {

    }

    void RegisterListener( EventListener* listener );
    void UnregisterListener( EventListener* listener );
    void DispatchEvent( const Event& evtName, const void* data, pUint32 len );

private:

    typedef std::list< EventListener* > ListenerList_t;

    ListenerList_t  listenerList_;
};

class EventListener
{
public:
    EventListener() 
        : eventDispatcher_(NULL)
    {
    }
    virtual void EventRecieve( const Event& evt ) = 0;
    void         UnregisterListener();
private:
    friend class EventDispatcher;

    EventDispatcher* eventDispatcher_;
};

#endif // EVENTDISPATHCER_H__