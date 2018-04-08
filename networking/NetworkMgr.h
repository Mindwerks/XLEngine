#ifndef NETWORKMGR_H
#define NETWORKMGR_H

#include "../CommonTypes.h"
#include "../math/Vector4.h"

class Engine;

//static input class
//the Windowing system (Win API on Windows XP/Vista/7, Cocoa on OS X) passes the keyboard and mouse
//messages to this class, which is then used by the game systems.
class NetworkMgr {
public:
    enum ChatMsgType_e {
        CHATMSG_NORMAL = 0,
        CHATMSG_SYS_INFO,
        CHATMSG_SYS_ERROR,
        CHATMSG_COUNT
    };
public:
    //init and destroy, done by the OS layer.
    static bool Init(Engine *pEngine);

    static void Destroy();

    static bool CreateServer();

    static bool CreateClient();

    static void SetLocalPlayerName(const char *pszName);

    static void SendChatMessage(const char *pszMsg, uint8_t uMsgType = CHATMSG_NORMAL);

    static bool HasRecievedStartupMsg();

    static void Loop();

private:
    static void _ServerLoop();

    static void _ClientLoop();

    static void SendPacket_Client(uint32_t uChannel, const uint8_t *data, uint32_t uDataSize);

    static void SendPacket_Server(uint32_t uChannel, uint32_t uClientID, const uint8_t *data, uint32_t uDataSize);

    //move this somewhere?
    static void Client_ProcessPacket(uint8_t type, const uint8_t *data, uint32_t dataSize, uint32_t channel);

    static void
    Server_ProcessPacket(int32_t nClientID, uint8_t type, const uint8_t *data, uint32_t dataSize, uint32_t channel);

    static uint8_t *SetupPacket(uint8_t *packet, uint8_t type);

    static uint8_t *UnpackPacket(uint8_t *packet, uint32_t packetSize, uint32_t &dataSize);

private:
    static int32_t m_nPlayerCount;
    static bool m_bRecievedStartupMsg;
    static char m_szLocalPlayer[32];
    static Engine *m_pEngine;
    static Vector4 m_aMsgColorTable[];
    static uint32_t m_SeqNum;
    static uint8_t m_PacketType;
    static uint32_t m_PeerSeqNum;
};

#endif    //NETWORKMGR_H
