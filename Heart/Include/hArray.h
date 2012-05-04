/********************************************************************

	filename: 	hArray.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef huArray_h__
#define huArray_h__

namespace Heart
{
	template< class _Ty, hUint32 ArraySize > 
	class hArray
	{
	public: 

        typedef _Ty* TypePtr;
        typedef const _Ty* ConstTypePtr;

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

        ConstTypePtr GetBuffer() const
        {
            return values_;
        }

        hUint32 GetMaxSize() const { return ArraySize; }

        operator TypePtr ()
        {
            return values_;
        }

        operator ConstTypePtr ()
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
			for ( hUint32 i = size; i < size_; ++i )
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