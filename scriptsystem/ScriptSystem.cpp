#include "ScriptSystem.h"
#if PLATFORM_WIN
    #include "Windows.h"
#endif
//#include "System.h"
#include "../ui/XL_Console.h"
#include "../fileformats/Vfs.h"
#include <algorithm>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <memory.h>

#include "scriptstdstring.h"
#include "scriptarray.h"

asIScriptEngine *m_Engine;
asIScriptContext *m_pContext;
float m_afGlobalStore[32];
char m_szGameDir[260];
int m_aTimers[32];

char _szTempString[64];

const char *pszModules[]=
{
    "MODULE_LOGICS",
    "MODULE_WEAPONS",
    "MODULE_UI",
    "MODULE_UI_EDITOR"
};

const char *pszSections[]=
{
    "SECTION_CORE",
    "SECTION_USER",
};

void _MessageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";

    char szDebugOut[256];
    sprintf(szDebugOut, "^1ScriptError: %s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
    XL_Console::Print(szDebugOut);
}

bool ScriptSystem::Init()
{
    // Create the script engine
    m_Engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if( m_Engine == nullptr )
    {
        return false;
    }

    // The script compiler will write any compiler messages to the callback.
    m_Engine->SetMessageCallback( asFUNCTION(_MessageCallback), 0, asCALL_CDECL );

    // Register the script string type
    // Look at the implementation for this function for more information
    // on how to register a custom string type, and other object types.
    // The implementation is in "/add_on/scriptstring/scriptstring.cpp"
    RegisterStdString(m_Engine);
    RegisterScriptArray(m_Engine, true);

    m_pContext = nullptr;

    memset(m_afGlobalStore, 0, sizeof(float)*32);
    memset(m_aTimers, 0, sizeof(int)*32);

    XL_Console::Print("Script System Initialized.");

    return true;
}

void ScriptSystem::Destroy()
{
    if (m_pContext) m_pContext->Release();
    if (m_Engine)   m_Engine->Release();
}

bool ScriptSystem::RegisterFunc(const char *decl, const asSFuncPtr& pFunc)
{
    int r = m_Engine->RegisterGlobalFunction(decl, pFunc, asCALL_CDECL);
    return (r >= 0) ? true : false;
}

bool ScriptSystem::RegisterVar(const char *decl, void *pVar)
{
    int r = m_Engine->RegisterGlobalProperty(decl, pVar);
    return (r >= 0) ? true : false;
}

void ScriptSystem::SetGameDir(const char *pszGameDir)
{
    strcpy(m_szGameDir, pszGameDir);
}

bool ScriptSystem::ReloadScript(int nModule, int nSection, const char *pszFile, bool bBuildModule)
{
    istream_ptr file = Vfs::get().openInput(pszFile);
    if(!file) return false;

    // Determine the size of the file
    if(!file->seekg(0, std::ios_base::end))
        return false;
    size_t length = file->tellg();
    file->seekg(0);

    std::vector<char> script(length);
    file->read(script.data(), script.size());
    file = nullptr;

    //discard existing script code for this module.
    m_Engine->DiscardModule(pszModules[nModule]);

    //now look for #includes.
    FindAndLoadIncludes(script, nModule, nSection);

    // Compile the script
    asIScriptModule *pModule = m_Engine->GetModule(pszModules[nModule], asGM_CREATE_IF_NOT_EXISTS);
    pModule->AddScriptSection(pszSections[nSection], script.data(), script.size());
    if(bBuildModule) pModule->Build();

    if(!m_pContext)
        m_pContext = m_Engine->CreateContext();

    return true;
}

void ScriptSystem::FindAndLoadIncludes(std::vector<char> &script, int nModule, int nSection)
{
    //1. look for and load includes
    auto iter = std::find(script.begin(), script.end(), '#');
    while(script.end()-iter > 10)
    {
        if(iter != script.begin() && *(iter-1) != '\n')
        {
            iter = std::find(iter+1, script.end(), '#');
            continue;
        }

        if(std::strncmp(&*(iter+1), "include", 7) != 0 || !std::isspace(*(iter+8)))
        {
            iter = std::find(iter+1, script.end(), '#');
            continue;
        }
        auto incStart = iter;

        iter += 9;
        while(iter != script.end() && std::isspace(*iter))
            ++iter;
        if(iter == script.end() || *iter != '"')
            continue;
        ++iter;
        std::string incname;
        while(iter != script.end() && *iter != '"')
            incname += *(iter++);
        while(iter != script.end() && *iter != '\n')
            ++iter;

        // Erase the include statement, leaving the line numbers and rest of
        // the file intact.
        if(incStart == script.begin())
        {
            script.erase(incStart, iter);
            iter = std::find(script.begin(), script.end(), '#');
        }
        else
        {
            script.erase((--incStart)+1, iter);
            iter = std::find(incStart+1, script.end(), '#');
        }

        incname = m_szGameDir+incname;
        LoadScript(nModule, nSection, incname.c_str(), false);
    }
}

