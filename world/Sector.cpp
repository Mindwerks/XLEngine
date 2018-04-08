#include "Sector.h"

uint32_t Sector::s_MaxSecDrawCnt = 0;

Sector::Sector() {
    m_uTypeFlags = SECTOR_TYPE_DUNGEON;
    m_Bounds[0].Set(0.0f, 0.0f, 0.0f);
    m_Bounds[1].Set(0.0f, 0.0f, 0.0f);
    m_x = 0;
    m_y = 0;
    m_bActive = true;
    m_pValidNodes = NULL;
}

Sector::~Sector() {
    vector<LightObject *>::iterator iLight = m_Lights.begin();
    vector<LightObject *>::iterator eLight = m_Lights.end();
    for (; iLight != eLight; ++iLight)
    {
        xlDelete *iLight;
    }
    m_Lights.clear();
    xlDelete[] m_pValidNodes;
}

void Sector::AddObject(uint32_t uHandle) {
    m_Objects.push_back(uHandle);
}

void Sector::RemoveObject(uint32_t uHandle) {
    //search for object handle and then erase it.
    vector<uint32_t>::iterator iObj = m_Objects.begin();
    vector<uint32_t>::iterator eObj = m_Objects.end();
    for (; iObj != eObj; ++iObj)
    {
        if (*iObj == uHandle)
        {
            m_Objects.erase(iObj);
            break;
        }
    }
}
