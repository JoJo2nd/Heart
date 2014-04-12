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
    class HEART_DLLEXPORT hArray
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

        _Ty& At(hUint32 i)
        {
            hcAssert( i < ArraySize );
            return values_[ i ];
        }

        const _Ty& At(hUint32 i) const
        {
            hcAssert( i < ArraySize );
            return values_[ i ];
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

        _Ty values_[ ArraySize ];
    };

    template< class _Ty, hSize_t _Granularity = 8 > 
    class HEART_DLLEXPORT hVector
    {
    public: 
        typedef hVector< _Ty, _Granularity > SelfType;
        typedef _Ty* TypePtr;

        hVector()
            : values_( NULL )
            , size_( 0 )
            , reserve_( 0 )
        {}
        hVector( const hVector& rhs )
        {
            rhs.copy_to( this );
        }
        hVector& operator = ( const hVector& rhs )
        {
            rhs.copy_to( this );
            return *this;
        }
        ~hVector()
        {
            clear();
            hFreeSafe(values_);
            reserve_ = 0;
        }

        template< typename _Uy, hUint32 _OtherGranularity >
        void copy_to( hVector< _Uy, _OtherGranularity >* rhs ) const
        {
            rhs->clear();
            rhs->reserve(size_);
            for ( hUint32 i = 0; i < size_; ++i )
            {
                rhs->push_back(values_[i]);
            }
        }

        template< typename _Uy, hUint32 _OtherGranularity >
        void swap( hVector< _Uy, _OtherGranularity >* rhs )
        {
            std::swap(values_, rhs->values_);
            std::swap(size_, rhs->size_);
            std::swap(reserve_, rhs->reserve_);
        }

        hFORCEINLINE _Ty& operator [] ( hSize_t i )
        {
            hcAssert( i >= 0 && i < size_ );
            return values_[ i ];
        }

        hFORCEINLINE const _Ty& operator [] ( hSize_t i ) const
        {
            hcAssert( i>= 0 && i < size_ );
            return values_[ i ];
        }

        void reserve( hUint32 size )
        {
            if ( size > reserve_ )
            {
                reserve_ = size;
                values_ = (_Ty*)hAlignRealloc(values_, hAlignOf(_Ty), sizeof(_Ty)*reserve_);
            }
        }

        void reserve_grow(hUint32 size) {
            reserve(size_+size);
        }

        void resize( hUint32 size )
        {
            if ( size > size_ )
                grow( size );
            else
                shrink( size );
        }

        void push_back( const _Ty& val )
        {
            grow(size_ + 1);
            values_[size_-1] = val;
        }

        void clear() {
            shrink( 0 );
        }

        hUint32      size() const { return size_; }

        operator TypePtr () 
        {
            return values_;
        }

        operator const TypePtr () const
        {
            return values_;
        }

        TypePtr       data() { return values_; }
        const TypePtr data() const { return values_; }

    private:
        
        void reserve_gran( hUint32 size )
        {
            if ( size > reserve_ ) {
                reserve_ = hAlign( size, _Granularity );
                hcAssert( reserve_ >= size );
                values_ = (_Ty*)hAlignRealloc(values_, hAlignOf(_Ty), sizeof(_Ty)*reserve_);
            }
        }

        void grow( hUint32 size ) 
        {
            reserve_gran( size );
            //Construct
            for ( hUint32 i = size_; i < size; ++i )
                new ( values_+i ) _Ty;
            size_ = size;
        }

        void shrink( hUint32 size )
        {
            //Destruct
            for ( hUint32 i = size; i < size_; ++i )
                (values_+i)->~_Ty();
            size_ = size;
        }

        TypePtr	            values_;
        hUint32             size_;
        hUint32             reserve_;
    };

}

#endif // huArray_h__