#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
using namespace std;

class DynamicLibrary;
struct XLEngine_Plugin_API;

class PluginManager
{
public:

    static bool Init(XLEngine_Plugin_API *pluginAPI);
    static void Destroy();

    static bool InitGame(const string& path);
    static void UnloadGame();

private:
    static DynamicLibrary *m_pGameLib;
    static XLEngine_Plugin_API *m_pAPI;
};

#endif //PLUGIN_MANAGER_H
