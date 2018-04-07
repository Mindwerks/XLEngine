#include "NetworkMgr.h"
#include "../Engine.h"
#include "../ui/XL_Console.h"
#include "../EngineSettings.h"
#include "../memory/ScratchPad.h"
#include "../world/ObjectManager.h"
#include <enet/enet.h>

#define MAX_PLAYERS 32

ENetHost *_pServer = NULL;
ENetHost *_pClient = NULL;
ENetPeer *_pServerPeer = NULL;

struct NetworkPlayer
{
	ENetPeer *peer;
	char szName[32];
};
NetworkPlayer _apPlayers[MAX_PLAYERS];

int32_t NetworkMgr::m_nPlayerCount;
bool NetworkMgr::m_bRecievedStartupMsg;
char NetworkMgr::m_szLocalPlayer[32];
Engine *NetworkMgr::m_pEngine;
uint32_t NetworkMgr::m_SeqNum;
uint8_t NetworkMgr::m_PacketType;
uint32_t NetworkMgr::m_PeerSeqNum;
bool m_bInitSeqNum;

Vector4 NetworkMgr::m_aMsgColorTable[] =
{
	Vector4(0.75f, 0.75f, 1.0f,  1.0f),
	Vector4(0.75f, 0.75f, 0.75f, 1.0f),
	Vector4(1.0f,  0.0f,  0.0f,  1.0f),
};

enum PacketType_e
{
	PTYPE_SET_MAP=0,	//set the initial map data and other starting parameters.
	PTYPE_CLIENT_DATA,
	PTYPE_CHAT_MSG,
	PTYPE_COUNT
};

//Game packet packing and unpacking.
#define GetPacketOverhead() (1 + sizeof(m_SeqNum))

uint8_t *NetworkMgr::SetupPacket(uint8_t *packet, uint8_t type)
{
	packet[0] = type;
	*((uint32_t *)&packet[1]) = m_SeqNum;

	return &packet[1+sizeof(m_SeqNum)];
}

uint8_t *NetworkMgr::UnpackPacket(uint8_t *packet, uint32_t packetSize, uint32_t& dataSize)
{
	uint32_t uOffset = 0;
	m_PacketType = packet[0];				uOffset++;
	m_PeerSeqNum = *((uint32_t *)&packet[1]);	uOffset += sizeof(uint32_t);
	dataSize = packetSize - GetPacketOverhead();
	
	return &packet[uOffset];
}

//Mgr Implementation.
bool NetworkMgr::Init(Engine *pEngine)
{
	bool bSucceeded = false;

	m_nPlayerCount = 0;
	m_bRecievedStartupMsg = false;

	if ( enet_initialize() != 0 )
    {
		XL_Console::PrintF("^1Error: Error initializing the NetworkMgr.");
        return false;
    }
	XL_Console::PrintF("Initializing the Network subsystem.");

	if ( EngineSettings::IsServer() )
	{
		bSucceeded = CreateServer();
	}
	else
	{
		bSucceeded = CreateClient();
	}
	m_pEngine = pEngine;
	m_SeqNum = 0;
	m_bInitSeqNum = true;

	return bSucceeded;
}

void NetworkMgr::Destroy()
{
	XL_Console::PrintF("Destroying the Network subsystem.");
	if ( _pServer )
	{
		enet_host_destroy(_pServer);
		_pServer = NULL;
	}
	if ( _pClient )
	{
		//we need to disconnect.
		ENetEvent event;
		bool bDisconnectSuccess = false;
		if ( _pServerPeer )
		{
    		enet_peer_disconnect(_pServerPeer, 0);

			// Allow up to 3 seconds for the disconnect to succeed and drop any packets received packets.
			while ( enet_host_service(_pClient, &event, 3000) > 0 )
			{
				switch (event.type)
				{
					case ENET_EVENT_TYPE_RECEIVE:
						enet_packet_destroy(event.packet);
						break;

					case ENET_EVENT_TYPE_DISCONNECT:
						bDisconnectSuccess = true;
						break;
				}
			}
		}
		if ( bDisconnectSuccess == false && _pServerPeer )
		{
			//force the disconnection.
			enet_peer_reset(_pServerPeer);
			_pServerPeer = NULL;
		}

		enet_host_destroy(_pClient);
		_pClient = NULL;
	}
	enet_deinitialize();
}

