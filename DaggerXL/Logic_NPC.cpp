#include "Logic_NPC.h"
#include "../math/Math.h"
#include "../math/Vector3.h"

enum NPCStates_e
{
	NPC_STATE_IDLE=0,
	NPC_STATE_WALK,
	NPC_STATE_DISABLED
};

NPC::NPC(const XLEngine_Plugin_API *pAPI)
{
	//Create the NPC object.
	m_Data.uObjID = pAPI->Object_Create("NPC", 0xffff);
	pAPI->Object_SetRenderComponent(m_Data.uObjID, "SPRITE_ZAXIS");

	pAPI->Object_AddLogic(m_Data.uObjID, "LOGIC_NPC");
	pAPI->Object_SetGameData(m_Data.uObjID, &m_Data);

	Enable(pAPI, false);
}

void NPC::Reset(const XLEngine_Plugin_API *pAPI, int32_t NPC_file, float x, float y, float z, int32_t worldX, int32_t worldY, float dirx, float diry)
{
	char szTexName[64];
	sprintf(szTexName, "TEXTURE.%03d", NPC_file);
	m_Data.ahTex[0] = pAPI->Texture_LoadTexList(2, 7, 0xff, "", szTexName, 0, false);
	pAPI->Object_SetRenderTexture(m_Data.uObjID, m_Data.ahTex[0]);

	int32_t ox, oy;
	uint32_t w, h;
	float fw, fh;
	pAPI->Texture_GetSize(ox, oy, w, h, fw, fh);

	ObjectPhysicsData *physics = pAPI->Object_GetPhysicsData(m_Data.uObjID);

	//sprite scale...
	int16_t *pSpriteScale = (int16_t *)pAPI->Texture_GetExtraData();
	int32_t newWidth  = w*(256+pSpriteScale[0])>>8;
	int32_t newHeight = h*(256+pSpriteScale[1])>>8;

	Vector3 vScale;
	vScale.x = (float)newWidth  / 8.0f;
	vScale.y = vScale.x;
	vScale.z = (float)newHeight / 8.0f;
	physics->m_Scale = vScale;
	physics->m_Loc.Set(x, y, z + vScale.z);
	physics->m_worldX = worldX;
	physics->m_worldY = worldY;
	physics->m_Dir.Set(dirx,diry,0);
	physics->m_Dir.Normalize();
	physics->m_Up.Set(0,0,1);
	physics->m_uSector = 0;
	physics->m_Velocity.Set(0,0,0);

	Vector3 vMin = physics->m_Loc - vScale;
	Vector3 vMax = physics->m_Loc + vScale;
	pAPI->Object_SetWorldBounds( m_Data.uObjID, vMin.x, vMin.y, vMin.z, vMax.x, vMax.y, vMax.z );
				
	//Load the rest of the textures.
	for (int32_t t=1; t<6; t++)
	{
		m_Data.ahTex[t] = pAPI->Texture_LoadTexList(2, 7, 0xff, "", szTexName, t, false);
	}

	Enable(pAPI, false);
}

void NPC::Enable(const XLEngine_Plugin_API *pAPI, bool bEnable)
{
	if ( bEnable )
	{
		m_Data.uState = NPC_STATE_WALK;
		pAPI->Object_SetActive( m_Data.uObjID, true );
	}
	else
	{
		m_Data.uState = NPC_STATE_DISABLED;
		pAPI->Object_SetActive( m_Data.uObjID, false );
	}
}

void NPC::GetWorldPos(const XLEngine_Plugin_API *pAPI, int32_t& x, int32_t& y)
{
	ObjectPhysicsData *pPhysics = pAPI->Object_GetPhysicsData(m_Data.uObjID);
	if ( pPhysics )
	{
		x = pPhysics->m_worldX;
		y = pPhysics->m_worldY;
	}
}

bool NPC::IsEnabled()
{
	return m_Data.uState!=NPC_STATE_DISABLED;
}

NPC::~NPC(void)
{
}

LOGIC_CB_MAP(Logic_NPC);

Logic_NPC::Logic_NPC(const XLEngine_Plugin_API *API)
{
	m_pAPI = API;
	
	//Create the NPC logic.
	LOGIC_FUNC_LIST(funcs);
	m_pAPI->Logic_CreateFromCode("LOGIC_NPC", this, funcs);
}

Logic_NPC::~Logic_NPC(void)
{
}

void Logic_NPC::LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
	m_pAPI->Logic_SetMessageMask(LMSG_ACTIVATE);
}

void Logic_NPC::ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
}

void Logic_NPC::Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
	NPC::GameData *pData = (NPC::GameData *)m_pAPI->Object_GetGameData(uObjID);

	//do nothing if the NPC is disabled.
	if ( pData->uState == NPC_STATE_DISABLED )
		return;

	ObjectPhysicsData *pPhysics = m_pAPI->Object_GetPhysicsData(uObjID);

	float x, y, z;
	m_pAPI->Object_GetCameraVector(uObjID, x, y, z);

	if ( Math::abs(x) < 40.0f && Math::abs(y) < 40.0f )
	{
		pData->uState = NPC_STATE_IDLE;

		m_pAPI->Object_SetRenderFlip(uObjID, false, false);
		m_pAPI->Object_SetRenderTexture(uObjID, pData->ahTex[5]);
	}
	else
	{
		pData->uState = NPC_STATE_WALK;

		Vector3 vDir(x,y,0);
		vDir.Normalize();

		XL_BOOL bFlipX = false;
		float angle = -pPhysics->m_Dir.Dot(vDir);
		int32_t image = (int32_t)( (angle*0.5f+0.5f) * 5.0f );
		if ( image > 4 ) image = 4;

		Vector3 vPerp;
		vPerp.Cross(pPhysics->m_Dir, vDir);
		if ( vPerp.z < 0.0f && image > 0 && image < 4 )
			 bFlipX = true;
		
		m_pAPI->Object_SetRenderFlip(uObjID, bFlipX, false);
		m_pAPI->Object_SetRenderTexture(uObjID, pData->ahTex[image]);
	}
}

void Logic_NPC::Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
	//If this is an ACTIVATE message start up the door animation
	//if the door is not already animating.
	if ( uParamCount && param[0].nParam == LMSG_ACTIVATE )
	{
		//Open the NPC conversation dialog.
	}
}
