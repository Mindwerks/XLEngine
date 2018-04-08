#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "../CommonTypes.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Matrix.h"
#include <vector>

class IDriver3D;

class VertexBuffer;

class IndexBuffer;

using namespace std;


class CommandBuffer {
public:
    //Initialize memory.
    void Init(IDriver3D *pDriver);

    void Destroy();

    //Commands
    void SetBlendMode(uint32_t uMode);

    void SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter, bool bWrap);

    void SetVertexBuffer(VertexBuffer *pVB);

    void DrawIndexed(IndexBuffer *pIB, int32_t nStartIndex, int32_t nTriCnt);

    //Higher level commands.
    void DrawCall(const Matrix &mWorld, TextureHandle hTex, VertexBuffer *pVB, IndexBuffer *pIB, int32_t nStartIndex,
                  int32_t nTriCnt);

    //Custom command, returns memory allocated from the command buffer.
    void *CB_Command(uint32_t uSize);

    //Execution.
    void Execute();

private:
    enum {
        CB_BLEND_MODE = 0,
        CB_SET_TEXTURE,
        CB_SET_VERTEXBUFFER,
        CB_DRAW_INDEXED,
        CB_DRAW_CALL,
        CB_END_MARKER
    };

    struct CB_Base {
        uint16_t type;
        uint16_t size;
    };

    struct CB_BlendMode {
        uint16_t type;
        uint16_t size;
        uint32_t blendMode;
    };

    struct CB_SetTexture {
        uint16_t type;
        uint16_t size;
        int32_t slot;
        TextureHandle hTex;
        uint32_t uFilter;
        bool bWrap;
    };

    struct CB_SetVertexBuffer {
        uint16_t type;
        uint16_t size;
        VertexBuffer *pVB;
    };

    struct CB_DrawIndexed {
        uint16_t type;
        uint16_t size;
        IndexBuffer *pIB;
        int startIndex;
        int primCount;
    };

    struct CB_DrawCall {
        uint16_t type;
        uint16_t size;
        TextureHandle hTex;
        Matrix mWorld;
        VertexBuffer *pVB;
        IndexBuffer *pIB;
        int32_t nStartIndex;
        int32_t nTriCnt;
    };

    enum {
        BUFFER_COUNT = 3,
        BUFFER_SIZE = 1048576,    //1MB.
        END_MARKER_SIZE = 4
    };

    static IDriver3D *s_pDriver;
    static uint8_t *s_pCommandBuffer[BUFFER_COUNT];
    static uint32_t s_uReadBuffer;
    static uint32_t s_uWriteBuffer;
    static uint32_t s_uCommandPos;
};

#endif //COMMANDBUFFER_H
