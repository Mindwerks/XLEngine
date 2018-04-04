#include "OutlawsXL_Player.h"
#include "../math/Math.h"
#include "../math/Vector3.h"

LOGIC_CB_MAP(OutlawsXL_Player);
const f32 _VertClamp = 0.785398f;
const f32 m_PlayerHeight = 5.8f;
#define CAMERA_ROT_INC 0.0021333333f

OutlawsXL_Player::OutlawsXL_Player(const XLEngine_Plugin_API *API)
{
	m_pAPI = API;

	//1. Create the player logic.
	LOGIC_FUNC_LIST(funcs);
	m_pAPI->Logic_CreateFromCode("LOGIC_PLAYER", this, funcs);

	//2. Create the player object.
	m_uObjID = m_pAPI->Object_Create("PLAYER", -1);
	m_pAPI->Object_AddLogic(m_uObjID, "LOGIC_PLAYER");
	m_pAPI->Object_SetGameData(m_uObjID, &m_PlayerData);

	//3. Reserve enough permanent space for the player.
	//It should never be deleted until exit.
	m_pAPI->Object_ReserveObjects(1);

	//4. Hold onto the physics data so we don't have to get it all the time.
	m_PhysicsData = m_pAPI->Object_GetPhysicsData(m_uObjID);

	m_PlayerData.m_fYaw = 0.0f;
	m_PlayerData.m_fPitch = 0.0f;
	m_bPassthruAdjoins = false;
}

OutlawsXL_Player::~OutlawsXL_Player(void)
{
}

void OutlawsXL_Player::LogicSetup(u32 uObjID, u32 uParamCount, LogicParam *param)
{
}

void OutlawsXL_Player::ObjectSetup(u32 uObjID, u32 uParamCount, LogicParam *param)
{
}

void OutlawsXL_Player::Update(u32 uObjID, u32 uParamCount, LogicParam *param)
{
	//Handle player control
	//Look
	f32 fDeltaX = m_pAPI->GetMouseDx();
	f32 fDeltaY = m_pAPI->GetMouseDy();

	if ( fDeltaX )
	{
		m_PlayerData.m_fYaw = fmodf( m_PlayerData.m_fYaw - fDeltaX*CAMERA_ROT_INC, MATH_TWO_PI );
	}
	if ( fDeltaY )
	{
		m_PlayerData.m_fPitch -= fDeltaY*CAMERA_ROT_INC;
		if ( m_PlayerData.m_fPitch > _VertClamp )
		{
			m_PlayerData.m_fPitch = _VertClamp;
		}
		else if ( m_PlayerData.m_fPitch < -_VertClamp )
		{
			m_PlayerData.m_fPitch = -_VertClamp;
		}
	}
	Vector3 vDir;
	vDir.x = cosf(m_PlayerData.m_fYaw);// * cosf(m_PlayerData.m_fPitch);
	vDir.y = sinf(m_PlayerData.m_fYaw);// * cosf(m_PlayerData.m_fPitch);
	vDir.z = 0.0f;//sinf(m_PlayerData.m_fPitch);
	vDir.Normalize();

	m_PhysicsData->m_Dir.Set( vDir.x, vDir.y, 0.0f );
	m_PhysicsData->m_Dir.Normalize();

	Vector3 vRight, vUp(0.0f, 0.0f, 1.0f);
	vRight.CrossAndNormalize(vUp, m_PhysicsData->m_Dir);

	//Movement
	Vector3 vLoc = m_PhysicsData->m_Loc;
	Vector3 vel(0.0f, 0.0f, 0.0f);
	f32 fSpeed = 20.0f/60.0f;
	if ( m_pAPI->IsKeyDown( XL_W ) )
	{
		vel = vel + m_PhysicsData->m_Dir*fSpeed;
	}
	else if ( m_pAPI->IsKeyDown( XL_S ) )
	{
		vel = vel - m_PhysicsData->m_Dir*fSpeed;
	}

	if ( m_pAPI->IsKeyDown( XL_A ) )
	{
		vel = vel + vRight*fSpeed;
	}
	else if ( m_pAPI->IsKeyDown( XL_D ) )
	{
		vel = vel - vRight*fSpeed;
	}

	f32 fDelta = 0.0f;
	f32 fMoveSpd = vel.Normalize();
	//split collision into 1 unit segments.
	if ( fMoveSpd != 0.0f )
	{
		do
		{
			fDelta = (fMoveSpd<1.0f) ? (fMoveSpd) : (1.0f);
			Vector3 vNewLoc = vLoc + vel*fDelta;
			m_pAPI->World_Collide(&vLoc, &vNewLoc, m_PhysicsData->m_uSector, 1.51f, m_bPassthruAdjoins?1:0);

			fMoveSpd -= fDelta;
		} while (fMoveSpd > 0.0f);
	}

	m_PhysicsData->m_Loc = vLoc;
	vLoc.z += m_PlayerHeight;
	
	//now update the camera....
	m_pAPI->Engine_SetCameraData( &vLoc.x, &vDir.x, m_PlayerData.m_fPitch, 1.0f, m_PhysicsData->m_uSector );
}

void OutlawsXL_Player::Message(u32 uObjID, u32 uParamCount, LogicParam *param)
{
}
