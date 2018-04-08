#ifndef LOGIC_DOOR_H
#define LOGIC_DOOR_H

#include "../plugin_framework/plugin.h"
#include "../world/ObjectDef.h"
#include <string>

class Logic_Door {
public:
    Logic_Door(const XLEngine_Plugin_API *API);

    ~Logic_Door(void);

private:
    const XLEngine_Plugin_API *m_pAPI;

    void LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    void ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    void Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    void Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    LOGIC_CB_FUNC();
};

#endif //LOGIC_DOOR_H