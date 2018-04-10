#include "Logic_Door.h"
#include "../math/Math.h"
#include "../math/Vector3.h"

LOGIC_CB_MAP(Logic_Door);

const float s_fAnimDelta = (1.0f/60.0f);

Logic_Door::Logic_Door(const XLEngine_Plugin_API *API)
{
    m_pAPI = API;

    //1. Create the player logic.
    LOGIC_FUNC_LIST(funcs);
    m_pAPI->Logic_CreateFromCode("LOGIC_DOOR", this, funcs);
}

Logic_Door::~Logic_Door()
{
}

void Logic_Door::LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
    m_pAPI->Logic_SetMessageMask(LMSG_ACTIVATE);
}

void Logic_Door::ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
}

void Logic_Door::Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
    float *pData = (float *)m_pAPI->Object_GetGameData(uObjID);
    float animTime = pData[2];
    if ( animTime )
    {
        float yaw;
        if ( animTime >= 1.0f )
        {
            if ( pData[3] == 0 )
            {
                yaw = pData[1];
            }
            else
            {
                yaw = pData[0];
                m_pAPI->Object_EnableCollision(uObjID, true);
            }

            pData[2] = 0.0f;
            pData[3] = (pData[3] == 0.0f) ? 1.0f : 0.0f;
        }
        else
        {
            if ( pData[3] == 0 )
            {
                yaw = (1.0f - animTime)*pData[0] + animTime*pData[1];
            }
            else
            {
                yaw = (1.0f - animTime)*pData[1] + animTime*pData[0];
            }
            pData[2] += s_fAnimDelta;
        }

        m_pAPI->Object_SetAngles(uObjID, 0, 0, yaw);
    }
}

void Logic_Door::Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param)
{
    //If this is an ACTIVATE message start up the door animation
    //if the door is not already animating.
    if ( uParamCount && param[0].nParam == LMSG_ACTIVATE )
    {
        float *pData = (float *)m_pAPI->Object_GetGameData(uObjID);
        if ( pData[2] == 0.0f )
        {
            pData[2] = s_fAnimDelta;
            m_pAPI->Object_EnableCollision(uObjID, false);
        }
    }
}