bool ScriptSystem::LoadScript(int nModule, int nSection, const char *pszFile, bool bBuildModule)
{
    istream_ptr file = Vfs::get().openInput(pszFile);
    if(!file) return false;

    // Determine the size of the file
    if(!file->seekg(0, std::ios_base::end))
        return false;
    size_t length = file->tellg();
    file->seekg(0);

    std::vector<char> script(length);
    file->read(script.data(), script.size());
    file = nullptr;

    //now look for #includes.
    FindAndLoadIncludes(script, nModule, nSection);

    // Compile the script
    asIScriptModule *pModule = m_Engine->GetModule(pszModules[nModule], asGM_CREATE_IF_NOT_EXISTS);
    int ret = pModule->AddScriptSection(pszSections[nSection], script.data(), script.size());
    if(bBuildModule) ret = pModule->Build();

    if(!m_pContext)
        m_pContext = m_Engine->CreateContext();

    return true;
}

SHANDLE ScriptSystem::GetFunc(int nModule, const char *pszFunc)
{
    // Do some preparation before execution
    asIScriptModule *pModule = m_Engine->GetModule(pszModules[nModule], asGM_ONLY_IF_EXISTS);
    if ( pModule )
    {
        return pModule->GetFunctionByName(pszFunc);
    }
    return nullptr;
}

void ScriptSystem::SetCurFunction(SHANDLE hFunc)
{
    // Execute the script function
    m_pContext->Prepare(hFunc);
}

void ScriptSystem::ExecuteFunc()
{
    m_pContext->Execute();
}

uint32_t ScriptSystem::ExecuteFunc(SHANDLE hFunc, int32_t nArgCnt, const ScriptArgument *pArgs, bool bRetValueExpected)
{
    uint32_t uRetValue = 0;
    if ( hFunc )
    {
        int32_t return_code = 0;
        return_code = m_pContext->Prepare(hFunc);
        if (return_code < asSUCCESS){
            XL_Console::PrintF("AS return code %d in %s:%s ", return_code, hFunc->GetModuleName(), hFunc->GetName());
            return uRetValue;
        }

        if ( nArgCnt > 0 && pArgs )
        {
            for (int32_t i=0; i<nArgCnt; i++)
            {
                switch (pArgs[i].uType)
                {
                    case SC_ARG_uint8_t:
                        return_code =m_pContext->SetArgByte(i, pArgs[i].arguint8_t);
                        break;
                    case SC_ARG_uint16_t:
                        return_code = m_pContext->SetArgWord(i, pArgs[i].arguint16_t);
                        break;
                    case SC_ARG_uint32_t:
                        return_code = m_pContext->SetArgDWord(i, pArgs[i].arguint32_t);
                        break;
                    case SC_ARG_float:
                        return_code = m_pContext->SetArgFloat(i, pArgs[i].argfloat);
                        break;
                    default:
                        XL_Console::PrintF("Bad Argument Type %i", pArgs[i].uType);
                };
                if (return_code < asSUCCESS){
                    XL_Console::PrintF("AS return code %d in %s:%s ", return_code, hFunc->GetModuleName(), hFunc->GetName());
                    return uRetValue;
                }
            }
        }

        return_code = m_pContext->Execute();
        if (return_code < asSUCCESS){
            XL_Console::PrintF("AS return code %d in %s:%s ", return_code, hFunc->GetModuleName(), hFunc->GetName());
            return uRetValue;
        }

        if ( bRetValueExpected )
        {
            uRetValue = (uint32_t)m_pContext->GetReturnDWord();
        }
    }
    return uRetValue;
}

void ScriptSystem::SetGlobalStoreVal(int var, float val)
{
    if ( var >= 0 && var < 32 )
    {
        m_afGlobalStore[var] = val;
    }
}

float ScriptSystem::GetGlobalStoreVal(int var)
{
    float ret = 0.0f;

    if ( var >= 0 && var < 32 )
    {
        ret = m_afGlobalStore[var];
    }

    return ret;
}

//Timers
void ScriptSystem::SetTimer(int timer, int delay)
{
    if ( timer >= 0 && timer < 32 )
    {
        m_aTimers[ timer ] = delay;
    }
}

int ScriptSystem::GetTimer(int timer)
{
    if ( timer >= 0 && timer < 32 )
    {
        return m_aTimers[ timer ];
    }
    return 0;
}

void ScriptSystem::System_Print(std::string &szItem)
{
    //System::SetTextDisp( szItem.c_str() );
}

void ScriptSystem::System_PrintIndex(int idx)
{
    //System::SetTextDispIndex(idx);
}

void ScriptSystem::System_StartString()
{
    _szTempString[0] = 0;
}

void ScriptSystem::System_EndString()
{
    //System::SetTextDisp(_szTempString);
}

void ScriptSystem::System_AppendString(std::string& szStr)
{
    sprintf(_szTempString, "%s%s", _szTempString, szStr.c_str());
}

void ScriptSystem::System_AppendFloat(float fVal)
{
    sprintf(_szTempString, "%s%f", _szTempString, fVal);
}

void ScriptSystem::System_AppendInt(int iVal)
{
    sprintf(_szTempString, "%s%d", _szTempString, iVal);
}

void ScriptSystem::Update()
{
    //Log(LogVerbose, "ScriptSystem Update");

    for (int i=0; i<32; i++)
    {
        if ( m_aTimers[i] > 0 )
        {
            m_aTimers[i]--;
        }
    }
}

/*******************************
  Plugin API
 *******************************/
int32_t ScriptSystem_RegisterScriptFunc(const char *decl, const asSFuncPtr& pFunc)
{
    bool bSuccess = ScriptSystem::RegisterFunc(decl, pFunc);
    return (bSuccess) ? 1 : 0;
}
