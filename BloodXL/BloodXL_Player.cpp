#include "BloodXL_Player.h"
#include "../math/Math.h"
#include "../math/Vector3.h"

LOGIC_CB_MAP(BloodXL_Player);
const float _VertClamp = 0.785398f;
const float m_PlayerHeight = 4.697265625f;
const float m_PlayerCrouch = 1.800618409f;
#define CAMERA_ROT_INC 0.0021333333f

float m_fAccelInc = 0.01f;
float m_fAccelDec = 0.25f;

BloodXL_Player::BloodXL_Player(const XLEngine_Plugin_API *API)
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

BloodXL_Player::~BloodXL_Player()
{
}

void BloodXL_Player::LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
}

void BloodXL_Player::ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
}

void BloodXL_Player::KeyDown(int32_t key)
{
    if ( key == XL_SPACE )
    {
        Vector3 p0, p1;
        p0 = m_PhysicsData->m_Loc; p0.z += m_PlayerHeight;
        p1 = p0 + m_PhysicsData->m_Dir*10.0f;

        uint32_t uSector = m_PhysicsData->m_uSector;
        m_pAPI->World_Activate(&p0, &p1, uSector);
    }
}

void BloodXL_Player::Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
    XL_BOOL bUpdateControls = m_pAPI->Engine_AllowPlayerControls();

    //Handle player control
    //Look
    float fDeltaX = m_pAPI->GetMouseDx();
    float fDeltaY = m_pAPI->GetMouseDy();

    if ( bUpdateControls )
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
    static float fOldMoveSpd=0.0f;
    static Vector3 vLastDir(0.0f, 0.0f, 0.0f);

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
    float fSpeed = 0.45699f;
    bool bCrouch = false;
    static float fCameraHeightPrev = m_PlayerHeight;
    float fCameraHeight = m_PlayerHeight;
    if ( bUpdateControls )
    {
        if ( m_pAPI->IsKeyDown( XL_C ) )
        {
            bCrouch = true;
            fCameraHeight = m_PlayerCrouch*0.25f + fCameraHeightPrev*0.75f;
            fSpeed *= 0.5f * fCameraHeight/m_PlayerCrouch;
        }
        else
        {
            fCameraHeight = m_PlayerHeight*0.25f + fCameraHeightPrev*0.75f;
        }

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
    }
    fCameraHeightPrev = fCameraHeight;

    //now we want to find our final move speed.
    float fDesSpeed = vel.Normalize();
    float fMoveSpeed = fDesSpeed;
    if ( 1 )
    {
        float s  = (fDesSpeed-fOldMoveSpd) > 0.0f ? 1.0f : -1.0f;
        float ds = fabsf(fDesSpeed-fOldMoveSpd);
        if ( ds < 0.1f ) { ds = 0.1f; }
        fMoveSpeed = fOldMoveSpd + ((s>0.0f) ? (m_fAccelInc*s) : (m_fAccelDec*s*ds*ds));
        if ( ( fMoveSpeed > fDesSpeed && s > 0.0f ) || ( fMoveSpeed < fDesSpeed && s < 0.0f ) ) { fMoveSpeed = fDesSpeed; }
    }

    if ( fDesSpeed == 0.0f && fMoveSpeed != 0.0f )
    {
        vel = vLastDir*fMoveSpeed;
    }
    else
    {
        vLastDir = vel;
    }

    fOldMoveSpd = fMoveSpeed;
    vel = vel*fMoveSpeed;

    float fDelta = 0.0f;
    float fMoveSpd = vel.Normalize();
    float fPlayerRadius = 1.01f;    //was 1.51
    //split collision into 1 unit segments.
    float fPrevZ = vLoc.z;
    if ( fMoveSpd != 0.0f )
    {
        do
        {
            fDelta = (fMoveSpd<1.0f) ? (fMoveSpd) : (1.0f);
            Vector3 vNewLoc = vLoc + vel*fDelta;
            m_pAPI->World_Collide(&vLoc, &vNewLoc, m_PhysicsData->m_uSector, fPlayerRadius, m_bPassthruAdjoins?1:0);

            fMoveSpd -= fDelta;
        } while (fMoveSpd > 0.0f);
    }
    //if ( vLoc.z < fPrevZ )
    {
        vLoc.z = vLoc.z*0.25f + fPrevZ*0.75f;
    }

    m_PhysicsData->m_Loc = vLoc;
    vLoc.z += fCameraHeight;

    //now do the up/down walking/running movement...
    static float time_sum  = 0.0f;
    static float time_sum2 = 0.0f;
    static float fAnimSpd = 0.0f;
    const float K  = 16.0f;
    const float K2 = 4.0f;
    const float A  = 0.25f;
    const float dt = 1.0f/60.0f;
    if ( 1 )//!m_bJumping && !m_bFalling )
    {
        time_sum  = fmodf(time_sum +dt*K,  MATH_TWO_PI);
        time_sum2 = fmodf(time_sum2+dt*K2, MATH_TWO_PI);
    }
    float zAnim = sinf( time_sum ) * A * fMoveSpeed * m_PlayerHeight/fCameraHeight;
    vLoc.z += zAnim;

    /*
    if ( fAnimSpd < fMoveSpd )
        fAnimSpd += MIN( 0.2f, fMoveSpd-fAnimSpd );
    else if ( fAnimSpd > fMoveSpd )
        fAnimSpd -= MIN( 0.2f, fAnimSpd - fMoveSpd );

    if ( fAnimSpd < 0.0f ) { fAnimSpd = 0.0f; }
    else if ( fAnimSpd > fSpeed ) { fAnimSpd = fSpeed; }
    */
    
    //now update the camera....
    m_pAPI->Engine_SetCameraData( &vLoc.x, &vDir.x, m_PlayerData.m_fPitch, fMoveSpeed, m_PhysicsData->m_uSector );
}

void BloodXL_Player::Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
}
