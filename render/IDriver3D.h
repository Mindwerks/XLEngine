#ifndef IDRIVER3D_H
#define IDRIVER3D_H

#include "Driver3D_IPlatform.h"
#include "../math/Matrix.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include <assert.h>

class IndexBuffer;

//Move into a seperate file.
class LightObject
{
public:
	LightObject(Vector3 vLoc) 
	{ 
		m_vLoc = vLoc; 
		m_fLightAnim0 = s_fAnimOffset0; s_fAnimOffset0 +=  8.0f*0.13f;
		m_fLightAnim1 = s_fAnimOffset1; s_fAnimOffset1 += 32.0f*0.13f;
		m_fIntensity = 0.2f * (sinf(m_fLightAnim0)*0.5f+0.5f) + 0.1f * (sinf(m_fLightAnim1)*0.5f+0.5f) + 0.7f;
	}
	~LightObject() {};

	void Update(float dt)
	{
		m_fLightAnim0 = fmodf( m_fLightAnim0+ 8.0f*dt, 6.283185307179586476925286766559f );
		m_fLightAnim1 = fmodf( m_fLightAnim1+32.0f*dt, 6.283185307179586476925286766559f );
		m_fIntensity = 0.2f * (sinf(m_fLightAnim0)*0.5f+0.5f) + 0.1f * (sinf(m_fLightAnim1)*0.5f+0.5f) + 0.7f;
	}

	Vector3 m_vLoc;
	float m_fLightAnim0;
	float m_fLightAnim1;
	float m_fIntensity;

	static float s_fAnimOffset0;
	static float s_fAnimOffset1;
};

class IDriver3D
{
	public:
		enum TexFormats_e
		{
			TEX_FORMAT_RGBA8=0,
			TEX_FORMAT_RGBA16F,
			TEX_FORMAT_RGBA32F,
			TEX_FORMAT_R32F,
			TEX_FORMAT_FORCE_32bpp,
			TEX_FORMAT_COUNT
		};

		enum VBO_Flags_e
		{
			VBO_POSITION_ONLY = 0,
			VBO_HAS_NORMALS	  = (1<<0),
			VBO_HAS_COLORS	  = (1<<1),
			VBO_HAS_TEXCOORDS = (1<<2),
			VBO_WORLDSPACE	  = (1<<3),
		};

		enum BlendMode_e
		{
			BLEND_NONE = 0,
			BLEND_ALPHA,
			BLEND_ADDITIVE,
			BLEND_COUNT
		};

		enum FilterTypes_e
		{
			FILTER_POINT=0,
			FILTER_NORMAL_NO_MIP,
			FILTER_NORMAL,
		};

		enum Extensions_e
		{
			EXT_TEXTURE_INDEX = (1<<0),
			EXT_GOURAUD		  = (1<<1),
			EXT_POLYGON_DATA  = (1<<2),
		};

		enum
		{
			TEXINDEX_EXT_FLIPU  = (1<<8),
			TEXINDEX_EXT_FLIPV  = (1<<9),
			TEXINDEX_EXT_ROTATE = (1<<10),
		};

    public:
        IDriver3D();
        virtual ~IDriver3D();

		virtual bool Init(s32 w, s32 h) {return false;}
		virtual void Present() {};
		virtual void Clear(bool bClearColor=true) {};

		virtual void SetWorldMatrix(Matrix *pMtx, s32 worldX, s32 worldY) {};
		virtual void SetViewMatrix(Matrix *pMtx, Vector3 *pLoc, Vector3 *pDir) {};
		virtual void SetProjMtx(Matrix *pMtx) {};
		virtual void SetCamera(Camera *pCamera) {};
		virtual Camera *GetCamera() {return 0;}

		virtual void ChangeWindowSize(s32 w, s32 h) {};

		//Texture Functions.
		//SetTexture(...) : frame=-1 : use the automatic texture animation system (used for most things).
		virtual void SetTexture(s32 slot, TextureHandle hTex, u32 uFilter=FILTER_NORMAL, bool bWrap=true, s32 frame=-1) {};
		virtual void SetColor(Vector4 *pColor=0) {};
		virtual TextureHandle CreateTexture(u32 uWidth, u32 uHeight, u32 uFormat=TEX_FORMAT_RGBA8, u8 *pData=0, bool bGenMips=false, s32 nFrameCnt=1) {return 0;}
		virtual void FillTexture(TextureHandle hTex, u8 *pData, u32 uWidth, u32 uHeight, bool bGenMips=false) {};
		virtual void FreeTexture(TextureHandle hTex) {};

