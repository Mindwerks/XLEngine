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


class CommandBuffer
{
public:
	//Initialize memory.
	void Init(IDriver3D *pDriver);
	void Destroy();

	//Commands
	void SetBlendMode(u32 uMode);
	void SetTexture(s32 slot, TextureHandle hTex, u32 uFilter, bool bWrap);
	void SetVertexBuffer(VertexBuffer *pVB);
	void DrawIndexed(IndexBuffer *pIB, s32 nStartIndex, s32 nTriCnt);

	//Higher level commands.
	void DrawCall(const Matrix& mWorld, TextureHandle hTex, VertexBuffer *pVB, IndexBuffer *pIB, s32 nStartIndex, s32 nTriCnt);

	//Custom command, returns memory allocated from the command buffer.
	void *CB_Command(u32 uSize);

	//Execution.
	void Execute();

private:
	enum
	{
		CB_BLEND_MODE=0,
		CB_SET_TEXTURE,
		CB_SET_VERTEXBUFFER,
		CB_DRAW_INDEXED,
		CB_DRAW_CALL,
		CB_END_MARKER
	};

	struct CB_Base
	{
		u16 type;
		u16 size;
	};

	struct CB_BlendMode
	{
		u16 type;
		u16 size;
		u32 blendMode;
	};

	struct CB_SetTexture
	{
		u16 type;
		u16 size;
		s32 slot;
		TextureHandle hTex;
		u32 uFilter;
		bool bWrap;
	};

	struct CB_SetVertexBuffer
	{
		u16 type;
		u16 size;
		VertexBuffer *pVB;
	};

	struct CB_DrawIndexed
	{
		u16 type;
		u16 size;
		IndexBuffer *pIB;
		int startIndex;
		int primCount;
	};

	struct CB_DrawCall
	{
		u16 type;
		u16 size;
		TextureHandle hTex;
		Matrix mWorld;
		VertexBuffer *pVB;
		IndexBuffer *pIB;
		s32 nStartIndex;
		s32 nTriCnt;
	};

	enum
	{
		BUFFER_COUNT = 3,
		BUFFER_SIZE  = 1048576,	//1MB.
		END_MARKER_SIZE = 4
	};

	static IDriver3D *s_pDriver;
	static u8 *s_pCommandBuffer[ BUFFER_COUNT ];
	static u32 s_uReadBuffer;
	static u32 s_uWriteBuffer;
	static u32 s_uCommandPos;
};

#endif //COMMANDBUFFER_H
