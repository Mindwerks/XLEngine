#include "EngineSettings.h"
#include <fstream>
#include <cctype>
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

namespace
{

struct Common_Settings
{
    char szDataPath[260];

    uint16_t flags;
    short resX;
    short resY;
    int renderer;

    Common_Settings()
      : szDataPath(""), flags(0), resX(0), resY(0), renderer(EngineSettings::RENDERER_SOFT8)
    { }
};

void LoadSettingsFromFile(Common_Settings &settings, std::istream &file)
{
    char linestr[64];
    while(file.good() && !file.eof())
    {
        // Read a line from the input (until reaching a \n).
        std::string line;
        while(file.peek() != '\n' && file.get(linestr, sizeof(linestr)))
        {
            line += linestr;
            if(file.gcount() < sizeof(linestr)-1)
                break;
        }
        // Ignore the \n
        file.ignore();

        // Continue to the next line if this one was empty
        if(line.empty()) continue;

        std::size_t sep = line.find_first_of('=');
        if(sep == std::string::npos)
        {
            fprintf(stderr, "Config syntax error (missing '='): %s\n", line.c_str());
            break;
        }

        // Split the line into separate 'key' and 'value' pairs, strip whitespace.
        std::size_t begin = 0;
        while(begin < sep && std::isspace(line[begin]))
            ++sep;
        std::string key = line.substr(begin, sep);
        while(!key.empty() && std::isspace(key.back()))
            key.pop_back();

        ++sep;
        while(sep < line.size() && std::isspace(line[sep]))
            ++sep;
        std::string value = line.substr(sep);
        while(!value.empty() && std::isspace(value.back()))
            value.pop_back();

        // Make the key/setting name lower case
        for(size_t i = 0;i < key.size();i++)
            key[i] = std::tolower(key[i]);

        if(key == "data-path")
        {
            value = value.substr(0, 256);
            strcpy(settings.szDataPath, value.c_str());
        }
        else if(key == "width")
        {
            size_t end = 0;
            int v = !value.empty() ? std::stoi(value, &end) : 0;
            if(end == value.size() && v >= 320)
                settings.resX = v;
            else
                fprintf(stderr, "Invalid display width: %s\n", value.c_str());
        }
        else if(key == "height")
        {
            size_t end = 0;
            int v = !value.empty() ? std::stoi(value, &end) : 0;
            if(end == value.size() && v >= 200)
                settings.resY = v;
            else
                fprintf(stderr, "Invalid display height: %s\n", value.c_str());
        }
        else if(key == "fullscreen")
        {
            for(size_t i = 0;i < value.size();i++)
                value[i] = std::tolower(value[i]);

            if(value == "true" || value == "1")
                settings.flags |= EngineSettings::FULLSCREEN;
        }
        else if(key == "vsync")
        {
            for(size_t i = 0;i < value.size();i++)
                value[i] = std::tolower(value[i]);

            if(value == "true" || value == "1")
                settings.flags |= EngineSettings::VSYNC;
        }
        else if(key == "emulate-low-res")
        {
            for(size_t i = 0;i < value.size();i++)
                value[i] = std::tolower(value[i]);

            if(value == "true" || value == "1")
                settings.flags |= EngineSettings::EMULATE_320x200;
        }
        else if(key == "renderer")
        {
            for(size_t i = 0;i < value.size();i++)
                value[i] = std::tolower(value[i]);

            if(value == "opengl")
                settings.renderer = EngineSettings::RENDERER_OPENGL;
            else if(value == "soft32")
                settings.renderer = EngineSettings::RENDERER_SOFT32;
            else if(value == "soft8")
                settings.renderer = EngineSettings::RENDERER_SOFT8;
            else
                fprintf(stderr, "Invalid renderer: %s\n", value.c_str());
        }
    }
}

} // namespace

char EngineSettings::m_szGameDataDir[260];
char EngineSettings::m_szMapName[260];
char EngineSettings::m_szGameDir[260];
int32_t EngineSettings::m_nScreenWidth;
int32_t EngineSettings::m_nScreenHeight;
int32_t EngineSettings::m_nRenderer = EngineSettings::RENDERER_SOFT8;//OPENGL;

int32_t EngineSettings::m_nServerPlayerCnt;
int32_t EngineSettings::m_nPort;
char EngineSettings::m_szServerIP[32];

bool EngineSettings::m_bOverridePos;
int32_t EngineSettings::m_nStartSec;
Vector3 EngineSettings::m_vStartPos;

uint32_t EngineSettings::m_uFlags;

float EngineSettings::m_fBrightness;
float EngineSettings::m_fContrast;
float EngineSettings::m_fGamma;

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
    std::ifstream infile(pszSettingsFile, std::ios_base::binary);
    if ( infile.is_open() )
    {
        Common_Settings commonSettings;
        LoadSettingsFromFile(commonSettings, infile);
        infile.close();

        strcpy(m_szGameDataDir, commonSettings.szDataPath);
        size_t len = strlen(m_szGameDataDir);
        if ( len > 0 && m_szGameDataDir[len-1] != '\\' && m_szGameDataDir[len-1] != '/' )
        {
            m_szGameDataDir[len] = '/';
            m_szGameDataDir[len+1] = 0;
        }

        if(commonSettings.resX > 0 && commonSettings.resY > 0)
        {
            m_nScreenWidth  = commonSettings.resX;
            m_nScreenHeight = commonSettings.resY;
        }

        m_uFlags = commonSettings.flags;
        m_nRenderer = commonSettings.renderer;

        return true;
    }

    return false;
}

bool EngineSettings::IsFeatureEnabled(uint32_t uFeature)
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

void EngineSettings::SetStartPos( const Vector3 *pos, int32_t nSector )
{
	m_bOverridePos = true;
	m_vStartPos = *pos;
	m_nStartSec = nSector;
}

void EngineSettings::SetMultiplayerData( int32_t nServer_PlayerCnt, int32_t nPort, const char *pszJoinIP )
{
	m_nServerPlayerCnt = nServer_PlayerCnt;
	m_nPort = nPort;
	strcpy(m_szServerIP, pszJoinIP);
}
