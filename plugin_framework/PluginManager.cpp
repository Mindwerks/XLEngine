#include "PluginManager.h"
#include "DynamicLibrary.h"
#include "plugin.h"
#include "../Engine.h"

DynamicLibrary *PluginManager::m_pGameLib;
XLEngine_Plugin_API *PluginManager::m_pAPI;
XL_ExitFunc m_ExitFunc;

bool PluginManager::Init(XLEngine_Plugin_API *pluginAPI)
{
    m_pGameLib = nullptr;
    m_ExitFunc = nullptr;
    m_pAPI = pluginAPI;

    return true;
}

void PluginManager::Destroy()
{
    //Unload the current game.
    UnloadGame();
}

bool PluginManager::InitGame(const std::string& path)
{
    //first unload the game, since we can only have one at a time right now.
    UnloadGame();

    //load the game lib.
    std::string errorString;
    m_pGameLib = DynamicLibrary::Load(path, errorString);
    if (!m_pGameLib) // not a dynamic library? 
      return false;
                    
    // Get the XL_initPlugin() function
    XL_InitFunc initFunc = (XL_InitFunc)(m_pGameLib->GetSymbol("XL_initPlugin"));
    if (!initFunc) // dynamic library missing entry point?
      return false;

    m_ExitFunc = initFunc( m_pAPI );
    if ( !m_ExitFunc )
        return false;

    return true;
}

void PluginManager::UnloadGame()
{
    if ( m_ExitFunc )
    {
        m_ExitFunc();
        m_ExitFunc = nullptr;
    }
    if ( m_pGameLib )
    {
        xlDelete m_pGameLib;
        m_pGameLib = nullptr;
    }
}
