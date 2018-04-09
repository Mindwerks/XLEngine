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
    std::string dataPath;

    uint16_t flags;
    short resX;
    short resY;
    int renderer;

    Common_Settings() : flags(0), resX(0), resY(0), renderer(EngineSettings::RENDERER_SOFT8)
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
            settings.dataPath = std::move(value);
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

EngineSettings EngineSettings::sSettings;

// Initialize default settings.
EngineSettings::EngineSettings()
{
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

        mGameDataDir = std::move(commonSettings.dataPath);
        if(!mGameDataDir.empty() && mGameDataDir.back() != '\\' && mGameDataDir.back() != '/')
            mGameDataDir += '/';

        if(commonSettings.resX > 0 && commonSettings.resY > 0)
        {
            mScreenWidth  = commonSettings.resX;
            mScreenHeight = commonSettings.resY;
        }

        mFlags = commonSettings.flags;
        mRenderer = commonSettings.renderer;

        return true;
    }

    return false;
}

bool EngineSettings::IsFeatureEnabled(uint32_t feature)
{
    return (mFlags&feature) ? true : false;
}

void EngineSettings::SetDisplaySettings(float brightness/*=1.0f*/, float contrast/*=1.0f*/, float gamma/*=1.0f*/)
{
    mBrightness = brightness;
    mContrast   = contrast;
    mGamma      = 1.0f/gamma;
}

void EngineSettings::GetDisplaySettings(float& brightness, float& contrast, float& gamma)
{
    brightness = mBrightness;
    contrast   = mContrast;
    gamma      = mGamma;
}

void EngineSettings::SetGameDir(const char *game)
{
    char curDir[260];
    GetCurrentDir(curDir, 260);
    mGameDir = curDir;
    mGameDir += '/';
    mGameDir += game;
}

void EngineSettings::SetStartMap(const char *map_name)
{ 
    mMapName = map_name;
}

void EngineSettings::SetStartPos(const Vector3 *pos, int32_t sector)
{
    mOverridePos = true;
    mStartPos = *pos;
    mStartSec = sector;
}

void EngineSettings::SetMultiplayerData(int32_t server_PlayerCnt, int32_t port, const char *joinIP)
{
    mServerPlayerCnt = server_PlayerCnt;
    mPort = port;
    mServerIP = joinIP;
}