void NetworkMgr::Loop()
{
	if ( _pServer )
		_ServerLoop();

	if ( _pClient )
		_ClientLoop();

	//the loop is called using a fixed time step.
	m_SeqNum++;
}

bool NetworkMgr::HasRecievedStartupMsg()
{
	Loop();
	return m_bRecievedStartupMsg;
}

bool NetworkMgr::CreateServer()
{
	ENetAddress address;
	m_nPlayerCount = 0;

	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	/* enet_address_set_host (& address, "x.x.x.x"); */
	address.host = ENET_HOST_ANY;
	address.port = EngineSettings::GetPort();

	uint32_t uChannelCount = 2;
	uint32_t uIncomingBandwidth = 0;
	uint32_t uOutgoingBandwidth = 0;
	_pServer = enet_host_create(&address, EngineSettings::GetMaxPlayerCount(), uChannelCount, uIncomingBandwidth, uOutgoingBandwidth);

	const char *pszServerIP = EngineSettings::GetServerIP();
	if ( pszServerIP[0] )
	{
		enet_address_set_host(&address, pszServerIP);
	}

	if (_pServer == NULL)
	{
		XL_Console::PrintF("^1Error: An error occurred while trying to create an ENet server host.");
		return false;
	}
	XL_Console::PrintF("Server Created @ %x:%u, waiting for connections...", address.host, address.port);

	return true;
}

