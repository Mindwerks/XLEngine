#include "EngineSettings.h"
#include <string.h>
#include <stdio.h>
#include <memory.h>

#if PLATFORM_WIN
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

char EngineSettings::m_szGameDataDir[260];
char EngineSettings::m_szMapName[260];
char EngineSettings::m_szGameDir[260];
s32 EngineSettings::m_nScreenWidth;
s32 EngineSettings::m_nScreenHeight;
s32 EngineSettings::m_nRenderer = EngineSettings::RENDERER_SOFT8;//OPENGL;

s32 EngineSettings::m_nServerPlayerCnt;
s32 EngineSettings::m_nPort;
char EngineSettings::m_szServerIP[32];

bool EngineSettings::m_bOverridePos;
s32 EngineSettings::m_nStartSec;
Vector3 EngineSettings::m_vStartPos;

u32 EngineSettings::m_uFlags;

float EngineSettings::m_fBrightness;
float EngineSettings::m_fContrast;
float EngineSettings::m_fGamma;

struct Common_Settings
{
	int version;
	char szDataPath[256];

	char texFilter;
	char lightRange;
	u16 flags;
	short resX;
	short resY;
	char refresh;
};

//set default settings.
void EngineSettings::Init()
{
	memset(m_szGameDataDir, 0, 260);
	//
	m_nScreenWidth  = 1024;
	m_nScreenHeight =  768;
	//Multiplayer defaults.
	m_nServerPlayerCnt = 0;
	m_nPort = 0;
	memset(m_szServerIP, 0, 32);
	memset(m_szMapName, 0, 260);

	m_bOverridePos = false;
	m_nStartSec = -1;
	m_vStartPos.Set(0,0,0);

	m_uFlags = 0;

	SetDisplaySettings(1.0f, 1.0f, 1.0f);
}

bool EngineSettings::Load( const char *pszSettingsFile )
{
	//Load the settings file. For now we just read the common data, 
	//later we'll handle extra data per-game.
	FILE *f = fopen(pszSettingsFile, "rb");
	if ( f )
	{
		Common_Settings commonSettings;
		fread(&commonSettings, sizeof(Common_Settings), 1, f);
		fclose(f);

		strcpy(m_szGameDataDir, commonSettings.szDataPath);
		size_t len = strlen(m_szGameDataDir);
		if ( len > 0 && m_szGameDataDir[len-1] != '\\' && m_szGameDataDir[len-1] != '/' )
		{
			m_szGameDataDir[len] = '/';
			m_szGameDataDir[len+1] = 0;
		}

		m_nScreenWidth  = commonSettings.resX;
		m_nScreenHeight = commonSettings.resY;

		m_uFlags = commonSettings.flags;
		
		return true;
	}

	return false;
}

bool EngineSettings::IsFeatureEnabled(u32 uFeature)
{
	return (m_uFlags&uFeature) ? true : false;
}

void EngineSettings::SetDisplaySettings(float brightness/*=1.0f*/, float contrast/*=1.0f*/, float gamma/*=1.0f*/)
{
	m_fBrightness = brightness;
	m_fContrast   = contrast;
	m_fGamma      = 1.0f/gamma;
}

void EngineSettings::GetDisplaySettings(float& brightness, float& contrast, float& gamma)
{
	brightness = m_fBrightness;
	contrast   = m_fContrast;
	gamma	   = m_fGamma;
}

void EngineSettings::SetGameDir(const char *pszGame)
{
	char szCurDir[260];
	GetCurrentDir(szCurDir, 260);
	sprintf(m_szGameDir, "%s/%s", szCurDir, pszGame);
}

void EngineSettings::SetStartMap( const char *pszMapName ) 
{ 
	strcpy(m_szMapName, pszMapName); 
}

void EngineSettings::SetStartPos( const Vector3 *pos, s32 nSector )
{
	m_bOverridePos = true;
	m_vStartPos = *pos;
	m_nStartSec = nSector;
}

void EngineSettings::SetMultiplayerData( s32 nServer_PlayerCnt, s32 nPort, const char *pszJoinIP )
{
	m_nServerPlayerCnt = nServer_PlayerCnt;
	m_nPort = nPort;
	strcpy(m_szServerIP, pszJoinIP);
}