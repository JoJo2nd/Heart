/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef huArray_h__
#define huArray_h__

#include "base/hTypes.h"
#include "base/hMemory.h"

namespace Heart
{
    template< class _Ty, hUint32 ArraySize > 
    class  hArray
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

    template< class _Ty, hSize_t _Granularity = 8ull > 
    class  hVector
    {
    public: 
        typedef hVector< _Ty, _Granularity > SelfType;
        typedef _Ty* TypePtr;
        typedef const _Ty constTy;

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
            hFree(values_);
            values_=nullptr;
            reserve_ = 0;
        }

        template< typename _Uy, hSize_t _OtherGranularity >
        void copy_to( hVector< _Uy, _OtherGranularity >* rhs ) const
        {
            rhs->clear();
            rhs->reserve(size_);
            for ( hUint32 i = 0; i < size_; ++i )
            {
                rhs->push_back(values_[i]);
            }
        }

        template< typename _Uy, hSize_t _OtherGranularity >
        void swap( hVector< _Uy, _OtherGranularity >* rhs )
        {
            std::swap(values_, rhs->values_);
            std::swap(size_, rhs->size_);
            std::swap(reserve_, rhs->reserve_);
        }

        hFORCEINLINE _Ty& operator [] ( hSize_t i )
        {
            hcAssert(i < size_);
            return values_[ i ];
        }

        hFORCEINLINE constTy& operator [] ( hSize_t i ) const
        {
            hcAssert(i < size_);
            return values_[ i ];
        }

        void reserve(hUint32 size)
        {
            if ( size > reserve_ )
            {
                reserve_ = size;
                values_ = (_Ty*)hRealloc(values_, sizeof(_Ty)*reserve_, hAlignOf(_Ty));
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
                values_ = (_Ty*)hRealloc(values_, sizeof(_Ty)*reserve_, hAlignOf(_Ty));
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