bool NetworkMgr::CreateClient()
{
	uint32_t uChannelCount = 1;
	uint32_t uIncomingBandwidth = 57600 / 8;	//56K modem with 56 Kbps downstream bandwidth
	uint32_t uOutgoingBandwidth = 14400 / 8; //56K modem with 14 Kbps upstream bandwidth
    _pClient = enet_host_create(NULL, 1, uChannelCount, uIncomingBandwidth, uOutgoingBandwidth);

    if (_pClient == NULL)
    {
		XL_Console::PrintF("^1Error: An error occurred while trying to create an ENet client host.");
		return false;
    }
	XL_Console::PrintF("Client Created, preparing to connect to server...");

	//Now try to connect to the server...
	ENetAddress address;
    ENetEvent event;

    /* Connect to some.server.net:1234. */
	enet_address_set_host(&address, EngineSettings::GetServerIP());
    address.port = 5030;

    /* Initiate the connection, allocating the two channels 0 and 1. */
    _pServerPeer = enet_host_connect(_pClient, &address, 2, 0);    
    if (_pServerPeer == NULL)
    {
		XL_Console::PrintF("^1Error: No available peers for initiating an ENet connection.");
		return false;
    }
    
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(_pClient, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    {
		XL_Console::PrintF("Connection to %s:%u succeeded", EngineSettings::GetServerIP(), 5030);
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(_pServerPeer);
		XL_Console::PrintF("^1Error: Connection to %s:%u failed", EngineSettings::GetServerIP(), 5030);
		return false;
    }

	//now send the server client info.
	ScratchPad::StartFrame();
	{
		uint32_t uPacketSize = 2 + strlen(m_szLocalPlayer) + sizeof(m_SeqNum);
		uint8_t *packet = (uint8_t *)ScratchPad::AllocMem( uPacketSize );
		packet[0] = PTYPE_CLIENT_DATA;								    //type.
		*((uint32_t *)&packet[1]) = m_SeqNum;
		memcpy(&packet[1+sizeof(m_SeqNum)], m_szLocalPlayer, strlen(m_szLocalPlayer)+1);	//data
		SendPacket_Client(1, packet, uPacketSize);
	}
	ScratchPad::FreeFrame();

	return true;
}

void NetworkMgr::SendChatMessage(const char *pszMsg, uint8_t uMsgType)
{
	if ( pszMsg[0] == 0 )
		return;

	ScratchPad::StartFrame();
	{
		//finally send the left the game message.
		uint32_t uPacketSize = 3 + strlen(pszMsg) + sizeof(m_SeqNum);
		uint8_t *packet = (uint8_t *)ScratchPad::AllocMem( uPacketSize );
		packet[0] = PTYPE_CHAT_MSG;						//type.
		*((uint32_t *)&packet[1]) = m_SeqNum;
		packet[1 + sizeof(m_SeqNum)] = uMsgType;					//chat msg type.
		memcpy(&packet[2+sizeof(m_SeqNum)], pszMsg, strlen(pszMsg)+1);		//data

		if ( _pServer )
		{
			//Send a chat message all the clients that the player has joined...
			for (int32_t c=0; c<m_nPlayerCount; c++)
			{
				SendPacket_Server(1, c, packet, uPacketSize);	
			}
		}
		else
		{
			SendPacket_Client(1, packet, uPacketSize);	
		}
	}
	ScratchPad::FreeFrame();
}

void NetworkMgr::SetLocalPlayerName(const char *pszName)
{
	strcpy(m_szLocalPlayer, pszName);
}

void NetworkMgr::SendPacket_Client(uint32_t uChannel, const uint8_t *data, uint32_t uDataSize)
{
	if ( _pServerPeer == NULL )
		return;

    ENetPacket *packet = enet_packet_create(data, uDataSize, ENET_PACKET_FLAG_RELIABLE);
    
    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by         */
    /* enet_host_broadcast (host, 0, packet);         */
    enet_peer_send(_pServerPeer, uChannel, packet);
    /* One could just use enet_host_service() instead. */
    enet_host_flush(_pClient);
}

void NetworkMgr::SendPacket_Server(uint32_t uChannel, uint32_t uClientID, const uint8_t *data, uint32_t uDataSize)
{
    ENetPacket *packet = enet_packet_create(data, uDataSize, ENET_PACKET_FLAG_RELIABLE);
    
    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by         */
    /* enet_host_broadcast (host, 0, packet);         */
    enet_peer_send(_apPlayers[uClientID].peer, uChannel, packet);
    /* One could just use enet_host_service() instead. */
    enet_host_flush(_pServer);
}

uint32_t m_uMPGame_NextStartPoint = 0;
uint32_t m_uMPGame_NumStartPoints = 8;

void NetworkMgr::_ServerLoop()
{
	ENetEvent event;
    
    //Don't wait for events, just gather all the current events for the frame.
	ScratchPad::StartFrame();
    while ( enet_host_service(_pServer, &event, 0) > 0 )
    {
        switch (event.type)
        {
			case ENET_EVENT_TYPE_CONNECT:
			{
				XL_Console::PrintF("Player connected from %x:%u.", event.peer->address.host, event.peer->address.port);

				/* Store any relevant client information here. */
				_apPlayers[ m_nPlayerCount++ ].peer = event.peer;
				event.peer->data = (void *)(intptr_t)m_nPlayerCount;

				//now that the client has connected, send the startup data.
				const char *pszMap = EngineSettings::GetStartMap();
				uint32_t uPacketSize = 2 + strlen(pszMap) + sizeof(m_SeqNum) + sizeof(Vector3) + sizeof(int32_t);
				uint32_t uPacketIdx = 0;
				uint8_t *packet = (uint8_t *)ScratchPad::AllocMem( uPacketSize );
				packet[0] = PTYPE_SET_MAP;	uPacketIdx++;
				*((uint32_t *)&packet[uPacketIdx]) = m_SeqNum;	uPacketIdx+=sizeof(m_SeqNum);
				memcpy(&packet[uPacketIdx], pszMap, strlen(pszMap)+1);	//data
				uPacketIdx += strlen(pszMap)+1;

				//this stuff should really go into a gamemode class
				//possibly in the game itself.
				//It's going here for now for testing.
				char szStartName[32];
				sprintf(szStartName, "mp0_start%d", m_uMPGame_NextStartPoint);
				Object *start = ObjectManager::FindObject(szStartName);
				Vector3 start_pos(0,0,0);
				int32_t nSector=0;
				if ( start )
				{
					start->GetLoc(start_pos);
					nSector = (int32_t)start->GetSector();
				}
				m_uMPGame_NextStartPoint = (m_uMPGame_NextStartPoint+1)%m_uMPGame_NumStartPoints;
				memcpy(&packet[uPacketIdx], &start_pos, sizeof(Vector3));
				uPacketIdx += sizeof(Vector3);
				*((int32_t *)&packet[uPacketIdx]) = nSector;

				SendPacket_Server(0, m_nPlayerCount-1, packet, uPacketSize);
			}
            break;

			case ENET_EVENT_TYPE_RECEIVE:
				if ( event.packet->dataLength > GetPacketOverhead() )
				{
					int32_t nClientID = -1 + (int32_t)(intptr_t)event.peer->data;
					if ( nClientID > -1 && nClientID < m_nPlayerCount )
					{
						uint32_t dataSize;
						uint8_t *data = UnpackPacket( (uint8_t *)event.packet->data, event.packet->dataLength, dataSize );

						Server_ProcessPacket( nClientID, m_PacketType, data, dataSize, event.channelID );
					}
				}

				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);
			break;
           
			case ENET_EVENT_TYPE_DISCONNECT:
				//now shuffle the client list...
				if ( event.peer->data && m_nPlayerCount > 0 )
				{
					int32_t nClientID = -1 + (int32_t)(intptr_t)event.peer->data;

					char szMsg[64];
					sprintf(szMsg, "Player \"%s\" has left the game.", _apPlayers[nClientID].szName);
					XL_Console::PrintF(szMsg);

					for (int32_t i=nClientID; i<m_nPlayerCount-1; i++)
					{
						_apPlayers[i] = _apPlayers[i+1];
					}
					m_nPlayerCount--;
				
					//finally send the left the game message.
					uint32_t uPacketSize = 3 + strlen(szMsg) + sizeof(m_SeqNum);
					uint8_t *packet = (uint8_t *)ScratchPad::AllocMem( uPacketSize );
					packet[0] = PTYPE_CHAT_MSG;						//type.
					*((uint32_t *)&packet[1]) = m_SeqNum;
					packet[1+sizeof(m_SeqNum)] = CHATMSG_SYS_INFO;					//chat msg type.
					memcpy(&packet[2 + sizeof(m_SeqNum)], szMsg, strlen(szMsg)+1);		//data

					//Send a chat message all the clients that the player has joined...
					for (int32_t c=0; c<m_nPlayerCount; c++)
					{
						SendPacket_Server(1, c, packet, uPacketSize);	
					}
				}
				/* Reset the peer's client information. */
				event.peer->data = NULL;
			break;
        }
    }
	ScratchPad::FreeFrame();
}

