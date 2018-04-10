#include "Logic_Obj_Action.h"
#include "../math/Math.h"
#include "../math/Vector3.h"

LOGIC_CB_MAP(Logic_Obj_Action);

struct LOA_GameData
{
    int nParentID;  //-1 = no parent.
    int nTargetID;  //-1 = no target.
    int nType;
    int nAxis;
    float Duration;
    float Delta;

    float origPos[3];
    float origAng[3];

    //animation
    float curDelta; //which direction to move in and how much per step.
    float fAnim;    //current animation state.
    int   bAnim;    //is currently animating?
};

Logic_Obj_Action::Logic_Obj_Action(const XLEngine_Plugin_API *API)
{
    m_pAPI = API;

    //1. Setup the logic functions.
    LOGIC_FUNC_LIST(funcs);
    m_pAPI->Logic_CreateFromCode("LOGIC_OBJ_ACTION", this, funcs);
}

Logic_Obj_Action::~Logic_Obj_Action()
{
}

void Logic_Obj_Action::LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
    m_pAPI->Logic_SetMessageMask(LMSG_ACTIVATE | LMSG_CHILD_ACTIVATE);  //only actions without parents can be directly activated.
}

void Logic_Obj_Action::ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
}

void Logic_Obj_Action::Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
    LOA_GameData *pData = (LOA_GameData *)m_pAPI->Object_GetGameData(uObjID);

    if (pData->bAnim)
    {
        Vector3 vPos(pData->origPos[0], pData->origPos[1], pData->origPos[2]);
        Vector3 vAngles(pData->origAng[0], pData->origAng[1], pData->origAng[2]);

        pData->fAnim += pData->curDelta / pData->Duration;
        if ( pData->fAnim >= 1.0f )      { pData->fAnim = 1.0f; pData->bAnim = false; }
        else if ( pData->fAnim <= 0.0f ) { pData->fAnim = 0.0f; pData->bAnim = false; }

        float t = pData->fAnim;
        if ( pData->nType&0x01 )
        {
            //move...
            Vector3 vDir;
            if ( pData->nAxis == 1 )
                vDir.Set(-1, 0, 0);
            else if ( pData->nAxis == 2 )
                vDir.Set( 1, 0, 0);
            else if ( pData->nAxis == 3 )
                vDir.Set( 0, 0, -1);
            else if ( pData->nAxis == 4 )
                vDir.Set( 0, 0,  1);
            else if ( pData->nAxis == 5 )
                vDir.Set(0, 1, 0);
            else if ( pData->nAxis == 6 )
                vDir.Set(0, -1, 0);

            Vector3 vStart(pData->origPos[0], pData->origPos[1], pData->origPos[2]);
            Vector3 vEnd = vStart + vDir*pData->Delta*0.25f;

            vPos = vStart*(1.0f - t) + vEnd*t;
        }
        if ( pData->nType&0x04 )
        {
            //????
            Vector3 vDir;
            vDir.Set( 0, 0,  1);

            Vector3 vStart(pData->origPos[0], pData->origPos[1], pData->origPos[2]);
            Vector3 vEnd = vStart + vDir*20.0f;

            vPos = vStart*(1.0f - t) + vEnd*t;
        }
        if ( pData->nType&0x08 )
        {
            //hackity hack... hack...
            int axis = pData->nAxis;
            float angY = pData->origAng[1];
            if ( angY < 0 ) angY += 2048;
            if ( pData->nAxis == 1 && angY > 1024 )
                axis = 5;

            Vector3 vDir;
            if ( axis == 1 )
                vDir.Set( 0,  1, 0 );
            else if ( axis == 2 )
                vDir.Set( 0, -1, 0 );
            else if ( axis == 3 )
                vDir.Set( 0, 0, -1 );
            else if ( axis == 4 )
                vDir.Set( 0, 0, 1 );
            else if ( axis == 5 )
                vDir.Set(  1, 0, 0 );
            else if ( axis == 6 )
                vDir.Set( -1, 0, 0 );

            Vector3 vStart(pData->origAng[0], pData->origAng[1], pData->origAng[2]);
            Vector3 vEnd = vStart + vDir*pData->Delta;

            vAngles = vStart*(1.0f - t) + vEnd*t;

            const float piOver2 = 1.5707963267948966192313216916398f;
            Vector3 vAnglesFinal = vAngles * piOver2/512.0f;
            m_pAPI->Object_SetAngles(uObjID, vAnglesFinal.x, vAnglesFinal.y, vAnglesFinal.z);
        }
        m_pAPI->Object_SetPos(uObjID, vPos.x, vPos.y, vPos.z);
    }
}

void Logic_Obj_Action::Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
    if ( uParamCount && (param[0].nParam == LMSG_ACTIVATE || param[0].nParam == LMSG_CHILD_ACTIVATE) )
    {
        LOA_GameData *pData = (LOA_GameData *)m_pAPI->Object_GetGameData(uObjID);
        if ( pData->bAnim == false || param[0].nParam == LMSG_CHILD_ACTIVATE )
        {
            //only the a top level object can recieve a direct "ACTIVATE" call.
            if ( pData->nParentID > -1 && param[0].nParam == LMSG_ACTIVATE )
                return;

            //send commands to the target.
            if ( pData->nTargetID > -1 )
            {
                m_pAPI->Object_SendMessage((uint32_t)pData->nTargetID, LMSG_CHILD_ACTIVATE, 0);
            }

            //nothing to animate if there is no delta or type flags.
            if ( pData->Delta && pData->nType > 0 )
            {
                pData->bAnim = true;
                if ( pData->curDelta == 0.0f )
                {
                    pData->curDelta =  1.0f/60.0f;
                }
                else
                {
                    pData->curDelta *= -1.0f;
                }
            }
        }
    }
}
