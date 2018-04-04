#include "Object.h"
#include "Sprite_ZAxis.h"
#include "../render/IDriver3D.h"
#include "../render/RenderQue.h"
#include "../render/Camera.h"

#define _FRAME_DELAY 4

Sprite_ZAxis::Sprite_ZAxis() : RenderComponent()
{
	m_hTex = XL_INVALID_TEXTURE;
	m_aFlip[0] = 0;
	m_aFlip[1] = 0;
	m_aFlip[2] = 0;
	m_fBaseItens = 1.0f;
	m_fAlpha = 1.0f;
	m_uCurFrame = 0;
	m_uFlags = 0;
	m_nFrameDelay = _FRAME_DELAY;
}

Sprite_ZAxis::~Sprite_ZAxis()
{
	RenderComponent::~RenderComponent();
}

void Sprite_ZAxis::Render(Object *pObj, IDriver3D *pDriver, f32 fIntensity, const Vector3& vOffset)
{
	const Matrix& viewMtx = pDriver->GetRenderCam_ViewMtx();

	ObjectPhysicsData *pData = pObj->GetPhysicsData();
	Vector3 vRight(viewMtx.m[0], viewMtx.m[4], viewMtx.m[8]);
	Vector3 vUp(viewMtx.m[1], viewMtx.m[5], viewMtx.m[9]);
	Vector3 vDir(viewMtx.m[2], viewMtx.m[6], viewMtx.m[10]);

	Vector3 vFinalOffset = vOffset;
	Camera *pCamera = pDriver->GetCamera();
	vFinalOffset.x += (pObj->GetWorldX() - pCamera->GetWorldPosX())*1024.0f;
	vFinalOffset.y += (pObj->GetWorldY() - pCamera->GetWorldPosY())*1024.0f;
	
	Vector3 A, B;
	A = pData->m_Loc - vRight*pData->m_Scale - pData->m_Up*pData->m_Scale + vFinalOffset;
	B = pData->m_Loc + vRight*pData->m_Scale + pData->m_Up*pData->m_Scale + vFinalOffset;

	Vector3 posList[4];
	Vector2 uvList[4];
	if ( pData->m_Up.z > 0.7071f )
	{
		posList[0].Set(A.x, A.y, A.z);
		posList[1].Set(B.x, B.y, A.z);
		posList[2].Set(B.x, B.y, B.z);
		posList[3].Set(A.x, A.y, B.z);
	}
	else
	{
		posList[0].Set(A.x, A.y, A.z);
		posList[1].Set(B.x, A.y, A.z);
		posList[2].Set(B.x, B.y, B.z);
		posList[3].Set(A.x, B.y, B.z);
	}

	Vector2 uv0(m_aFlip[0]?1.0f:0.0f, m_aFlip[1]?0.0f:1.0f);
	Vector2 uv1(m_aFlip[0]?0.0f:1.0f, m_aFlip[1]?1.0f:0.0f);

	if ( m_aFlip[2] )
	{
		uvList[0].Set( uv0.y, uv0.x );
		uvList[1].Set( uv0.y, uv1.x );
		uvList[2].Set( uv1.y, uv1.x );
		uvList[3].Set( uv1.y, uv0.x );
	}
	else
	{
		uvList[0].Set( uv0.x, uv0.y );
		uvList[1].Set( uv1.x, uv0.y );
		uvList[2].Set( uv1.x, uv1.y );
		uvList[3].Set( uv0.x, uv1.y );
	}

	fIntensity *= m_fBaseItens;
	Vector4 color(fIntensity, fIntensity, fIntensity, m_fAlpha);
	RenderQuad *pQuad = RenderQue::GetRenderQuad();
	pQuad->hTex = m_hTex;
	pQuad->color = color;
	memcpy(pQuad->posList, posList, 4*sizeof(Vector3));
	memcpy(pQuad->uvList, uvList, 4*sizeof(Vector2));
	pQuad->bForceZWrite = false;
	pQuad->bApplyLighting = !IsFlagSet(FLAG_EMISSIVE);

	//now render the "FX Frames" - used for things like adding fire to torches.
	if ( m_fxFrames.size() > 0 )
	{
		f32 yScale = (f32)m_fxFrames[ m_uCurFrame ].uHeight / (f32)m_fxFrames[ m_uCurFrame ].uWidth;
		posList[0].z += pData->m_Up.z*pData->m_Scale.z*2.0f;
		posList[1].z += pData->m_Up.z*pData->m_Scale.z*2.0f;
		posList[2].z += pData->m_Up.z*pData->m_Scale.z*2.0f;
		posList[3].z += pData->m_Up.z*pData->m_Scale.z*2.0f;

		posList[2].z = (posList[2].z - posList[0].z) * yScale + posList[0].z;
		posList[3].z = (posList[3].z - posList[1].z) * yScale + posList[1].z;

		pDriver->SetTexture(0, m_fxFrames[ m_uCurFrame ].hTex, IDriver3D::FILTER_NORMAL_NO_MIP, false);
		pDriver->RenderWorldQuad( posList, uvList, color, !IsFlagSet(FLAG_EMISSIVE) );

		m_nFrameDelay--;
		if ( m_nFrameDelay <= 0 )
		{
			m_uCurFrame = ( m_uCurFrame + 1 ) % m_fxFrames.size();
			m_nFrameDelay = _FRAME_DELAY;
		}
	}
}
