/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HGEOMETRYOBJECT_H__
#define HGEOMETRYOBJECT_H__

namespace Heart
{

    class hGeometryObject
    {
    public:
        hGeometryObject() {}
        ~hGeometryObject() {}

    private:

        hVector< hRenderable >      renderables_;
    };
}

#endif // HGEOMETRYOBJECT_H__