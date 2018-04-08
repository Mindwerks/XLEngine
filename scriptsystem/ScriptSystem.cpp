#include "ScriptSystem.h"

#if PLATFORM_WIN
#include "Windows.h"
#endif
//#include "System.h"
#include "../ui/XL_Console.h"

asIScriptEngine *m_Engine;
asIScriptContext *m_pContext;
float m_afGlobalStore[32];
char m_szGameDir[260];
int m_aTimers[32];

char _szTempString[64];

const char *pszModules[] =
        {
                "MODULE_LOGICS",
                "MODULE_WEAPONS",
                "MODULE_UI",
                "MODULE_UI_EDITOR"
        };

const char *pszSections[] =
        {
                "SECTION_CORE",
                "SECTION_USER",
        };

void _MessageCallback(const asSMessageInfo *msg, void *param) {
    const char *type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";

    char szDebugOut[256];
    sprintf(szDebugOut, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
#if PLATFORM_WIN
    OutputDebugString(szDebugOut);
#else
    printf(szDebugOut);
#endif

    sprintf(szDebugOut, "^1ScriptError: %s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
    XL_Console::Print(szDebugOut);
}

bool ScriptSystem::Init() {
    // Create the script engine
    m_Engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if (m_Engine == NULL)
    {
        return false;
    }

    // The script compiler will write any compiler messages to the callback.
    m_Engine->SetMessageCallback(asFUNCTION(_MessageCallback), 0, asCALL_CDECL);

    // Register the script string type
    // Look at the implementation for this function for more information
    // on how to register a custom string type, and other object types.
    // The implementation is in "/add_on/scriptstring/scriptstring.cpp"
    RegisterStdString(m_Engine);
    RegisterScriptArray(m_Engine, true);

    m_pContext = NULL;

    memset(m_afGlobalStore, 0, sizeof(float) * 32);
    memset(m_aTimers, 0, sizeof(int) * 32);

    XL_Console::Print("Script System Initialized.");

    return true;
}

void ScriptSystem::Destroy() {
    if (m_pContext) m_pContext->Release();
    if (m_Engine) m_Engine->Release();
}

bool ScriptSystem::RegisterFunc(const char *decl, const asSFuncPtr &pFunc) {
    int r = m_Engine->RegisterGlobalFunction(decl, pFunc, asCALL_CDECL);
    return (r >= 0) ? true : false;
}

bool ScriptSystem::RegisterVar(const char *decl, void *pVar) {
    int r = m_Engine->RegisterGlobalProperty(decl, pVar);
    return (r >= 0) ? true : false;
}

void ScriptSystem::SetGameDir(const char *pszGameDir) {
    strcpy(m_szGameDir, pszGameDir);
}

bool ScriptSystem::ReloadScript(int nModule, int nSection, const char *pszFile, bool bBuildModule) {
    FILE *f = fopen(pszFile, "rb");
    if (!f)
    { return false; }

    // Determine the size of the file
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *script = xlNew char[length];
    char *final_script = xlNew char[length];
    fread(script, length, 1, f);
    fclose(f);

    //discard existing script code for this module.
    m_Engine->DiscardModule(pszModules[nModule]);

    //now look for #includes.
    length = FindAndLoadIncludes(script, length, final_script, nModule, nSection);

    // Compile the script
    asIScriptModule *pModule = m_Engine->GetModule(pszModules[nModule], asGM_CREATE_IF_NOT_EXISTS);
    pModule->AddScriptSection(pszSections[nSection], final_script, length);
    if (bBuildModule)
    {
        pModule->Build();
    }

    xlDelete[] script;
    xlDelete[] final_script;

    if (!m_pContext)
    {
        m_pContext = m_Engine->CreateContext();
    }

    return true;
}

int ScriptSystem::FindAndLoadIncludes(const char *inScript, int inLength, char *outScript, int nModule, int nSection) {
    int outLength = inLength;

    //1. look for and load includes
    int incEnd = 0;
    for (int l = 0; l < inLength - 8; l++)
    {
        if (inScript[l] == '#')
        {
            if (inScript[l + 1] == 'i' && inScript[l + 7] == 'e')
            {
                l += 8;
                //now keep looking until a " is found.
                while (inScript[l] != '"')
                { l++; }
                l++;
                char szInclude[32];
                char szPath[64];
                int c = 0;
                while (inScript[l] != '"')
                {
                    szInclude[c++] = inScript[l];
                    l++;
                }
                szInclude[c] = 0;
                l++;
                incEnd = l;

                sprintf(szPath, "%s%s", m_szGameDir, szInclude);
                LoadScript(nModule, nSection, szPath, false);
            }
        }
    }

    //2. copy the script, starting on the line AFTER the last include
    if (incEnd == 0)
    {
        memcpy(outScript, inScript, inLength);
    }
    else
    {
        outLength = inLength - incEnd;
        memcpy(outScript, &inScript[incEnd], outLength);
    }

    return outLength;
}

bool ScriptSystem::LoadScript(int nModule, int nSection, const char *pszFile, bool bBuildModule) {
    FILE *f = fopen(pszFile, "rb");
    if (!f)
    { return false; }

    // Determine the size of the file
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *script = xlNew char[length];
    char *final_script = xlNew char[length];
    fread(script, length, 1, f);
    fclose(f);

    //now look for #includes.
    length = FindAndLoadIncludes(script, length, final_script, nModule, nSection);

    // Compile the script
    asIScriptModule *pModule = m_Engine->GetModule(pszModules[nModule], asGM_CREATE_IF_NOT_EXISTS);
    int ret = pModule->AddScriptSection(pszSections[nSection], final_script, length);
    if (bBuildModule)
    {
        ret = pModule->Build();
    }

    xlDelete[] script;
    xlDelete[] final_script;

    if (!m_pContext)
    {
        m_pContext = m_Engine->CreateContext();
    }

    return true;
}

SHANDLE ScriptSystem::GetFunc(int nModule, const char *pszFunc) {
    // Do some preparation before execution
    asIScriptModule *pModule = m_Engine->GetModule(pszModules[nModule], asGM_ONLY_IF_EXISTS);
    if (pModule)
    {
        return pModule->GetFunctionIdByName(pszFunc);
    }
    return -1;
}

void ScriptSystem::SetCurFunction(SHANDLE hFunc) {
    // Execute the script function
    m_pContext->Prepare(hFunc);
}

void ScriptSystem::ExecuteFunc() {
    m_pContext->Execute();
}

uint32_t
ScriptSystem::ExecuteFunc(SHANDLE hFunc, int32_t nArgCnt, const ScriptArgument *pArgs, bool bRetValueExpected) {
    uint32_t uRetValue = 0;
    if (hFunc >= 0)
    {
        m_pContext->Prepare(hFunc);
        if (nArgCnt > 0 && pArgs)
        {
            for (int32_t i = 0; i < nArgCnt; i++)
            {
                switch (pArgs[i].uType)
                {
                    case SC_ARG_uint8_t:
                        m_pContext->SetArgByte(i, pArgs[i].arguint8_t);
                        break;
                    case SC_ARG_uint16_t:
                        m_pContext->SetArgWord(i, pArgs[i].arguint16_t);
                        break;
                    case SC_ARG_uint32_t:
                        m_pContext->SetArgDWord(i, pArgs[i].arguint32_t);
                        break;
                    case SC_ARG_float:
                        m_pContext->SetArgFloat(i, pArgs[i].argfloat);
                        break;
                };
            }
        }
        m_pContext->Execute();
        if (bRetValueExpected)
        {
            uRetValue = (uint32_t) m_pContext->GetReturnDWord();
        }
    }
    return uRetValue;
}

void ScriptSystem::SetGlobalStoreVal(int var, float val) {
    if (var >= 0 && var < 32)
    {
        m_afGlobalStore[var] = val;
    }
}

float ScriptSystem::GetGlobalStoreVal(int var) {
    float ret = 0.0f;

    if (var >= 0 && var < 32)
    {
        ret = m_afGlobalStore[var];
    }

    return ret;
}

//Timers
void ScriptSystem::SetTimer(int timer, int delay) {
    if (timer >= 0 && timer < 32)
    {
        m_aTimers[timer] = delay;
    }
}

int ScriptSystem::GetTimer(int timer) {
    if (timer >= 0 && timer < 32)
    {
        return m_aTimers[timer];
    }
    return 0;
}

void ScriptSystem::System_Print(string &szItem) {
    //System::SetTextDisp( szItem.c_str() );
}

void ScriptSystem::System_PrintIndex(int idx) {
    //System::SetTextDispIndex(idx);
}

void ScriptSystem::System_StartString() {
    _szTempString[0] = 0;
}

void ScriptSystem::System_EndString() {
    //System::SetTextDisp(_szTempString);
}

void ScriptSystem::System_AppendString(string &szStr) {
    sprintf(_szTempString, "%s%s", _szTempString, szStr.c_str());
}

void ScriptSystem::System_AppendFloat(float fVal) {
    sprintf(_szTempString, "%s%f", _szTempString, fVal);
}

void ScriptSystem::System_AppendInt(int iVal) {
    sprintf(_szTempString, "%s%d", _szTempString, iVal);
}

void ScriptSystem::Update() {
    //Log(LogVerbose, "ScriptSystem Update");

    for (int i = 0; i < 32; i++)
    {
        if (m_aTimers[i] > 0)
        {
            m_aTimers[i]--;
        }
    }
}

/*******************************
  Plugin API
 *******************************/
int32_t ScriptSystem_RegisterScriptFunc(const char *decl, const asSFuncPtr &pFunc) {
    bool bSuccess = ScriptSystem::RegisterFunc(decl, pFunc);
    return (bSuccess) ? 1 : 0;
}