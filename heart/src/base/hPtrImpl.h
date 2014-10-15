/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef HPTRIMPL_H__
#define HPTRIMPL_H__

namespace Heart
{
    template< typename _Ty >
    class hPtrImpl
    {
    public:

        hPtrImpl() : pImpl_(NULL) {}
        explicit hPtrImpl( _Ty* pImpl ) : pImpl_( pImpl ) {}
        virtual ~hPtrImpl() { delete pImpl_; pImpl_ = nullptr; }

        hFORCEINLINE void SetImpl(_Ty* pImpl) { pImpl_ = pImpl; }
        hFORCEINLINE _Ty* pImpl() { return pImpl_; }
        hFORCEINLINE const _Ty* pImpl() const { return pImpl_; }

    private:

        _Ty* pImpl_;
    };
}

#endif // HPTRIMPL_H__