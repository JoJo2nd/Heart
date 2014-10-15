/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef HRESOURCESTREAM_H__
#define HRESOURCESTREAM_H__

namespace Heart
{
    template<typename _ty>
    void serialiseToStreamWithSizeHeader(const _ty& object, google::protobuf::io::CodedOutputStream* stream) {
        stream->WriteVarint32(object.ByteSize());
        object.SerializeToCodedStream(stream);
    }
}

#endif // HRESOURCESTREAM_H__