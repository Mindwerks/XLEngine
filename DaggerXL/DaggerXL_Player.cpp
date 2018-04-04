#include "DaggerXL_Player.h"
#include "Logic_NPC.h"
#include "../math/Math.h"
#include "../math/Vector3.h"

#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/CellTypes.h"

LOGIC_CB_MAP(DaggerXL_Player);
const f32 _VertClamp       = 0.90f;
const f32 m_PlayerHeight   = 16.8f;
const f32 m_PlayerCrouch   = 1.800618409f;
const f32 m_PlayerSwimming = 1.800618409f;
#define CAMERA_ROT_INC 0.0021333333f

//#define DAGGERFALL_START_WORLDX  876
//#define DAGGERFALL_START_WORLDY 2731

#define DAGGERFALL_START_WORLDX 1656
#define DAGGERFALL_START_WORLDY 2288

Vector3 m_StartGravity(0,0,-0.0166666667f);
Vector3 m_GravVector(0,0,0);
Vector3 m_GravityAccel(0,0,-0.0166666667f*10);
Vector3 m_PrevVel(0,0,0);

f32 m_fAccelInc = 0.01f;
f32 m_fAccelDec = 0.25f;

DaggerXL_Player::DaggerXL_Player(const XLEngine_Plugin_API *API)
{
	m_pAPI = API;
	m_bOnGround = true;

	//1. Create the player logic.
	LOGIC_FUNC_LIST(funcs);
	m_pAPI->Logic_CreateFromCode("LOGIC_PLAYER", this, funcs);

	//2. Create the player object.
	m_uObjID  = m_pAPI->Object_Create("PLAYER", -1);
	m_pAPI->Object_AddLogic(m_uObjID, "LOGIC_PLAYER");
	m_pAPI->Object_SetGameData(m_uObjID, &m_PlayerData);

	//3. Reserve enough permanent space for the player,
	//it should never be deleted until exit.
	m_pAPI->Object_ReserveObjects(1);

	//4. Hold onto the physics data so we don't have to get it all the time.
	m_PhysicsData = m_pAPI->Object_GetPhysicsData(m_uObjID);
	m_PhysicsData->m_worldX = DAGGERFALL_START_WORLDX;
	m_PhysicsData->m_worldY = DAGGERFALL_START_WORLDY;

	//-------------------------
	//float fx, fy, fz;
	//m_pAPI->World_LoadCell( CELLTYPE_DAGGERFALL, ARCHIVETYPE_BSA, "", "The Ashford Graveyard", 0, 0 );
	//m_pAPI->World_GetCellWorldPos("The Ashford Graveyard", &m_PhysicsData->m_worldX, &m_PhysicsData->m_worldY);
	//m_pAPI->World_GetCellStartPos("The Ashford Graveyard", &fx, &fy, &fz);
	//SetPos( Vector3(fx, fy, fz+0.5f) );

	m_PlayerData.m_fYaw   = 0.0f;
	m_PlayerData.m_fPitch = 0.0f;
	m_nFrameRotDelay      = 8;
	m_bAutoMove           = false;
}

DaggerXL_Player::~DaggerXL_Player(void)
{
}

void DaggerXL_Player::LogicSetup(u32 uObjID, u32 uParamCount, LogicParam *param)
{
}

void DaggerXL_Player::ObjectSetup(u32 uObjID, u32 uParamCount, LogicParam *param)
{
}

void DaggerXL_Player::KeyDown(s32 key)
{
	if ( key == XL_SPACE && m_bOnGround && m_GravVector.z == m_StartGravity.z )
	{
		m_bOnGround    = false;
		m_GravVector.z = 1.5f;
	}
	else if ( key == XL_LBUTTON )
	{
		Vector3 p0, p1;
		p0 = m_PhysicsData->m_Loc; p0.z += (m_PlayerHeight);
		p1 = p0 + m_vLookDir*50.0f;

		u32 uSector = 0;//m_PhysicsData->m_uSector;
		m_pAPI->World_Activate(&p0, &p1, uSector);
	}
	else if ( key == XL_T )
	{
		m_bAutoMove = !m_bAutoMove;
	}
}

void DaggerXL_Player::SetPos(Vector3& pos)
{
	m_PhysicsData->m_Loc = pos;
	m_PlayerData.m_fPitch = 0;
	m_PlayerData.m_fYaw = 4.71238898f;
	m_nFrameRotDelay = 8;
}

void DaggerXL_Player::GetPos(Vector3& pos, s32& worldX, s32& worldY)
{
	pos = m_PhysicsData->m_Loc;
	worldX = m_PhysicsData->m_worldX;
	worldY = m_PhysicsData->m_worldY;
}

