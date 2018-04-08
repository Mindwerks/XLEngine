#ifndef XL_PLUGIN_H
#define XL_PLUGIN_H

#include "XLEngine_Plugin_API.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XL_PluginAPI_Version
{
  int32_t major;
  int32_t minor;
} XL_PluginAPI_Version;

typedef int32_t (*XL_ExitFunc)();

/** Type definition of the XL_initPlugin function below (used by PluginManager to initialize plugins)
 * Note the return type is the XL_ExitFunc (used by PluginManager to tell plugins to cleanup). If 
 * the initialization failed for any reason the plugin may report the error via the error reporting
 * function of the provided platform services. Nevertheless, it must return NULL exit func in this case
 * to let the plugin manger that the plugin wasn't initialized properly. The plugin may use the runtime
 * services - allocate memory, log messages and of course register node types.
 *
 * @param  [const XL_PlatformServices *] params - the platform services struct 
 * @retval [XL_ExitFunc] the exit func of the plugin or NULL if initialization failed.
 */
typedef XL_ExitFunc (*XL_InitFunc)(const XLEngine_Plugin_API *);

/** 
 * Named exported entry point into the plugin
 * This definition is required eventhough the function 
 * is loaded from a dynamic library by name
 * and casted to XL_InitFunc. If this declaration is 
 * commented out DynamicLibrary::getSymbol() fails
 *
 * The plugin's initialization function MUST be called "XL_initPlugin"
 * (and conform to the signature of course).
 *
 * @param  [const XL_PlatformServices *] params - the platform services struct 
 * @retval [XL_ExitFunc] the exit func of the plugin or NULL if initialization failed.
 */

#ifndef PLUGIN_API
    #if PLATFORM_WIN
        #define PLUGIN_API __declspec(dllimport)
    #else
        #define PLUGIN_API
    #endif
#endif

extern
#ifdef  __cplusplus
"C" 
#endif
PLUGIN_API XL_ExitFunc XL_initPlugin(const XLEngine_Plugin_API *API);

#ifdef  __cplusplus
}
#endif

#endif /* XL_PLUGIN_H */

