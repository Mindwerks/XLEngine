#ifndef NETWORKMGR_H
#define NETWORKMGR_H

#include "../CommonTypes.h"
#include "../math/Vector4.h"

class Engine;

//static input class
//the Windowing system (Win API on Windows XP/Vista/7, Cocoa on OS X) passes the keyboard and mouse
//messages to this class, which is then used by the game systems.
class NetworkMgr
{
public:
	enum ChatMsgType_e
	{
		CHATMSG_NORMAL=0,
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
	static void SendChatMessage(const char *pszMsg, u8 uMsgType=CHATMSG_NORMAL);

	static bool HasRecievedStartupMsg();
	static void Loop();
private:
	static void _ServerLoop();
	static void _ClientLoop();

	static void SendPacket_Client(u32 uChannel, const u8 *data, u32 uDataSize);
	static void SendPacket_Server(u32 uChannel, u32 uClientID, const u8 *data, u32 uDataSize);

	//move this somewhere?
	static void Client_ProcessPacket( u8 type, const u8 *data, u32 dataSize, u32 channel );
	static void Server_ProcessPacket( s32 nClientID, u8 type, const u8 *data, u32 dataSize, u32 channel );

	static u8 *SetupPacket(u8 *packet, u8 type);
	static u8 *UnpackPacket(u8 *packet, u32 packetSize, u32& dataSize);

private:
	static s32 m_nPlayerCount;
	static bool m_bRecievedStartupMsg;
	static char m_szLocalPlayer[32];
	static Engine *m_pEngine;
	static Vector4 m_aMsgColorTable[];
	static u32 m_SeqNum;
	static u8 m_PacketType;
	static u32 m_PeerSeqNum;
};

#endif	//NETWORKMGR_H
