/********************************************************************

    filename:   hNetHost.cpp  
    
    Copyright (c) 20:9:2013 James Moran
    
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

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hNetHost::initialise(hUint port) {
        registerPacketHandler("service.register", hFUNCTOR_BINDMEMBER(hNetPacketHandleCallback, hNetHost, handleRegisterMessage, this));
        registerPacketHandler("service.unregister", hFUNCTOR_BINDMEMBER(hNetPacketHandleCallback, hNetHost, handleUnregisterMessage, this));
        address_.host=ENET_HOST_ANY;
        address_.port=port;
        host_=enet_host_create(&address_, 32/*peers*/, 1/*channels*/, 0/*in b/w*/, 0/*out b/w*/);
        if (!host_) {
            return hFalse;
        }
        peers_.reserve(32);
        enet_address_get_host(&address_, ipstring_, (hUint)hStaticArraySize(ipstring_));
        enet_address_set_host(&thisaddress_, ipstring_);
        enet_address_get_host_ip(&thisaddress_, ipstring_, (hUint)hStaticArraySize(ipstring_));
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::destroy() {
        enet_host_destroy(host_);
        host_=hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::printDebugInfo(hFloat screenwidth, hFloat screenheight) {
        hChar debugstring[128];
        hVec3 screenpos((screenwidth/2.f)-250.f, (screenheight/2.f)-32.f, 0.f);
        hColour textcolour(1.f, 1.f, 1.f, 1.f);
        hDebugDraw* dd=hDebugDraw::it();

        dd->begin();
        hStrPrintf(debugstring, (hUint)hStaticArraySize(debugstring), "Server IP[%s:%u]", ipstring_, address_.port);
        dd->drawText(screenpos, debugstring, textcolour);
        dd->end();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::service() {
        if (!host_) {
            return;
        }
        ENetEvent enetevent;

        while (enet_host_service(host_, &enetevent, 0) > 0) {
            switch(enetevent.type) {
            case ENET_EVENT_TYPE_NONE: break;
            case ENET_EVENT_TYPE_CONNECT: {
                hUint address=enetevent.peer->address.host;
                hcPrintf("Connection from %u.%u.%u.%u : %u", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24, enetevent.peer->address.port);
                peers_.push_back(enetevent.peer);
            } break;
            case ENET_EVENT_TYPE_DISCONNECT: {
                hUint address=enetevent.peer->address.host;
                hcPrintf("Disconnect from %u.%u.%u.%u : %u", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24, enetevent.peer->address.port);
                for (hUint i=0, n=(hUint)peers_.size(); i<n; ++i) {
                    if (peers_[i]==enetevent.peer) {
                        peers_[i]=peers_[n-1];
                        break;
                    }
                }
            } break;
            case ENET_EVENT_TYPE_RECEIVE: {
                if (enetevent.packet) {
                    receivePacket(enetevent.peer, enetevent.packet);
                    enet_packet_destroy(enetevent.packet);
                }
            } break;
            default: break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::registerPacketHandler(const hChar* packetid, hNetPacketHandleCallback cb) {
        hUint32 packetidcrc=hCRC32::StringCRC(packetid);
        handlerMap_.insert(std::pair<hUint32, hNetPacketHandleCallback>(packetidcrc, cb));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::unregisterPacketHandler(const hChar* packetid, hNetPacketHandleCallback cb) {
        hUint32 packetidcrc=hCRC32::StringCRC(packetid);
        auto range=handlerMap_.equal_range(packetidcrc);
        for (auto i=range.first; i!=range.second; ++i) {
            if (i->second==cb) {
                handlerMap_.erase(i);
                break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hNetHost::receivePacket(ENetPeer* peer, ENetPacket* packet) {
        hNetPacketHeader* header=(hNetPacketHeader*)packet->data;
        void* data=(void*)(((hByte*)packet->data)+hNetPacketHeader::s_packetHeaderByteSize);
        hUint datasize=(hUint)((hPtrdiff_t)packet->dataLength-hNetPacketHeader::s_packetHeaderByteSize);
        if (header->type_==hNetPacketHeader::eCommand) {
            auto range=handlerMap_.equal_range(header->nameID_);
            if (range.first == range.second) {
                return hFalse;
            }
            for (auto i=range.first; i!=range.second; ++i) {
                i->second(enetPeerToPeerID(peer), header->seqID_, data, datasize);
            }
            return hTrue;
        }
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hNetHost::dispatchPacketToPeers(hUint32 hdrcrc, ENetPacket* packet) {
        hBool sentsomewhere=hFalse;
        auto range=listeningPeers_.equal_range(hdrcrc);
        for (auto i=range.first; i!=range.second; ++i) {
            enet_peer_send(i->second, 0, packet);
            sentsomewhere=hTrue;
        }
        return sentsomewhere;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::dispatchCommand(const hChar* command, hUint options, const void* data, hUint datasize) {
        hcAssertMsg(command && data, "Command & data cannot be NULL");
        hcAssertMsg(datasize > 0, "Data must have a size");

        hUint32 cmdcrc=hCRC32::StringCRC(command);
        if (hasValidPeerRange(cmdcrc)) {
            ENetPacket* packet=preparePacket(hNetPacketHeader::eCommand, options, cmdcrc, data, datasize);
            dispatchPacketToPeers(cmdcrc, packet);
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::dispatchReport(const hChar* command, hUint options, const void* data, hUint datasize) {
        hcAssertMsg(command && data, "Command & data cannot be NULL");
        hcAssertMsg(datasize > 0, "Data must have a size");

        hUint32 cmdcrc=hCRC32::StringCRC(command);
        if (hasValidPeerRange(cmdcrc)) {
            ENetPacket* packet=preparePacket(hNetPacketHeader::eReport, options, cmdcrc, data, datasize);
            dispatchPacketToPeers(cmdcrc, packet);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hNetHost::hasValidPeerRange(hUint32 crc) const {
        auto range=handlerMap_.equal_range(crc);
        if (range.first == range.second) {
            return hFalse;
        }
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    ENetPacket* hNetHost::preparePacket(hNetPacketHeader::typeID type, hUint options, hUint32 cmdcrc, const void* data, hUint datasize) {
        hUint flags = 0;
        flags |= options & eDispatchReliable ? ENET_PACKET_FLAG_RELIABLE : 0;
        flags |= options & eDispatchUnsequenced ? ENET_PACKET_FLAG_UNSEQUENCED : 0;
        ENetPacket* packet=enet_packet_create(hNullptr, datasize+hNetPacketHeader::s_packetHeaderByteSize, flags);
        hNetPacketHeader* hdr=(hNetPacketHeader*)packet->data;
        hdr->type_ =type;
        hdr->seqID_=commandCounter_;
        hdr->nameID_=cmdcrc;
        hMemCpy(((hByte*)packet->data)+hNetPacketHeader::s_packetHeaderByteSize, data, datasize);
        return packet;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::handleRegisterMessage(hNetPeerID peerid, hNetCommandSequence resultid, const void* data, hUint size) {
        proto::ServiceRegister msg;
        msg.ParseFromArray(data, size);
        if (msg.has_sevicename()) {
            listeningPeers_.insert(std::pair<hUint32, ENetPeer*>(hCRC32::StringCRC(msg.sevicename().c_str()), peerIDToEnetPeer(peerid)));
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::handleUnregisterMessage(hNetPeerID peerid, hNetCommandSequence resultid, const void* data, hUint size) {
        google::protobuf::io::ArrayInputStream instream(data, size);
        proto::ServiceUnregister msg;
        msg.ParseFromZeroCopyStream(&instream);
        if (msg.has_sevicename()) {
            hUint32 serviceHash=hCRC32::StringCRC(msg.sevicename().c_str());
            removeListeningPeer(peerid, serviceHash);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hNetHost::removeListeningPeer(hNetPeerID peerid, hUint32 serviceHash) {
        ENetPeer* peer=peerIDToEnetPeer(peerid);
        hBool notdone=hTrue;
        while (notdone){
            auto range=listeningPeers_.equal_range(serviceHash);
            notdone=hFalse;
            for (auto i=range.first; i!=range.second; ++i) {
                if (i->second==peer) {
                    listeningPeers_.erase(i);
                    notdone=hTrue;
                    break;
                }
            }
        }
    }

}