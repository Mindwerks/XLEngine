#include "Sector.h"

u32 Sector::s_MaxSecDrawCnt=0;

Sector::Sector()
{
	m_uTypeFlags = SECTOR_TYPE_DUNGEON;
	m_Bounds[0].Set(0.0f, 0.0f, 0.0f);
	m_Bounds[1].Set(0.0f, 0.0f, 0.0f);
	m_x = 0;
	m_y = 0;
	m_bActive = true;
	m_pValidNodes = NULL;
}

Sector::~Sector()
{
	vector<LightObject *>::iterator iLight = m_Lights.begin();
	vector<LightObject *>::iterator eLight = m_Lights.end();
	for (; iLight != eLight; ++iLight)
	{
		xlDelete *iLight;
	}
	m_Lights.clear();
	xlDelete [] m_pValidNodes;
}

void Sector::AddObject(u32 uHandle)
{
	m_Objects.push_back( uHandle );
}

void Sector::RemoveObject(u32 uHandle)
{
	//search for object handle and then erase it.
	vector<u32>::iterator iObj = m_Objects.begin();
	vector<u32>::iterator eObj = m_Objects.end();
	for ( ; iObj != eObj; ++iObj )
	{
		if ( *iObj == uHandle )
		{
			m_Objects.erase( iObj );
			break;
		}
	}
}
