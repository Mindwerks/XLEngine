#ifndef LOGICDEF_H
#define LOGICDEF_H

#include "../CommonTypes.h"

//These are accessed via the "C" interface which is why they exist outside of the parent class.
enum LogicType_e {
    LTYPE_CODE = 0,
    LTYPE_SCRIPT,
    LTYPE_COUNT
};

enum LogicCallback_e {
    LCB_LOGIC_SETUP = 0,
    LCB_OBJECT_SETUP,
    LCB_UPDATE,
    LCB_MESSAGE,
    LCB_COUNT,

    MAX_LOGIC_PARAM = 2
};

union LogicParam {
    uint32_t uParam;
    int32_t nParam;
    float fParam;
    void *pParam;
};

//default messages.
enum LogicMessages_e {
    LMSG_ACTIVATE = (1 << 0),
    LMSG_CHILD_ACTIVATE = (1 << 1),
};

typedef void (*LogicFunction)(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

#endif //LOGICDEF_H