void NetworkMgr::_ClientLoop()
{
	ENetEvent event;
    
    //Don't wait for events, just gather all the current events for the frame.
    while ( enet_host_service(_pClient, &event, 0) > 0 )
    {
        switch (event.type)
        {
			case ENET_EVENT_TYPE_CONNECT:
				XL_Console::PrintF("Successful connection to the server at %x:%u.", event.peer->address.host, event.peer->address.port);

				/* Store any relevant client information here. */
				event.peer->data = 0;	//this will be filled in later.
            break;

			case ENET_EVENT_TYPE_RECEIVE:
				if ( event.packet->dataLength > GetPacketOverhead() )
				{
					uint32_t dataSize;
					uint8_t *data = UnpackPacket( (uint8_t *)event.packet->data, event.packet->dataLength, dataSize );

					//if this is the first server packet, we have to sync our sequence numbers.
					if ( m_bInitSeqNum )
					{
						m_SeqNum = m_PeerSeqNum+1;
						m_bInitSeqNum = false;
					}

					Client_ProcessPacket( m_PacketType, data, dataSize, event.channelID );
				}

				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);
			break;
           
			case ENET_EVENT_TYPE_DISCONNECT:
				XL_Console::PrintF("You have been disconnected.");
				Vector4 red(1.0f, 0.0f, 0.0f, 1.0f);
				m_pEngine->AddDisplayMessage( "You have been disconnected.", &red, 1000.0f );
				/* Reset the peer's client information. */
				event.peer->data = NULL;
				_pServerPeer = NULL;
			break;
        }
    }
}