void DaggerXL_Player::Update(u32 uObjID, u32 uParamCount, LogicParam *param)
{
	XL_BOOL bUpdateControls = m_pAPI->Engine_AllowPlayerControls();
	m_PhysicsData->m_uSector = 0;

	//Handle player control
	//Look
	f32 fDeltaX = m_pAPI->GetMouseDx();
	f32 fDeltaY = m_pAPI->GetMouseDy();

	if ( bUpdateControls && m_nFrameRotDelay > 0 )
	{
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
	}
	else if ( m_nFrameRotDelay > 0 )
	{
		m_nFrameRotDelay--;
	}
	static f32 fOldMoveSpd=0.0f;
	static Vector3 vLastDir(0.0f, 0.0f, 0.0f);
	static int nLevitateTime = 0;

	Vector3 vDir;
	vDir.x = cosf(m_PlayerData.m_fYaw) * cosf(m_PlayerData.m_fPitch);
	vDir.y = sinf(m_PlayerData.m_fYaw) * cosf(m_PlayerData.m_fPitch);
	vDir.z = sinf(m_PlayerData.m_fPitch);
	vDir.Normalize();

	m_vLookDir = vDir;

	m_PhysicsData->m_Dir.Set( vDir.x, vDir.y, 0.0f );
	m_PhysicsData->m_Dir.Normalize();

	Vector3 vRight, vUp(0.0f, 0.0f, 1.0f);
	vRight.CrossAndNormalize(vUp, m_PhysicsData->m_Dir);

	//Movement
	Vector3 vLoc = m_PhysicsData->m_Loc;
	Vector3 vel(0.0f, 0.0f, 0.0f);
	f32 fSpeed = 1.0f;//0.5333333f;
	bool bCrouch = false;
	static bool bLevitating = false;
	static f32 fCameraHeightPrev = m_PlayerHeight;
	f32 fCameraHeight = m_PlayerHeight;
	if ( bUpdateControls )
	{
		if ( m_pAPI->IsKeyDown( XL_SHIFT ) || m_pAPI->IsKeyDown( XL_R ) || m_bAutoMove )
		{
			fSpeed *= 20.0f;
		}

		if ( m_pAPI->World_IsPointInWater(&vLoc) && m_bOnGround && !bLevitating )
		{
			fSpeed *= 0.25f;
		}
		else if ( m_pAPI->IsKeyDown( XL_C ) )
		{
			bCrouch = true;
			fCameraHeight = m_PlayerCrouch*0.25f + fCameraHeightPrev*0.75f;
			fSpeed *= 0.5f * fCameraHeight/m_PlayerCrouch;
		}
		else
		{
			fCameraHeight = m_PlayerHeight*0.25f + fCameraHeightPrev*0.75f;
		}

		if ( m_pAPI->IsKeyDown( XL_Q ) )
		{
			bLevitating = true;
			nLevitateTime = 600;
		}

		if ( m_bOnGround || bLevitating )
		{
			if ( m_pAPI->IsKeyDown( XL_W ) || m_bAutoMove )
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

			m_PrevVel = vel;
		}
		else
		{
			vel = m_PrevVel;
		}

		if ( bLevitating )
		{
			if ( m_pAPI->IsKeyDown( XL_PRIOR ) )
			{
				vel.z = fSpeed*120.0f;
			}
			else if ( m_pAPI->IsKeyDown( XL_NEXT ) )
			{
				vel.z = -fSpeed*20.0f;
			}
		}
	}

	fCameraHeightPrev = fCameraHeight;

	//now we want to find our final move speed.
	f32 fDesSpeed = vel.Normalize();
	f32 fMoveSpeed = fDesSpeed;
	if ( 0 )
	{
		float s  = (fDesSpeed-fOldMoveSpd) > 0.0f ? 1.0f : -1.0f;
		float ds = fabsf(fDesSpeed-fOldMoveSpd);
		if ( ds < 0.1f ) { ds = 0.1f; }
		fMoveSpeed = fOldMoveSpd + ((s>0.0f) ? (m_fAccelInc*s) : (m_fAccelDec*s*ds*ds));
		if ( ( fMoveSpeed > fDesSpeed && s > 0.0f ) || ( fMoveSpeed < fDesSpeed && s < 0.0f ) ) { fMoveSpeed = fDesSpeed; }
	}

	fOldMoveSpd = fMoveSpeed;
	vel = vel*fMoveSpeed;

	f32 fDelta = 0.0f;
	f32 fMoveSpd = vel.Normalize();
	f32 fPlayerRadius = 1.01f;	//was 1.51
	//split collision into 1 unit segments.
	f32 fPrevZ = vLoc.z;

	vLoc.z += (fCameraHeight+1.0f)*0.5f;
	if (vel.z == 0 && !bLevitating)
	{
		if ( m_bOnGround )
		{
			float fStartZ = vLoc.z;
			Vector3 vGravLoc = vLoc + Vector3(0,0,-4.0f);
			m_pAPI->World_Collide(&vLoc, &vGravLoc, m_PhysicsData->m_uSector, fPlayerRadius, m_bPassthruAdjoins?1:0);
			if ( fabsf(vGravLoc.z - fStartZ) < 4.0f )
			{
				m_bOnGround = true;
				vLoc = vGravLoc;
				m_GravVector = m_StartGravity;
			}
			else
			{
				m_bOnGround = false;
			}
		}
		else
		{
			float fStartZ = vLoc.z;
			Vector3 vGravLoc = vLoc + m_GravVector;
			m_pAPI->World_Collide(&vLoc, &vGravLoc, m_PhysicsData->m_uSector, fPlayerRadius, m_bPassthruAdjoins?1:0);
			if ( fabsf(vGravLoc.z - fStartZ) < fabsf(m_GravityAccel.z)*0.5f && vGravLoc.z <= fStartZ )
			{
   				m_bOnGround = true;
				m_GravVector = m_StartGravity;
			}
			vLoc = vGravLoc;

			m_GravVector = m_GravVector + m_GravityAccel;
		}
	}
	//do a single raycast to make sure the player hasn't sunk into the ground...
	Vector3 vStart, vEnd, vInter;
	vStart = vLoc + Vector3(0.0f, 0.0f, 0.0f);
	vEnd = vStart - Vector3(0.0f, 0.0f, 8.9f);
	if ( m_pAPI->World_Raycast(&vStart, &vEnd, &vInter) )
	{
		if ( vInter.z - vEnd.z > 0.0f )
		{
			vLoc.z += (vInter.z - vEnd.z);
			m_bOnGround = true;
		}
	}

	if ( fMoveSpd != 0.0f )
	{
		Vector3 vNewLoc = vLoc + vel*fMoveSpd;
		m_pAPI->World_Collide(&vLoc, &vNewLoc, m_PhysicsData->m_uSector, fPlayerRadius, m_bPassthruAdjoins?1:0);
		vLoc = vNewLoc;
	}

	vLoc.z -= (fCameraHeight+1.0f)*0.5f;

	m_PhysicsData->m_Loc = vLoc;

	if ( m_pAPI->World_IsPointInWater(&vLoc) && m_bOnGround )
	{
		vLoc.z += m_PlayerSwimming;
	}
	else
	{
		vLoc.z += fCameraHeight;
	}

	//now do the up/down walking/running movement...
	if ( !bLevitating )
	{
		static f32 time_sum  = 0.0f;
		static f32 time_sum2 = 0.0f;
		static f32 fAnimSpd = 0.0f;
		const f32 K  = 6.0f;//16.0f;
		const f32 K2 = 4.0f;
		const f32 A  = 0.33f;//0.25f;
		const f32 dt = 1.0f/60.0f;
		if ( 1 )//!m_bJumping && !m_bFalling )
		{
			time_sum  = fmodf(time_sum +dt*K,  MATH_TWO_PI);
			time_sum2 = fmodf(time_sum2+dt*K2, MATH_TWO_PI);
		}
		f32 zAnim = sinf( time_sum ) * A * fMoveSpeed;// * m_PlayerHeight/fCameraHeight;
		vLoc.z += zAnim;
	}

	/*
	if ( fAnimSpd < fMoveSpd )
		fAnimSpd += MIN( 0.2f, fMoveSpd-fAnimSpd );
	else if ( fAnimSpd > fMoveSpd )
		fAnimSpd -= MIN( 0.2f, fAnimSpd - fMoveSpd );

	if ( fAnimSpd < 0.0f ) { fAnimSpd = 0.0f; }
	else if ( fAnimSpd > fSpeed ) { fAnimSpd = fSpeed; }
	*/
	
	//now update the camera....
	m_pAPI->Engine_SetCameraData( &vLoc.x, &vDir.x, 0.0f, fMoveSpeed, m_PhysicsData->m_uSector );

	if ( bLevitating )
	{
		nLevitateTime--;
		if ( nLevitateTime <= 0 )
		{
			nLevitateTime = 0;
			bLevitating = false;
		}
	}
}

void DaggerXL_Player::Message(u32 uObjID, u32 uParamCount, LogicParam *param)
{
}
