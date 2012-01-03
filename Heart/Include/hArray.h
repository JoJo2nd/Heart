/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   19:37
	filename: 	huArray.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef huArray_h__
#define huArray_h__

#include "hTypes.h"
#include "hDebugMacros.h"

namespace Heart
{
	template< class _Ty, hUint32 ArraySize > 
	class hArray
	{
	public: 

        typedef _Ty* TypePtr;

		_Ty& operator [] ( hUint32 i )
		{
			hcAssert( i < ArraySize );
			return values_[ i ];
		}

		const _Ty& operator [] ( hUint32 i ) const
		{
			hcAssert( i < ArraySize );
			return values_[ i ];
		}

        TypePtr GetBuffer() 
        {
            return values_;
        }

        const TypePtr GetBuffer() const
        {
            return values_;
        }

	private:

        template< typename _Ty, hUint32 arraySize >
        friend void SerialiseMethod( Heart::hSerialiser* , const Heart::hArray< _Ty, arraySize >& );
        template< typename _Ty, hUint32 arraySize >
        friend void DeserialiseMethod( Heart::hSerialiser* , const Heart::hArray< _Ty, arraySize >& );

		_Ty values_[ ArraySize ];
	};

	template< class _Ty, hUint32 _Granularity = 8 > 
	class hVector
	{
	public: 
        typedef _Ty* TypePtr;

		hVector()
			: values_( NULL )
			, size_( 0 )
			, reserve_( 0 )
		{}
		~hVector()
		{
			Clear();
		}

        template< typename _Uy, hUint32 _OtherGranularity >
        void CopyTo( hVector< _Uy, _OtherGranularity >* rhs ) const
        {
            rhs->Reserve( size_ );
            for ( hUint32 i = 0; i < size_; ++i )
            {
                rhs->PushBack( values_[i] );
            }
        }

		hFORCEINLINE _Ty& operator [] ( const hUint32 i )
		{
			hcAssert( i >= 0 && i < size_ );
			return values_[ i ];
		}

		hFORCEINLINE const _Ty& operator [] ( const hUint32 i ) const
		{
			hcAssert( i>= 0 && i < size_ );
			return values_[ i ];
		}

		void Reserve( hUint32 size )
		{
			if ( size > reserve_ )
			{
				reserve_ = hAlign( size, _Granularity );
				hcAssert( reserve_ >= size );
				values_ = (_Ty*)hRealloc( values_, sizeof(_Ty)*reserve_ );
			}
		}

		void Resize( hUint32 size )
		{
			if ( size > size_ )
				Grow( size );
			else
				Shrink( size );
		}

		void PushBack( const _Ty& val )
		{
			Resize( size_ + 1 );
			values_[size_-1] = val;
		}

		void Clear() 
		{
			Shrink( 0 );
			hFree( values_ );
			values_ = NULL;
			reserve_ = 0;
		}

		hUint32      GetSize() const { return size_; }
		hUint32      GetReserve() const { return reserve_; }

		operator TypePtr () 
		{
			return values_;
		}

		operator const TypePtr () const
		{
			return values_;
		}

        TypePtr       GetBuffer() { return values_; }
        const TypePtr GetBuffer() const { return values_; }

	private:

        template< typename _Ty >
        friend void SerialiseMethod( Heart::hSerialiser* , const Heart::hVector< _Ty >& );
        template< typename _Ty >
        friend void DeserialiseMethod( Heart::hSerialiser* , Heart::hVector< _Ty >& );

        // Hidden to prevent mistaken copies. If a copy is need use explict Copy() function
        hVector( const hVector& rhs )
        {
            rhs.CopyTo( this );
        }
        hVector& operator = ( const hVector& rhs )
        {
            rhs.CopyTo( this );

            return *this;
        }
		
		void Grow( hUint32 size ) 
		{
			Reserve( size );
			//Construct
			for ( hUint32 i = size_; i < size; ++i )
				new ( values_+i ) _Ty();
			size_ = size;
		}

		void Shrink( hUint32 size )
		{
			//Destruct
			for ( hUint32 i = size-1; i < size_; --i )
				(values_+i)->~_Ty();
			size_ = size;
		}

        TypePtr	values_;
		hUint32 size_;
		hUint32 reserve_;
	};

    template< typename _Ty >
    inline void SerialiseMethod( Heart::hSerialiser* ser, const Heart::hVector< _Ty >& data )
    {
        SERIALISE_ELEMENT( data.size_ );
        SERIALISE_ELEMENT_COUNT( data.values_, data.size_ );
    }

    template< typename _Ty >
    inline void DeserialiseMethod( Heart::hSerialiser* ser, Heart::hVector< _Ty >& data )
    {
        DESERIALISE_ELEMENT( data.size_ );
        DESERIALISE_ELEMENT( data.values_ );
        data.reserve_ = data.size_;
    }

    template< typename _Ty, hUint32 arraySize >
    inline void SerialiseMethod( Heart::hSerialiser* ser, const Heart::hArray< _Ty, arraySize >& data )
    {
        SERIALISE_ELEMENT( data.values_ );
    }

    template< typename _Ty, hUint32 arraySize >
    inline void DeserialiseMethod( Heart::hSerialiser* ser, Heart::hArray< _Ty, arraySize >& data )
    {
        DESERIALISE_ELEMENT( data.values_ );
    }

}

#endif // huArray_h__