//this might be moved somewhere else later...
void NetworkMgr::Client_ProcessPacket( uint8_t type, const uint8_t *data, uint32_t dataSize, uint32_t channel )
{
	switch (type)
	{
		case PTYPE_SET_MAP:
		{
			const char *pszMapName = (const char *)data;
			size_t offset = strlen(pszMapName) + 1;
			const Vector3 *startPos = (const Vector3 *)&data[offset]; offset += sizeof(Vector3);
			const int32_t *sector = (const int32_t *)&data[offset];
			XL_Console::PrintF("Setup MP parameters. Map = %s, start = (%2.2f,%2.2f,%2.2f), %d.", pszMapName, startPos->x, startPos->y, startPos->z, *sector);
			EngineSettings::SetStartMap( pszMapName );
			EngineSettings::SetStartPos(startPos, *sector);
			m_bRecievedStartupMsg = true;
		}
		break;
		case PTYPE_CHAT_MSG:
			uint8_t uMsgType = data[0];
			m_pEngine->AddDisplayMessage( (const char *)&data[1], &m_aMsgColorTable[uMsgType], 30.0f );
		break;
	};
}

void NetworkMgr::Server_ProcessPacket( int32_t nClientID, uint8_t type, const uint8_t *data, uint32_t dataSize, uint32_t channel )
{
	ScratchPad::StartFrame();
	switch (type)
	{
		case PTYPE_CLIENT_DATA:
		{
			char szMsg[64];
			sprintf(szMsg, "Player \"%s\" joined the game.", (const char *)data);

			XL_Console::PrintF(szMsg);
			strcpy( _apPlayers[nClientID].szName, (const char *)data );

			//now that the client has connected, send the startup data.
			uint32_t uPacketSize = GetPacketOverhead() + 2 + strlen(szMsg);
			uint8_t *base_packet = (uint8_t *)ScratchPad::AllocMem( uPacketSize );
			uint8_t *packet = SetupPacket(base_packet, PTYPE_CHAT_MSG);
			packet[0] = CHATMSG_SYS_INFO;
			memcpy(&packet[1], szMsg, strlen(szMsg)+1);	//data

			//Send a chat message all the clients that the player has joined...
			for (int32_t c=0; c<m_nPlayerCount; c++)
			{
				SendPacket_Server(1, c, base_packet, uPacketSize);	
			}
		}
		break;
		case PTYPE_CHAT_MSG:	//we just pass this to all the clients...
		{
			uint8_t uMsgType = data[0];
			const char *pszMsg = (const char *)&data[1];
			char szChatMsg[128];
			sprintf(szChatMsg, "%s: %s", _apPlayers[nClientID].szName, pszMsg);
			XL_Console::PrintF(szChatMsg);

			//broadcast chat messages.
			uint32_t uPacketSize = 2 + strlen(szChatMsg) + GetPacketOverhead();
			uint8_t *base_packet = (uint8_t *)ScratchPad::AllocMem( uPacketSize );
			uint8_t *packet = SetupPacket(base_packet, PTYPE_CHAT_MSG);
			packet[0] = uMsgType;					//chat msg type.
			memcpy(&packet[1], szChatMsg, strlen(szChatMsg)+1);	//data

			//Send a chat message all the clients that the player has joined...
			for (int32_t c=0; c<m_nPlayerCount; c++)
			{
				SendPacket_Server(1, c, base_packet, uPacketSize);	
			}
		}
		break;
	};
	ScratchPad::FreeFrame();
}
