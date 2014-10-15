/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef HNETHOST_H__
#define HNETHOST_H__

#include "enet/enet.h"
#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "network/hNetDataStructs.h"
#include <vector>
#include <map>

namespace Heart
{

    typedef hUint hNetCommandSequence;
    typedef hUintptr_t hNetPeerID;

    enum hDispatchOptions {
        eDispatchReliable       = 1 << 1,
        eDispatchUnsequenced    = 1 << 2,
        eDispatchReport         = 1 << 3,
        eDispatchCommand        = 1 << 4,
        eDispatchResult         = 1 << 5,
    };

    hFUNCTOR_TYPEDEF(void (*)(hNetPeerID, hNetCommandSequence, const void*, hUint), hNetPacketHandleCallback);

    class hNetHost
    {
    public:
        hNetHost()
            : host_(hNullptr)
        {}
        ~hNetHost() {
            destroy();
        }

        hBool           initialise(hUint port);
        void            destroy();
        void            service();
        void            dispatchCommand(const hChar* command, hUint options, const void* data, hUint datasize); //< commands can expect a result. May have to install a callback for the result?
        void            dispatchReport(const hChar* command, hUint options, const void* data, hUint datasize);
        void            dispatchResult(hNetCommandSequence seq, hUint options, const void* data, hUint datasize);
        void            registerPacketHandler(const hChar* packetid, hNetPacketHandleCallback cb);
        void            unregisterPacketHandler(const hChar* packetid, hNetPacketHandleCallback cb);
        void            printDebugInfo(hFloat screenwidth, hFloat screenheight);
        const hChar*    getIPstring() const { return ipstring_; }

    private:
        hNetHost(const hNetHost& rhs);
        hNetHost& operator = (const hNetHost& rhs);

        typedef std::vector<ENetPeer*> PeerArray;
        typedef std::multimap<hUint32, hNetPacketHandleCallback> HandlerMultiMap;
        typedef std::multimap<hUint32, ENetPeer*> PeerMultiMap;

        hBool       hasValidPeerRange(hUint32 crc) const;
        ENetPacket* preparePacket(hNetPacketHeader::typeID type, hUint options, hUint32 cmdcrc, const void* data, hUint datasize);
        hBool       dispatchPacketToPeers(hUint32 hdrcrc, ENetPacket* packet);
        hBool       receivePacket(ENetPeer* peer, ENetPacket* packet);
        void        removeListeningPeer(hNetPeerID peerid, hUint32 serviceHash);

        //basic packet handlers
        void handleRegisterMessage(hNetPeerID peerid, hNetCommandSequence resultid, const void* data, hUint size);
        void handleUnregisterMessage(hNetPeerID peerid, hNetCommandSequence resultid, const void* data, hUint size);

        static hNetPeerID enetPeerToPeerID(ENetPeer* peer) {
            return (hNetPeerID)peer;
        }
        static ENetPeer*  peerIDToEnetPeer(hNetPeerID id) {
            return (ENetPeer*)id;
        }

        ENetAddress     address_;
        ENetAddress     thisaddress_;
        ENetHost*       host_;
        PeerArray       peers_;
        PeerMultiMap    listeningPeers_;
        HandlerMultiMap handlerMap_;
        hUint           commandCounter_;
        hChar           ipstring_[64];
    };
}

#endif // HNETHOST_H__