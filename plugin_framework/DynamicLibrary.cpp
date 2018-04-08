#include "../Engine.h"

#if PLATFORM_WIN
  #include <Windows.h>
#else
  #include <dlfcn.h>
#endif

#include "DynamicLibrary.h"
#include <sstream>
#include <iostream>

#if PLATFORM_OSX
    static string dynamicLibraryExtension("dylib");
#elif PLATFORM_LINUX
    static string dynamicLibraryExtension("so");
#elif PLATFORM_WIN
    static string dynamicLibraryExtension("dll");
#endif

DynamicLibrary::DynamicLibrary(void *handle)
{
    m_handle = handle;
}

DynamicLibrary::~DynamicLibrary()
{
    if (m_handle)
    {
        #if PLATFORM_WIN
            FreeLibrary( (HMODULE)m_handle );
        #else
            dlclose( m_handle );
        #endif
    }
}

DynamicLibrary *DynamicLibrary::Load(const string& name, string& errorString)
{
    if (name.empty()) 
    {
        errorString = "Empty path.";
        return NULL;
    }

    void *handle = NULL;
    string path = name + "." + dynamicLibraryExtension;

#if PLATFORM_WIN
    handle = LoadLibraryA( path.c_str() );
    if ( !handle )
    {
        DWORD errorCode = GetLastError();
        stringstream ss;
        ss << std::string("LoadLibrary(") << name 
           << std::string(") Failed. errorCode: ") 
           << errorCode; 
        errorString = ss.str();

        return NULL;
    }
#else
    handle = dlopen( path.c_str(), RTLD_NOW );
    if (!handle) 
    {
        string dlErrorString;
        const char *zErrorString = dlerror();
        if (zErrorString)
            dlErrorString = zErrorString;

        errorString += "Failed to load \"" + name + '"';
        if(dlErrorString.size())
            errorString += ": " + dlErrorString;

        return NULL;
    }
#endif

    return xlNew DynamicLibrary(handle);
}

void *DynamicLibrary::GetSymbol(const std::string& symbol)
{
    if ( !m_handle )
        return NULL;

#if PLATFORM_WIN
    return GetProcAddress( (HMODULE)m_handle, symbol.c_str() );
#else
    return dlsym( m_handle, symbol.c_str() );
#endif
}