		//VBO/IBO Support.
		virtual u32 CreateVBO() {return 0;}
		virtual void AllocVBO_Mem(u32 uID, u32 uVtxCnt, u32 uSize, bool bDynamic) {};
		virtual void FillVBO(u32 uID, void *pData, u32 uSize, bool bDynamic) {};
		virtual void SetVBO(u32 uID, u32 uStride, u32 uVBO_Flags) {};
		virtual u32 CreateIB() {return 0;}
		virtual void FillIB(u32 uID, void *pData, u32 uSize, bool bDynamic) {};
		virtual void ResetIBFlags(u32 uID) {};
		virtual void DeleteBuffer(u32 uID) {};
		virtual void ClearDrawData() {};

		//Draw!
		virtual void RenderIndexedTriangles(IndexBuffer *pIB, s32 nTriCnt, s32 startIndex=0) {};
		virtual void RenderScreenQuad(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot) {};
		virtual void RenderWorldQuad(const Vector3& pos0, const Vector3& pos1, const Vector2& uv0, const Vector2& uv1, const Vector4& color) {};
		virtual void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4& color, bool bRecieveLighting=false) {};
		virtual void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4 *color, bool bRecieveLighting=false) {};

		//Render States
		virtual void EnableDepthWrite(bool bEnable) {};
		virtual void EnableDepthRead(bool bEnable) {};
		virtual void EnableStencilWriting(bool bEnable, u32 uValue) {};
		virtual void EnableStencilTesting(bool bEnable) {};
		virtual void EnableCulling(bool bEnable) {};
		virtual void EnableAlphaTest(bool bEnable, u8 uAlphaCutoff=128) {};
		virtual void SetBlendMode(u32 uMode=BLEND_NONE) {};
		virtual void EnableFog(bool bEnable, f32 fEnd=10000.0f) {};
		virtual void SetFogDensity(f32 fDensity=1.0f) {};

		//Palette
		void SetCurrentPalette(u32 uPalID, bool UpdatePal=false) { m_uPaletteID = uPalID; m_bUpdatePal = UpdatePal; }
		void SetCurrentColormap(u32 uColormapID) {m_uColormapID = uColormapID; }
		void SetCurrentClearColor(u32 uColor) {m_uClearColor = uColor;}
		virtual void SetClearColorFromTex(TextureHandle hTex) {};

		void SetPlatform( Driver3D_IPlatform *platform ) { m_Platform = platform; }
		void SetWindowData(s32 nParam, void **param) { m_Platform->SetWindowData(nParam, param); }

		//Sorting
		virtual bool ApplyOpaqueSort() { return false; }
		virtual bool ApplyTransSort()  { return false; }

		//Render Camera
		const Matrix& GetRenderCam_ViewMtx() { return m_ViewMtx; }

		void GetWindowSize(s32& nWidth, s32& nHeight) 
		{ 
			nWidth = m_nWindowWidth; nHeight = m_nWindowHeight; 
		}

		//Lights.
		void SetLights(int nLightCnt, const LightObject **apLights) { m_nLightCnt = nLightCnt; m_apLights = apLights; }
		const LightObject **GetLights() { return m_apLights; }
		int GetLightCount() { return m_nLightCnt; }
		void SetSunlight(const Vector3& dir) { m_SunlightDir = dir; }
		void SetAmbient(float fAmbient) { m_fAmbient = fAmbient; }
		float GetAmbient() { return m_fAmbient; }

		const Vector3& GetEyePos()  { return m_Eye; }
		const Vector3& GetViewDir() { return m_ViewDir; }

		//Overlays
		void AddOverlay(s32 x, s32 y, s32 scale, TextureHandle hTex);

		//Driver extensions
		bool HasExtension(u32 uExtension) { return (m_uExtensions&uExtension)?true:false; }
		//Force mipmapping if the renderer has support for it, even in software.
		void ForceMipmapping(bool bForce) { m_bForceMip = bForce; }
		bool GetForceMipmapping() { return m_bForceMip; }
		virtual void SetExtension_Data(u32 uExtension, void *pData0, void *pData1) {};
			
    protected:
		struct Overlay
		{
			s32 x;
			s32 y;
			s32 scale;
			TextureHandle hTex;
		};

		enum
		{
			MAX_OVERLAY_COUNT = 16
		};

        Driver3D_IPlatform *m_Platform;
		Matrix m_ViewMtx;
		Matrix m_WorldMtx;
		Matrix m_WorldView;
		Vector3 m_Eye;
		Vector3 m_ViewDir;

		u32 m_uOverlayCount;
		Overlay m_Overlays[MAX_OVERLAY_COUNT];

		s32 m_nWindowWidth;
		s32 m_nWindowHeight;
		u32 m_uPaletteID;
		u32 m_uColormapID;
		u32 m_uClearColor;
		u32 m_uExtensions;

		float m_fAmbient;

		bool m_bForceMip;
		bool m_bUpdatePal;

		int m_nLightCnt;
		const LightObject **m_apLights;
		Vector3 m_SunlightDir;
};

#endif // IDRIVER3D_H
