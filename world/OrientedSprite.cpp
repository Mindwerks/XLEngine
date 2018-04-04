#include "Object.h"
#include "OrientedSprite.h"
#include "../render/IDriver3D.h"

OrientedSprite::OrientedSprite() : RenderComponent()
{
	m_hTex = XL_INVALID_TEXTURE;
	m_aFlip[0] = 0;
	m_aFlip[1] = 0;
	m_aFlip[2] = 0;
	m_fBaseItens = 1.0f;
	m_fAlpha = 1.0f;
}

OrientedSprite::~OrientedSprite()
{
	RenderComponent::~RenderComponent();
}

void OrientedSprite::Render(Object *pObj, IDriver3D *pDriver, f32 fIntensity, const Vector3& vOffset)
{
	pDriver->SetTexture(0, m_hTex, IDriver3D::FILTER_NORMAL_NO_MIP, false);

	ObjectPhysicsData *pData = pObj->GetPhysicsData();
	Vector3 vRight;
	vRight.CrossAndNormalize(pData->m_Dir, pData->m_Up);

	Vector3 A, B;
	A = pData->m_Loc - vRight*pData->m_Scale - pData->m_Up*pData->m_Scale + vOffset;
	B = pData->m_Loc + vRight*pData->m_Scale + pData->m_Up*pData->m_Scale + vOffset;

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
	pDriver->RenderWorldQuad( posList, uvList, color );
}
