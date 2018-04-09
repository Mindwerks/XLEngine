#ifndef LEVELFUNCMGR_H
#define LEVELFUNCMGR_H

#include "../CommonTypes.h"
#include "LevelFunc.h"

#include <map>
#include <string>
#include <vector>

class WorldCell;

class LevelFuncMgr
{
public:
    static bool Init();
    static void Destroy();

    static void SetWorldCell(WorldCell *pCell) { m_pWorldCell = pCell; }
    static LevelFunc *CreateLevelFunc(const char *pszFuncName, int32_t nSector, int32_t nWall);
    static void DestroyLevelFunc(LevelFunc *pFunc);

    static void AddToActiveList(LevelFunc *pFunc);
    static void RemoveFromActiveList(LevelFunc *pFunc);
    static void Update();

    static void AddLevelFuncCB(const std::string& sFuncName, LevelFunc::LFunc_ActivateCB pActivate, LevelFunc::LFunc_SetValueCB pSetValue);

private:
    struct LevelFuncCB
    {
        LevelFunc::LFunc_ActivateCB activateCB;
        LevelFunc::LFunc_SetValueCB setValueCB;
    };

    static std::vector<LevelFunc *> m_FuncList;
    static std::vector<LevelFunc *> m_Active;
    static std::vector<LevelFunc *> m_AddList;
    static std::vector<LevelFunc *> m_RemoveList;

    static WorldCell *m_pWorldCell;

    static std::map<std::string, LevelFuncCB> m_LevelFuncCB;
};

#endif //LEVELFUNCMGR_H
