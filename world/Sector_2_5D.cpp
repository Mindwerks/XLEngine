#include "Sector_2_5D.h"
#include "ObjectManager.h"
#include "Object.h"
#include "../math/Vector4.h"
#include "../math/Math.h"
#include "../render/IDriver3D.h"
#include "../render/Camera.h"
#include "../render/RenderQue.h"
#include "WorldCell.h"

Vector2 Sector_2_5D::m_nearPlane[2];
bool Sector_2_5D::m_bUpdateVis = true;

#define _MAX_RECURSION_LEVEL 1024
#define _TEST_WALL_RASTERIZER 0
#define _SHOW_1D_DEPTH 0

Sector_2_5D::VisStack Sector_2_5D::m_visStack[ _MAX_RECURSION_LEVEL ];
Vector3 Sector_2_5D::s_vCurrentOffs;
s32 Sector_2_5D::m_visStackCnt;
s32 Sector_2_5D::m_visStackIdx;

Sector_2_5D::VisStack Sector_2_5D::m_visStack_VAdjoin[ _MAX_RECURSION_LEVEL ];
s32 Sector_2_5D::m_visStackCnt_VAdjoin;

bool s_bPassThruAdjoins=false;

#define _VADJOIN_BUFFER_SIZE 32768
static f32 _VAdjoinBuffer[_VADJOIN_BUFFER_SIZE];
static u32 _uAdjoinBufferLoc;
static u32 _uRenderKey=0;
static Vector3 _vCamDir;

Camera Sector_2_5D::m_Camera2D;

Vector2 Nl, Nr, Nz;
Vector2 Dl, Dr, Dz;
Vector2 _fL, _fR;
Vector2 _nearZ;
Vector2 _nearZ_Nrml;
Vector3 _cpos;
Vector2 ssPos[65536];
Sector_2_5D *_pStartSec;

const s32 bwidth = 1024;
const f32 fbwidth = 1024.0f;

f32 fDepth[bwidth];
f32 fDepthPrev[bwidth];
u16 idBuffer[bwidth];
u16 sBuffer[bwidth];

//150 for Blood, 1000 for Outlaws
f32 Sector_2_5D::s_fFogRange = 100.0f;	//150 for Blood, make this game settable.
//136 for Blood, 136*2 for Outlaws
f32 s_fSkyTop =  136.0f;	// 136 for Blood - make adjustable.
//-120 for Blood, -16 Outlaws
f32 s_fSkyBot =  -120.0f;	//-120 for Blood - make adjustable.

struct ObjToRender
{
	Object *pObj;
	Vector3 offset;
	f32 fIntensity;
};

#define MAX_RENDER_OBJECTS 1024
ObjToRender s_Objects[MAX_RENDER_OBJECTS];
u32 s_uObjectCnt = 0;

Sector_2_5D::Sector_2_5D() : Sector()
{
	m_uTypeFlags = SECTOR_TYPE_25D;

	m_uVertexCount = 0;
	m_uWallCount   = 0;
	m_uFlags	   = 0;

	m_uLayer   = 0;
	m_uAmbientFloor = 0;
	m_uAmbientCeil = 0;

	memset(m_szName, 0, 64);
	
	m_pVertexBase=NULL;
	m_pVertexCur =NULL;
	m_ZRangeBase.Set(0,0);
	m_ZRangeCur.Set(0,0);
	m_vAdjOffset[0].Set(0,0,0);
	m_vAdjOffset[1].Set(0,0,0);

	m_Walls = NULL;
	m_vAdjoin[0] = SOLID_WALL;
	m_vAdjoin[1] = SOLID_WALL;

	m_FloorTexScale.Set(1.0f, 1.0f);
	m_CeilTexScale.Set(1.0f, 1.0f);

	m_hFloorTex = XL_INVALID_TEXTURE;
	m_hCeilTex  = XL_INVALID_TEXTURE;

	m_pFunc = NULL;

	m_aLightFX[0] = 0;
	m_aLightFX[1] = 0;
	m_aLightFX[2] = 0;
}

Sector_2_5D::~Sector_2_5D()
{
	SafeDeleteArr_Test( m_pVertexBase );
	SafeDeleteArr_Test( m_pVertexCur );
	SafeDeleteArr_Test( m_Walls );
}

void Sector_2_5D::Render(IDriver3D *pDriver, Camera *pCamera)
{
	pDriver->EnableCulling(true);
	
	const f32 fOO255 = (1.0f/255.0f);
	f32 fI;
	Vector4 color;
	
	fI = fOO255 * (f32)m_uAmbientFloor;
	color.Set(fI, fI, fI, 1.0f);

	//for now just render the walls - brute force.
	for (u32 w=0; w<m_uWallCount; w++)
	{
		//if ( m_Walls[w].m_adjoin[0] != SOLID_WALL && !(m_Walls[w].m_flags&Wall::WALL_FLAGS_SOLIDTEX) )
		//	continue;

		s32 shade = Math::clamp( (s32)m_uAmbientFloor + m_Walls[w].m_lightDelta + m_aLightFX[2], 0, 255 );
		fI = fOO255 * (f32)shade;
		color.Set(fI, fI, fI, 1.0f);

		TextureHandle hTex = m_Walls[w].m_textures[ Wall::WALL_TEX_MID ];
		pDriver->SetTexture(0, hTex);

		Vector3 p0, p1;
		Vector2 uv0, uv1;
		f32 dz = m_ZRangeCur.y - m_ZRangeCur.x;
		p0.Set( m_pVertexCur[m_Walls[w].m_idx[0]].x, m_pVertexCur[m_Walls[w].m_idx[0]].y, m_ZRangeCur.x );
		p1.Set( m_pVertexCur[m_Walls[w].m_idx[1]].x, m_pVertexCur[m_Walls[w].m_idx[1]].y, m_ZRangeCur.y );
		uv0.Set( m_Walls[w].m_texOffset[0].x, m_Walls[w].m_texOffset[0].y );
		uv1.Set( uv0.x+m_Walls[w].m_wallLen*m_Walls[w].m_texScale[0].x, uv0.y+dz*m_Walls[w].m_texScale[0].y );
		pDriver->RenderWorldQuad(p0, p1, uv0, uv1, color);
	}

	pDriver->EnableCulling(false);
}

bool Sector_2_5D::PointInsideSector(f32 x, f32 y)
{
	Vector2 p(x,y);
	s32 c=0;
		
	for (s32 w=0; w<m_uWallCount; w++)
	{
		const Vector2& v0 = m_pVertexCur[ m_Walls[w].m_idx[0] ];
		const Vector2& v1 = m_pVertexCur[ m_Walls[w].m_idx[1] ];

		if ( v0.y <= y )
		{
			if ( v1.y > y && Math::IsLeft( v0, v1, p ) < 0 )
				++c;
		}
		else
		{
			if ( v1.y <= y && Math::IsLeft( v0, v1, p ) > 0 )
				--c;
		}
	}
	return (c!=0) ? true : false;
}

//2.5D sector based visibility system.
void Sector_2_5D::VisStack_Push(Vector2& fL, Vector2& fR, u32 uStartX, u32 uEndX, Sector_2_5D *pNext, bool bUsePortalClip, Vector3& vOffset)
{
	if ( m_visStackCnt < _MAX_RECURSION_LEVEL )
	{
		m_visStack[m_visStackCnt].fL = fL;
		m_visStack[m_visStackCnt].fR = fR;
		m_visStack[m_visStackCnt].uStartX = uStartX;
		m_visStack[m_visStackCnt].uEndX   = uEndX;
		m_visStack[m_visStackCnt].pNext = pNext;
		m_visStack[m_visStackCnt].bUsePortalClip = bUsePortalClip;
		m_visStack[m_visStackCnt].offset = vOffset;
		m_visStackCnt++;
	}
}

Sector_2_5D::VisStack *Sector_2_5D::VisStack_Pop()
{
	VisStack *pVis = &m_visStack[m_visStackIdx];
	m_visStackIdx++;

	return pVis;
}

void Sector_2_5D::VisStack_Clear()
{
	m_visStackCnt = 0;
	m_visStackIdx = 0;
}

bool _bAllowVAdjoin;

void Sector_2_5D::RenderSky(IDriver3D *pDriver, WorldCell *pCell)
{
	f32 dA = MATH_TWO_PI / 16.0f;
	f32 A = 0.0f;
	f32 x0 =  cosf(A);
	f32 y0 = -sinf(A);
	f32 x1, y1;
	A += dA;

	Vector3 posList[4];
	Vector2 uvList[4];

	u32 uSkyTexCnt = pCell->GetSkyTexCount();

	uvList[0].Set( 0.01f, 0.01f );
	uvList[1].Set( 0.99f, 0.01f );
	uvList[2].Set( 0.99f, 0.99f );
	uvList[3].Set( 0.01f, 0.99f );

	const Vector3& vLoc = m_Camera2D.GetLoc();
	f32 zTop = vLoc.z + s_fSkyTop;
	f32 zBot = vLoc.z + s_fSkyBot;

	pDriver->EnableDepthRead(false);
	pDriver->EnableDepthWrite(false);
	pDriver->EnableStencilTesting(true);
	u32 uDiv = 16/uSkyTexCnt;
	f32 fWidth = 1.0f/(f32)uDiv;
	for (u32 s=0; s<16; s++)
	{
		if ( uSkyTexCnt < 16 )
		{
			u32 idx = s%uDiv;
			f32 fStart = (f32)idx * fWidth;

			uvList[0].x = fStart;
			uvList[1].x = fStart + fWidth;
			uvList[2].x = fStart + fWidth;
			uvList[3].x = fStart;

			if ( uvList[0].x < 0.5f/512.0f ) uvList[0].x = 0.5f/512.0f;
			if ( uvList[1].x > 1.0f-0.5f/512.0f ) uvList[1].x = 1.0f-0.5f/512.0f;
			if ( uvList[2].x > 1.0f-0.5f/512.0f ) uvList[2].x = 1.0f-0.5f/512.0f;
			if ( uvList[3].x < 0.5f/512.0f ) uvList[3].x = 0.5f/512.0f;
		}

		x1 =  cosf(A);
		y1 = -sinf(A);

		posList[0].Set( vLoc.x+x0*100.0f, vLoc.y+y0*100.0f, zTop );
		posList[1].Set( vLoc.x+x1*100.0f, vLoc.y+y1*100.0f, zTop );
		posList[2].Set( vLoc.x+x1*100.0f, vLoc.y+y1*100.0f, zBot );
		posList[3].Set( vLoc.x+x0*100.0f, vLoc.y+y0*100.0f, zBot );

		TextureHandle hTex = pCell->GetSkyTex(s%uSkyTexCnt);
		pDriver->SetTexture(0, hTex, IDriver3D::FILTER_NORMAL_NO_MIP, false);
		A += dA;

		x0 = x1;
		y0 = y1;

		pDriver->RenderWorldQuad(posList, uvList, Vector4::One);
	}
	pDriver->EnableDepthRead(true);
	pDriver->EnableDepthWrite(true);
	pDriver->EnableStencilTesting(false);
}

void Sector_2_5D::RenderSectors(IDriver3D *pDriver, WorldCell *pCell, Camera *pCamera, Sector_2_5D *pStart, const vector<Sector *>& Sectors)
{
	//1. Compute world space 2D edges.
	const Vector3& vDir3D = pCamera->GetDir();
	Vector3 vLoc   = pCamera->GetLoc();
	f32 fNearZ = 1.0f;//pCamera->GetNearZ();
	Vector3 vDir(vDir3D.x, vDir3D.y, 0.0f);
	vDir.Normalize();

	_vCamDir = vDir;

	Vector3 R, U(0.0f, 0.0f, 1.0f);
	R.CrossAndNormalize(vDir, U);

	Vector2 A, B;
	f32 x = pCamera->GetFrustumWidth()*10.0f;
	A.Set( vLoc.x+vDir.x*fNearZ+R.x*x, vLoc.y+vDir.y*fNearZ+R.y*x );
	B.Set( vLoc.x+vDir.x*fNearZ-R.x*x, vLoc.y+vDir.y*fNearZ-R.y*x );
	
	m_Camera2D = *pCamera;
	m_Camera2D.SetSkew(0.0f);
	m_Camera2D.SetDir( vDir );
	m_Camera2D.Compute();

	s_vCurrentOffs.Set(0.0f, 0.0f, 0.0f);

	m_visStackCnt_VAdjoin = 0;
	_uAdjoinBufferLoc = 0;
	s_uObjectCnt = 0;

	u32 maxDrawCnt = _MAX_RECURSION_LEVEL;
	if ( s_MaxSecDrawCnt > 0 )
	{
		maxDrawCnt = MIN(s_MaxSecDrawCnt, maxDrawCnt);
	}
	u32 recursion = 0;

	if ( pStart->m_vAdjoin[1] != 0xffff && vLoc.z > pStart->GetZ_Ceil(vLoc.x+pStart->m_vAdjOffset[1].x, vLoc.y+pStart->m_vAdjOffset[1].y, Sectors) )
	{
		Sector_2_5D *pNewSec = (Sector_2_5D *)Sectors[ pStart->m_vAdjoin[1] ];

		vLoc = vLoc + pStart->m_vAdjOffset[1];
		A.Set( A.x + pStart->m_vAdjOffset[1].x, A.y + pStart->m_vAdjOffset[1].y );
		B.Set( B.x + pStart->m_vAdjOffset[1].x, B.y + pStart->m_vAdjOffset[1].y );
		pStart = pNewSec;

		m_Camera2D.SetLoc( vLoc );
		m_Camera2D.Compute();
	}
	_pStartSec = pStart;
	_SetupCameraParameters(vLoc, vDir, A, B);

	_bAllowVAdjoin = true;

	for (s32 x=0; x<bwidth; x++)
	{
		fDepth[x] = 0.0f;
		fDepthPrev[x] = 10000000.0f;
		idBuffer[x] = 0;
		sBuffer[x] = 0xffff;
	}

#if _TEST_WALL_RASTERIZER
	if ( m_bUpdateVis )
	{
		VisStack_Clear();
		VisStack_Push(A, B, 0, bwidth-1, pStart);
		do
		{
			VisStack *pVis = VisStack_Pop();
			s_vCurrentOffs = pVis->offset;

			WallRasterizer(vLoc, pVis->uStartX, pVis->uEndX, pVis->pNext, Sectors);
			recursion++;

			if ( recursion >= maxDrawCnt )
			{
				//assert(0);
				break;
			}
		} while (m_visStackIdx < m_visStackCnt);
	}
	s_vCurrentOffs.Set(0.0f, 0.0f, 0.0f);
#else
	if ( m_bUpdateVis )
	{
		VisStack_Clear();
		VisStack_Push(A, B, 0, bwidth-1, pStart);
		do
		{
			VisStack *pVis = VisStack_Pop();
			s_vCurrentOffs = pVis->offset;
			Visibility2D(vLoc, pVis->fL, pVis->fR, pVis->uStartX, pVis->uEndX, pVis->pNext, Sectors, pVis->bUsePortalClip, pDriver);
			recursion++;

			if ( recursion >= maxDrawCnt )
			{
				//assert(0);
				break;
			}
		} while (m_visStackIdx < m_visStackCnt);

		if ( m_visStackCnt_VAdjoin )
		{
			_bAllowVAdjoin = false;
			for (u32 v=0; v<(u32)m_visStackCnt_VAdjoin; v++)
			{
				VisStack *pVAdjoin = &m_visStack_VAdjoin[v];

				if ( pVAdjoin->depth == NULL )
					continue;

				for (u32 x=pVAdjoin->uStartX; x<=pVAdjoin->uEndX; x++)
				{
					fDepth[x] = pVAdjoin->depth[x-pVAdjoin->uStartX];
				}
				recursion = 0;
				VisStack_Clear();
				VisStack_Push(pVAdjoin->fL, pVAdjoin->fR, pVAdjoin->uStartX, pVAdjoin->uEndX, pVAdjoin->pNext, true, pVAdjoin->offset);
				if ( pStart->m_vAdjoin[0] != 0xffff || pStart->m_vAdjoin[1] != 0xffff )
				{
					_pStartSec = pVAdjoin->pNext;
					for (u32 x=pVAdjoin->uStartX; x<=pVAdjoin->uEndX; x++)
					{
						fDepthPrev[x] = 1000000.0f;
					}
				}
				do
				{
					VisStack *pVis = VisStack_Pop();
					s_vCurrentOffs = pVis->offset;
					Visibility2D(vLoc, pVis->fL, pVis->fR, pVis->uStartX, pVis->uEndX, pVis->pNext, Sectors, pVis->bUsePortalClip, pDriver);
					recursion++;

					if ( recursion >= maxDrawCnt )
					{
						//assert(0);
						break;
					}
				} while (m_visStackIdx < m_visStackCnt);
			}
		}
	}
	s_vCurrentOffs.Set(0.0f, 0.0f, 0.0f);

	pDriver->EnableFog(true, s_fFogRange);
	RenderQue::Render();

	pDriver->EnableFog(false);
	RenderSky(pDriver, pCell);
	pDriver->EnableFog(true, s_fFogRange);

	RenderObjects( pDriver );
	pDriver->EnableFog(false);
#endif
	_uRenderKey++;

	//fill in the player brightness
	Object *player = ObjectManager::FindObject("PLAYER");
	if ( player )
	{
		const f32 fOO255 = (1.0f/255.0f);
		s32 shade = Math::clamp( (s32)pStart->m_uAmbientFloor + pStart->m_aLightFX[0], 0, 255 );
		f32 fI = fOO255 * (f32)shade;

		player->SetBrightness( fI );
	}

	//setup orthographic view, used for things like UI.
#if _SHOW_1D_DEPTH
	Matrix projMtx;
	projMtx.ProjOrtho((f32)1024, (f32)768);

	pDriver->SetProjMtx( &projMtx );
	pDriver->SetViewMatrix( &Matrix::s_Identity, &Vector3(0,0,0), &Vector3(0,0,1) );
	pDriver->SetWorldMatrix( &Matrix::s_Identity, 0, 0 );

	pDriver->EnableDepthWrite(false);
	pDriver->EnableDepthRead(false);
	pDriver->EnableCulling(false);
	pDriver->SetTexture(0, XL_INVALID_TEXTURE);
	u32 uDiv = bwidth/1024;
	//test
	s32 nInvalidPixelCount = 0;
	for (u32 x=1; x<bwidth-1; x+=uDiv)
	{
		if ( fDepth[x] <= 0.0f )
		{
			nInvalidPixelCount++;
		}
	}
	assert( nInvalidPixelCount == 0 );
	//
	for (u32 x=0; x<bwidth; x+=uDiv)
	{
		//ok extract the texture ID...
		u32 uTexID = 0;
		f32 fTex = 1.0f;
		if ( fDepth[x] != 0.0f )
		{
			uTexID = ((Sector_2_5D *)Sectors[ sBuffer[x] ])->m_Walls[ idBuffer[x] ].m_textures[ Wall::WALL_TEX_MID ]&15;
			fTex = 0.75f*(f32)uTexID/15.0f + 0.25f;
		}

		Vector4 posScale((f32)(x/uDiv), 0, (f32)uDiv, 768.0f);//384.0f);
		Vector2 uv(0,0);
		f32 fID = 0.25f + 0.75f*(f32)(idBuffer[x]&31) / 31.0f;
		f32 fd = sqrtf(fDepth[x]);
		Vector4 color(fd, fd, fd*fTex, 1.0f);
		pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
	}
	pDriver->EnableDepthWrite(true);
	pDriver->EnableDepthRead(true);
#endif
}

s32 _nCollideSecCnt;
s32 _anCollideSectors[256];

f32 m_fCollideHeight = 6.8f;
f32 m_fCollideHeight_DH = 4.5f;
f32 m_fStepSize = 4.5f;	//should be 3.5
f32 m_fJumpStepSize = 3.5f;

bool _AddToCollideList(s32 s)
{
	bool bInList = false;
	if ( s > -1 )
	{
		for (s32 i=0; i<_nCollideSecCnt; i++)
		{
			if ( _anCollideSectors[i] == s )
			{
				bInList = true;
				break;
			}
		}
		if ( bInList == false )
		{
			_anCollideSectors[ _nCollideSecCnt++ ] = s;
		}
	}
	else
	{
		bInList = true;
	}
	return bInList;
}

void Sector_2_5D::_AddSectorToList(s32 s, Vector3 *p0, Vector2& vPathMin, Vector2& vPathMax, const vector<Sector *>& Sectors)
{
	//now go through and add any portals that overlap the path...
	if ( _AddToCollideList(s) == true )
		return;

	Sector_2_5D *pSec = (Sector_2_5D *)Sectors[s];
	f32 col_z0 = pSec->GetZ_Floor(p0->x, p0->y, Sectors), col_z1;
	s32 w;

	for (w=0; w<pSec->m_uWallCount; w++)
	{
		//The Edge
		const Vector2& v0 = pSec->m_pVertexCur[ pSec->m_Walls[w].m_idx[0] ];
		const Vector2& v1 = pSec->m_pVertexCur[ pSec->m_Walls[w].m_idx[1] ];

		if ( Math::EdgeIntersectsBox2D(vPathMin, vPathMax, v0, v1) == false )
			continue;

		//Is this wall solid?
		bool bSolid = true;
		u16 uNextSec = pSec->m_Walls[w].m_adjoin[0];
		if ( uNextSec != 0xffff )
		{
			if ( pSec->m_Walls[w].m_adjoin[1] != 0xffff )
			{
				//dual adjoins, for now pick the closest one in Z.
				Sector_2_5D *pNext0 = (Sector_2_5D *)Sectors[ pSec->m_Walls[w].m_adjoin[0] ];
				Sector_2_5D *pNext1 = (Sector_2_5D *)Sectors[ pSec->m_Walls[w].m_adjoin[1] ];

				//which is lower?
				Sector_2_5D *pLowest, *pHighest;
				u16 uLowIdx, uHighIdx;
				if ( pNext0->m_ZRangeCur.y < pNext1->m_ZRangeCur.y )
				{
					pLowest  = pNext0;
					pHighest = pNext1;
					uLowIdx  = 0;
					uHighIdx = 1;
				}
				else
				{
					pHighest = pNext0;
					pLowest  = pNext1;
					uHighIdx = 0;
					uLowIdx  = 1;
				}

				//pick the lower adjoin?
				if ( p0->z+4.0f < pLowest->m_ZRangeCur.y )
				{
					uNextSec = pSec->m_Walls[w].m_adjoin[uLowIdx];
				}
				else
				{
					uNextSec = pSec->m_Walls[w].m_adjoin[uHighIdx];
				}
			}

			if ( s_bPassThruAdjoins )
			{
				bSolid = false;
			}
			else
			{
				Sector_2_5D *pNext = (Sector_2_5D *)Sectors[ uNextSec ];
				col_z1 = pNext->m_ZRangeCur.x;

				f32 z1 = pNext->GetZ_Ceil(p0->x, p0->y, Sectors);
				if ( (p0->z+m_fCollideHeight < z1) && (z1-col_z1 > m_fCollideHeight_DH) )
				{
					if ( col_z0 >= col_z1 - m_fStepSize || p0->z >= col_z1 - m_fJumpStepSize )
					{
						bSolid = false;
					}
				}
			}
		}
		if ( bSolid == true )
			continue;

		_AddSectorToList( uNextSec, p0, vPathMin, vPathMax, Sectors );
	}
}

bool _PushOutPositionList(Vector3 *p0, f32 fRadius, const vector<Sector *>& Sectors)
{
	bool bNeedPush = false;

	Vector2 vMinPath, vMaxPath;
	vMinPath.x = p0->x - fRadius;
	vMinPath.y = p0->y - fRadius;
	vMaxPath.x = p0->x + fRadius;
	vMaxPath.y = p0->y + fRadius;

	for (s32 p=0; p<16; p++)
	{
		bool bPush = false;
		for (s32 sec=0; sec<_nCollideSecCnt; sec++)
		{
			s32 s = _anCollideSectors[sec], w;
			Sector_2_5D *pSec = (Sector_2_5D *)Sectors[s];
			f32 col_z0 = pSec->m_ZRangeCur.x, col_z1;

			f32 r2 = fRadius*fRadius;
			for (w=0; w< pSec->m_uWallCount; w++)
			{
				//The Edge
				const Vector2& v0 = pSec->m_pVertexCur[ pSec->m_Walls[w].m_idx[0] ];
				const Vector2& v1 = pSec->m_pVertexCur[ pSec->m_Walls[w].m_idx[1] ];

				if ( Math::EdgeIntersectsBox2D(vMinPath, vMaxPath, v0, v1) == false )
					continue;

				//Is this wall solid?
				bool bSolid = true;
				if ( pSec->m_Walls[w].m_adjoin[0] != 0xffff )
				{	
					if ( s_bPassThruAdjoins )
					{
						bSolid = false;
					}
					else
					{
						Sector_2_5D *pNext = (Sector_2_5D *)Sectors[pSec->m_Walls[w].m_adjoin[0]];
						col_z1 = pNext->GetZ_Floor(p0->x, p0->y, Sectors);

						f32 z1 = pNext->GetZ_Ceil(p0->x, p0->y, Sectors);
						if ( (p0->z+m_fCollideHeight <= z1) && (z1-col_z1 > m_fCollideHeight_DH) )
						{
							if ( col_z0 >= col_z1 - m_fStepSize || p0->z >= col_z1 - m_fJumpStepSize )
							{
								bSolid = false;
							}
						}
					}
				}
				if ( bSolid == false )
					continue;

				//now find the closest point on the line.
				f32 d2 = (v1.x-v0.x)*(v1.x-v0.x) + (v1.y-v0.y)*(v1.y-v0.y);
				f32 ix, iy, lu;
				if ( d2 <= 0.00000001f ) { continue; }
				
				f32 ood = 1.0f/sqrtf(d2);
				lu = ( (p0->x - v0.x)*(v1.x-v0.x) + (p0->y-v0.y)*(v1.y-v0.y) ) / d2;
				lu = min( max(lu, 0.0f), 1.0f );
				ix = v0.x + lu*(v1.x-v0.x);
				iy = v0.y + lu*(v1.y-v0.y);

				f32 dx, dy;
				dx = ix - p0->x; dy = iy - p0->y;
				d2 = dx*dx + dy*dy;
				if ( d2 < r2 )
				{
					//now we need to push out of the wall...
					f32 d = sqrtf(d2);
					if (d > 0.000001f )
					{
						f32 offs = (fRadius - d)+0.00001f;

						//compute wall normal...
						Vector2 wD(v1.x-v0.x, v1.y-v0.y), wN;
						wN.x = wD.y; wN.y = -wD.x;	//normal is flipped due to winding. N = -perp. perp = (-dy,dx), so N = (dy,-dx)
						wN.Normalize();

						p0->x += wN.x*offs;
						p0->y += wN.y*offs;

						vMinPath.x = p0->x - fRadius;
						vMinPath.y = p0->y - fRadius;
						vMaxPath.x = p0->x + fRadius;
						vMaxPath.y = p0->y + fRadius;

						bNeedPush = true;
						bPush = true;
					}
				}
			}
		}
		if ( bNeedPush == false || bPush == false )
			break;
	}
	return bNeedPush;
}

void Sector_2_5D::RayCastAndActivate(Vector3 *p0, Vector3 *p1, u32& uSector, const vector<Sector *>& Sectors)
{
	bool bHit = false;
	u32 s = uSector;
	Sector_2_5D *pSec = (Sector_2_5D *)Sectors[s];
	s32 nClosestWall=-1;	//0xfffe = floor, 0xffff = ceiling.

	Vector3 N;
	N.Set(p1->x-p0->x, p1->y-p0->y, p1->z-p0->z);
	f32 ld = N.Length(), u, min_u_obj=10.0f, min_u_sec=10.0f;
	if ( ld > 0.000001f ) { ld = 1.0f / ld; }

	//collide with walls/floor in this sector
	//check floor
	if ( p0->z > pSec->m_ZRangeCur.x && p1->z <= pSec->m_ZRangeCur.x )
	{
		u = ( pSec->m_ZRangeCur.x - p0->z ) / ( p1->z - p0->z );
		if ( u >= 0.0f && u <= 1.0f && u < min_u_sec )
		{
			min_u_sec = u;
			nClosestWall = 0xfffe;
		}
	}
	//check ceiling
	if ( p0->z < pSec->m_ZRangeCur.y && p1->z >= pSec->m_ZRangeCur.y )
	{
		u = ( pSec->m_ZRangeCur.y - p0->z ) / ( p1->z - p0->z );
		if ( u >= 0.0f && u <= 1.0f && u < min_u_sec )
		{
			min_u_sec = u;
			nClosestWall = 0xffff;
		}
	}

	f32 x0, y0, x1, y1;
	f32 x2, y2, x3, y3;

	//check walls
	x2 = p0->x; x3 = p1->x;
	y2 = p0->y; y3 = p1->y;
	f32 z=0.0f;

	for (u32 w=0; w<pSec->m_uWallCount; w++)
	{
		u16 i0 = pSec->m_Walls[w].m_idx[0];
		u16 i1 = pSec->m_Walls[w].m_idx[1];
		x0 = pSec->m_pVertexCur[i0].x; x1 = pSec->m_pVertexCur[i1].x;
		y0 = pSec->m_pVertexCur[i0].y; y1 = pSec->m_pVertexCur[i1].y;

		//find ray, line intersect.
		f32 denom = (y3-y2)*(x1-x0) - (x3-x2)*(y1-y0);
		f32 u, v;
		if ( denom > 0.00001f )
		{
			denom = 1.0f / denom;
			u = ( (x3-x2)*(y0-y2) - (y3-y2)*(x0-x2) ) * denom;
			v = ( (x1-x0)*(y0-y2) - (y1-y0)*(x0-x2) ) * denom;

			//are we intersecting with this wall?
			if ( u > -0.001f && u < 1.001f && v > 0.0f && v <= 1.0f )
			{
				if ( min_u_sec > v )
				{
					min_u_sec = v;
					nClosestWall = (s32)w;
				}
			}
		}
	}

	//take closest hit, if that is an adjoin - now move into the next sector.
	if ( nClosestWall>-1 )
	{
		p0->x = p0->x + min_u_sec*(p1->x - p0->x);
		p0->y = p0->y + min_u_sec*(p1->y - p0->y);
		p0->z = p0->z + min_u_sec*(p1->z - p0->z);
		LevelFunc *pWallFunc = NULL;
		if ( nClosestWall < 0xfffe )
		{
			//Is this an adjoin?
			bool bSolid = pSec->m_Walls[nClosestWall].m_adjoin[0] == 0xffff;

			pWallFunc = pSec->m_Walls[nClosestWall].m_pFunc;
			if (!bSolid)
			{
				Sector_2_5D *pNext = (Sector_2_5D *)Sectors[ pSec->m_Walls[nClosestWall].m_adjoin[0] ];
				//ok is the intersection in range of the opening...
				f32 cz0 = pNext->m_ZRangeCur.x;
				f32 cz1 = pNext->m_ZRangeCur.y;

				if ( (pSec->m_uFlags&SEC_FLAGS_ALLOW_NONSOLID_ACTIVATE) || (pNext->m_uFlags&SEC_FLAGS_ALLOW_NONSOLID_ACTIVATE) )
				{
					if ( pWallFunc == NULL )
					{
						//we hit the wall in the next sector...
						pWallFunc = pNext->m_Walls[ pSec->m_Walls[nClosestWall].m_mirror[0] ].m_pFunc;
					}
					if ( pWallFunc )
					{
						pWallFunc->Activate(0, 0);
					}
				}

				if ( p0->z > cz0 && p0->z < cz1 )
				{
					uSector = pSec->m_Walls[nClosestWall].m_adjoin[0];
					RayCastAndActivate(p0, p1, uSector, Sectors);
					return;
				}
				else if ( pWallFunc == NULL )
				{
					//we hit the wall in the next sector...
					pWallFunc = pNext->m_Walls[ pSec->m_Walls[nClosestWall].m_mirror[0] ].m_pFunc;
				}
			}
		}

		//ok we hit a wall.
		if ( pWallFunc )
		{
			pWallFunc->Activate(0, 0);
		}
	}
}

void Sector_2_5D::Collide(Vector3 *p0, Vector3 *p1, u32& uSector, f32 fRadius, const vector<Sector *>& Sectors, bool bPassThruAdjoins)
{
	s_bPassThruAdjoins = bPassThruAdjoins;
	//start from p0, move to p1.
	//since we simulate at 60fps there's no point in doing continuous collision.
	u32 startSec = uSector;
	u32 s = uSector;
	f32 r2 = fRadius*fRadius;

	//Which adjoins do we intersect?
	Vector2 vMinPath, vMaxPath;
	vMinPath.x = MIN(p0->x, p1->x) - fRadius - 1.0f;
	vMinPath.y = MIN(p0->y, p1->y) - fRadius - 1.0f;
	vMaxPath.x = MAX(p0->x, p1->x) + fRadius + 1.0f;
	vMaxPath.y = MAX(p0->y, p1->y) + fRadius + 1.0f;

	_nCollideSecCnt = 0;
	_AddSectorToList(s, p1, vMinPath, vMaxPath, Sectors);
	assert( _anCollideSectors[0] == s );

	//if we're still inside, just push away from the walls.
	Vector3 iPos = *p1;
	//now which sector are we in now?
	bool bSectorFound = false;
	_PushOutPositionList(&iPos, fRadius, Sectors);

	for (s32 i=0; i<_nCollideSecCnt; i++)
	{
		s = _anCollideSectors[i];
		Sector_2_5D *pSec = (Sector_2_5D *)Sectors[s];

		if ( pSec->PointInsideSector(iPos.x, iPos.y) )
		{
			uSector = s;
			bSectorFound = true;
			break;
		}
	}

	if ( bSectorFound )
	{
		*p0 = iPos;
	}

	Sector_2_5D *pSec = (Sector_2_5D *)Sectors[uSector];
	//p0->z = MAX( p0->z, pSec->m_ZRangeCur.x );
	p0->z = pSec->GetZ_Floor(p0->x, p0->y, Sectors);

	//vertical adjoins...?
	if ( pSec->m_vAdjoin[0] != 0xffff )
	{
		Sector_2_5D *pNext = (Sector_2_5D *)Sectors[ pSec->m_vAdjoin[0] ];
		if ( pNext && !(pNext->m_uFlags&SEC_FLAGS_UNDERWATER) )
		{
			p0->x -= pSec->m_vAdjOffset[0].x;
			p0->y -= pSec->m_vAdjOffset[0].y;
			p0->z = pNext->GetZ_Floor(p0->x, p0->y, Sectors);

			uSector = pSec->m_vAdjoin[0];
		}
	}
}

f32 Sector_2_5D::GetZ_Floor(f32 x, f32 y, const vector<Sector *>& Sectors)
{
	f32 z = m_ZRangeCur.x + s_vCurrentOffs.z;
	if ( (m_uFlags&SEC_FLAGS_FLOOR_SLOPE) && m_auSlopeSector[0] < 0xffff )
	{
		const Sector_2_5D *pAnchor = (const Sector_2_5D *)Sectors[m_auSlopeSector[0]];

		Vector2 p(x-s_vCurrentOffs.x, y-s_vCurrentOffs.y);
		Vector2 D = pAnchor->m_pVertexCur[ pAnchor->m_Walls[m_auSlopeAnchor[0]].m_idx[1] ] - pAnchor->m_pVertexCur[ pAnchor->m_Walls[m_auSlopeAnchor[0]].m_idx[0] ];
		Vector2 N(-D.y, D.x);
		N.Normalize();

		Vector2 L = p - pAnchor->m_pVertexCur[ pAnchor->m_Walls[m_auSlopeAnchor[0]].m_idx[0] ];
		f32 m = -L.Dot(N);

		z = z + m*m_fFloorSlope;
	}
	return z;
}

f32 Sector_2_5D::GetZ_Ceil(f32 x, f32 y, const vector<Sector *>& Sectors)
{
	f32 z = m_ZRangeCur.y + s_vCurrentOffs.z;
	if ( (m_uFlags&SEC_FLAGS_CEIL_SLOPE) && m_auSlopeSector[1] < 0xffff )
	{
		const Sector_2_5D *pAnchor = (const Sector_2_5D *)Sectors[m_auSlopeSector[1]];

		Vector2 p(x-s_vCurrentOffs.x, y-s_vCurrentOffs.y);
		Vector2 D = pAnchor->m_pVertexCur[ pAnchor->m_Walls[m_auSlopeAnchor[1]].m_idx[1] ] - pAnchor->m_pVertexCur[ pAnchor->m_Walls[m_auSlopeAnchor[1]].m_idx[0] ];
		Vector2 N(-D.y, D.x);
		N.Normalize();

		Vector2 L = p - pAnchor->m_pVertexCur[ pAnchor->m_Walls[m_auSlopeAnchor[1]].m_idx[0] ];
		f32 m = -L.Dot(N);

		z = z + m*m_fCeilSlope;
	}
	return z;
}

void Sector_2_5D::AddObjectToRender(Object *pObj, f32 fIntensity, const Vector3& vOffs)
{
	if ( s_uObjectCnt < MAX_RENDER_OBJECTS )
	{
		s_Objects[s_uObjectCnt].pObj = pObj;
		s_Objects[s_uObjectCnt].offset = vOffs;
		s_Objects[s_uObjectCnt].fIntensity = fIntensity;

		//This is used by other systems so they know, in general, how bright an object
		//is without having to understand how to render it.
		pObj->SetBrightness( fIntensity );

		s_uObjectCnt++;
	}
}

void Sector_2_5D::RenderObjects(IDriver3D *pDriver)
{
	pDriver->EnableAlphaTest(true, 32);
	pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );

	for (u32 o=0; o<s_uObjectCnt; o++)
	{
		s_Objects[o].pObj->Render( pDriver, s_Objects[o].fIntensity, s_Objects[o].offset );
	}
	s_uObjectCnt = 0;

	pDriver->EnableAlphaTest(false);
	pDriver->SetBlendMode();
}

void Sector_2_5D::_DrawFloor(IDriver3D *pDriver, Sector_2_5D *pCurSec, const Vector2 *worldPos, const Vector2& a, const Vector2& n0, const Vector2& n1, const vector<Sector *>& Sectors)
{
	const f32 fOO255 = (1.0f/255.0f);
	f32 fI;
	Vector4 color;

	s32 shade = Math::clamp( (s32)pCurSec->m_uAmbientFloor + pCurSec->m_aLightFX[0], 0, 255 );
	fI = fOO255 * (f32)shade;
	color.Set(fI, fI, fI, 1.0f);

	f32 s, t;
	Math::LineIntersect2D(n0, n1, a, worldPos[0], s, t);
	Vector2 va = n0 + (n1-n0)*s;
	Math::LineIntersect2D(n0, n1, a, worldPos[1], s, t);
	Vector2 vb = n0 + (n1-n0)*s;

	Vector3 posList[4];
	Vector2 uvList[4];
	posList[0].Set(va.x, va.y, pCurSec->GetZ_Floor(va.x, va.y, Sectors));
	posList[1].Set(worldPos[0].x, worldPos[0].y, pCurSec->GetZ_Floor(worldPos[0].x, worldPos[0].y, Sectors));
	posList[2].Set(worldPos[1].x, worldPos[1].y, pCurSec->GetZ_Floor(worldPos[1].x, worldPos[1].y, Sectors));
	posList[3].Set(vb.x, vb.y, pCurSec->GetZ_Floor(vb.x, vb.y, Sectors));

	if ( pCurSec->m_vAdjoin[0] == 0xffff || pCurSec->m_uFlags&Sector_2_5D::SEC_FLAGS_FLOORWATER )
	{
		if ( pCurSec->m_uFlags&Sector_2_5D::SEC_FLAGS_FLOOR_FLIP )
		{
			uvList[0].Set( posList[0].y*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[0].x*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
			uvList[1].Set( posList[1].y*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[1].x*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
			uvList[2].Set( posList[2].y*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[2].x*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
			uvList[3].Set( posList[3].y*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[3].x*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
		}
		else
		{
			uvList[0].Set( posList[0].x*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[0].y*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
			uvList[1].Set( posList[1].x*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[1].y*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
			uvList[2].Set( posList[2].x*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[2].y*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
			uvList[3].Set( posList[3].x*pCurSec->m_FloorTexScale.x + pCurSec->m_texOffset[0].x, posList[3].y*pCurSec->m_FloorTexScale.y + pCurSec->m_texOffset[0].y );
		}

		bool bForceZWrite = false;
		if ( pCurSec->m_uFlags&Sector_2_5D::SEC_FLAGS_SKYFLOOR )
		{
			bForceZWrite = true;
			color.w = 0.0f;
		}

		if ( !(pCurSec->m_uFlags&Sector_2_5D::SEC_FLAGS_FLOORWATER) )
		{
			RenderQue::AddQuad(pCurSec->m_hFloorTex, posList, uvList, color, bForceZWrite);
		}
		else
		{
			color.w = 0.75f;
			RenderQue::AddQuad(pCurSec->m_hFloorTex, posList, uvList, color, bForceZWrite);
		}
	}

	if ( pCurSec->m_vAdjoin[1] == 0xffff )
	{
		posList[0].z = pCurSec->GetZ_Ceil(va.x, va.y, Sectors);
		posList[1].z = pCurSec->GetZ_Ceil(worldPos[0].x, worldPos[0].y, Sectors);
		posList[2].z = pCurSec->GetZ_Ceil(worldPos[1].x, worldPos[1].y, Sectors);
		posList[3].z = pCurSec->GetZ_Ceil(vb.x, vb.y, Sectors);

		shade = Math::clamp( (s32)pCurSec->m_uAmbientCeil + pCurSec->m_aLightFX[1], 0, 255 );
		fI = fOO255 * (f32)shade;
		color.Set(fI, fI, fI, 1.0f);

		if ( pCurSec->m_uFlags&Sector_2_5D::SEC_FLAGS_CEIL_FLIP )
		{
			uvList[0].Set( posList[0].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y, posList[0].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x );
			uvList[1].Set( posList[1].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y, posList[1].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x );
			uvList[2].Set( posList[2].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y, posList[2].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x );
			uvList[3].Set( posList[3].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y, posList[3].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x );
		}
		else
		{
			uvList[0].Set( posList[0].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x, posList[0].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y );
			uvList[1].Set( posList[1].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x, posList[1].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y );
			uvList[2].Set( posList[2].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x, posList[2].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y );
			uvList[3].Set( posList[3].x*pCurSec->m_CeilTexScale.x + pCurSec->m_texOffset[1].x, posList[3].y*pCurSec->m_CeilTexScale.y + pCurSec->m_texOffset[1].y );
		}

		bool bForceZWrite = false;
		if ( pCurSec->m_uFlags&Sector_2_5D::SEC_FLAGS_EXTERIOR )
		{
			bForceZWrite = true;
			color.w = 0.0f;
		}

		RenderQue::AddQuad(pCurSec->m_hCeilTex, posList, uvList, color, bForceZWrite);
	}
}

void Sector_2_5D::_DrawWall(IDriver3D *pDriver, Sector_2_5D *pCurSec, Sector_2_5D *pNextSec, Sector_2_5D *pBotSec, Sector_2_5D *pTopSec, u16 w, Vector2 *worldPos, const vector<Sector *>& Sectors)
{
	const f32 fOO255 = (1.0f/255.0f);
	f32 fI;
	Vector4 color;

	//later get the delta to work.
	s32 shade = Math::clamp( pCurSec->m_Walls[w].m_lightDelta + pCurSec->m_aLightFX[2], 0, 255 );
	fI = fOO255 * (f32)shade;
	color.Set(fI, fI, fI, 1.0f);

	TextureHandle hTex = pCurSec->m_Walls[w].m_textures[ Wall::WALL_TEX_MID ];

	Vector3 p0, p1;
	Vector2 uv0, uv1;
	f32 dz = pCurSec->m_ZRangeCur.y - pCurSec->m_ZRangeCur.x;

	u16 i0 = pCurSec->m_Walls[w].m_idx[0], i1 = pCurSec->m_Walls[w].m_idx[1];
	
	if ( pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_XFLIP )
	{
		uv0.x = pCurSec->m_Walls[w].m_texOffset[0].x + pCurSec->m_Walls[w].m_wallLen*pCurSec->m_Walls[w].m_texScale[0].x;
		uv1.x = pCurSec->m_Walls[w].m_texOffset[0].x;
	}
	else
	{
		uv0.x = pCurSec->m_Walls[w].m_texOffset[0].x;
		uv1.x = pCurSec->m_Walls[w].m_texOffset[0].x + pCurSec->m_Walls[w].m_wallLen*pCurSec->m_Walls[w].m_texScale[0].x;
	}

	if ( pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_YFLIP )
	{
		uv0.y = pCurSec->m_Walls[w].m_texOffset[0].y;
		uv1.y = uv0.y+dz*pCurSec->m_Walls[w].m_texScale[0].y;
	}
	else
	{
		uv1.y = pCurSec->m_Walls[w].m_texOffset[0].y;
		uv0.y = uv1.y+dz*pCurSec->m_Walls[w].m_texScale[0].y;
	}
	
	//now clip the u direction...
	f32 x0 = pCurSec->m_pVertexCur[ pCurSec->m_Walls[w].m_idx[0] ].x;
	f32 x1 = pCurSec->m_pVertexCur[ pCurSec->m_Walls[w].m_idx[1] ].x;
	f32 y0 = pCurSec->m_pVertexCur[ pCurSec->m_Walls[w].m_idx[0] ].y;
	f32 y1 = pCurSec->m_pVertexCur[ pCurSec->m_Walls[w].m_idx[1] ].y;

	f32 dx = Math::abs( x1 - x0 );
	f32 dy = Math::abs( y1 - y0 );
	f32 t0, t1;
	if ( dx >= dy )
	{
		t0 = (worldPos[0].x - x0) / (x1 - x0);
		t1 = (worldPos[1].x - x0) / (x1 - x0);
	}
	else
	{
		t0 = (worldPos[0].y - y0) / (y1 - y0);
		t1 = (worldPos[1].y - y0) / (y1 - y0);
	}
	f32 u0 = uv0.x * (1.0f - t0) + uv1.x * t0;
	f32 u1 = uv0.x * (1.0f - t1) + uv1.x * t1;
	uv0.x = u0; uv1.x = u1;

	if ( pNextSec == NULL || (pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_MASKWALL) )
	{
		Vector3 posList[4];
		Vector2 uvList[4];
		if ( pNextSec == NULL )
		{
			posList[0].Set(worldPos[0].x, worldPos[0].y, pCurSec->GetZ_Floor(worldPos[0].x, worldPos[0].y, Sectors));
			posList[1].Set(worldPos[1].x, worldPos[1].y, pCurSec->GetZ_Floor(worldPos[1].x, worldPos[1].y, Sectors));
			posList[2].Set(worldPos[1].x, worldPos[1].y, pCurSec->GetZ_Ceil(worldPos[1].x, worldPos[1].y, Sectors));
			posList[3].Set(worldPos[0].x, worldPos[0].y, pCurSec->GetZ_Ceil(worldPos[0].x, worldPos[0].y, Sectors));
		}
		else
		{
			posList[0].Set( worldPos[0].x, worldPos[0].y, MAX( pCurSec->GetZ_Floor(worldPos[0].x, worldPos[0].y, Sectors), pNextSec->GetZ_Floor(worldPos[0].x, worldPos[0].y, Sectors)) );
			posList[1].Set( worldPos[1].x, worldPos[1].y, MAX( pCurSec->GetZ_Floor(worldPos[1].x, worldPos[1].y, Sectors), pNextSec->GetZ_Floor(worldPos[1].x, worldPos[1].y, Sectors)) );
			posList[2].Set( worldPos[1].x, worldPos[1].y, MIN( pCurSec->GetZ_Ceil(worldPos[1].x, worldPos[1].y, Sectors), pNextSec->GetZ_Ceil(worldPos[1].x, worldPos[1].y, Sectors)) );
			posList[3].Set( worldPos[0].x, worldPos[0].y, MIN( pCurSec->GetZ_Ceil(worldPos[0].x, worldPos[0].y, Sectors), pNextSec->GetZ_Ceil(worldPos[0].x, worldPos[0].y, Sectors)) );
		}

		if ( (!(pCurSec->m_uFlags&SEC_FLAGS_FLOOR_SLOPE) && !(pCurSec->m_uFlags&SEC_FLAGS_CEIL_SLOPE)) || dz == 0.0f )
		{
			uvList[0].Set( uv0.x, uv0.y );
			uvList[1].Set( uv1.x, uv0.y );
			uvList[2].Set( uv1.x, uv1.y );
			uvList[3].Set( uv0.x, uv1.y );
		}
		else
		{
			f32 fOODZ = 1.0f/dz;
			f32 v0 = (posList[0].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v1 = (posList[1].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v2 = (posList[2].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v3 = (posList[3].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;

			uvList[0].Set( uv0.x, uv0.y + v0*(uv1.y - uv0.y) );
			uvList[1].Set( uv1.x, uv0.y + v1*(uv1.y - uv0.y) );
			uvList[2].Set( uv1.x, uv0.y + v2*(uv1.y - uv0.y) );
			uvList[3].Set( uv0.x, uv0.y + v3*(uv1.y - uv0.y) );
		}

		if ( pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_TRANS )
		{
			color.w = 0.5f;
		}
		else if ( pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_MASKWALL )
		{
			color.w = 0.99f;
		}
		
		bool bForceZWrite = false;
		if ( pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_SKY )
		{
			bForceZWrite = true;
			color.w = 0.0f;
		}

		RenderQue::AddQuad(hTex, posList, uvList, color, bForceZWrite);
	}
	color.w = 1.0f;
	if ( pNextSec )
	{
		if ( pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_XFLIP )
		{
			uv0.x = pCurSec->m_Walls[w].m_texOffset[Wall::WALL_TEX_BOT].x + pCurSec->m_Walls[w].m_wallLen*pCurSec->m_Walls[w].m_texScale[Wall::WALL_TEX_BOT].x;
			uv1.x = pCurSec->m_Walls[w].m_texOffset[Wall::WALL_TEX_BOT].x;
		}
		else
		{
			uv0.x = pCurSec->m_Walls[w].m_texOffset[Wall::WALL_TEX_BOT].x;
			uv1.x = pCurSec->m_Walls[w].m_texOffset[Wall::WALL_TEX_BOT].x + pCurSec->m_Walls[w].m_wallLen*pCurSec->m_Walls[w].m_texScale[Wall::WALL_TEX_BOT].x;
		}

		if ( pCurSec->m_Walls[w].m_flags&Wall::WALL_FLAGS_YFLIP )
		{
			uv0.y = pCurSec->m_Walls[w].m_texOffset[Wall::WALL_TEX_BOT].y;
			uv1.y = uv0.y+dz*pCurSec->m_Walls[w].m_texScale[Wall::WALL_TEX_BOT].y;
		}
		else
		{
			uv1.y = pCurSec->m_Walls[w].m_texOffset[Wall::WALL_TEX_BOT].y;
			uv0.y = uv1.y+dz*pCurSec->m_Walls[w].m_texScale[Wall::WALL_TEX_BOT].y;
		}

		u0 = uv0.x * (1.0f - t0) + uv1.x * t0;
		u1 = uv0.x * (1.0f - t1) + uv1.x * t1;
		uv0.x = u0; uv1.x = u1;

		Sector_2_5D *pTopSec = pNextSec;
		if ( pCurSec->m_Walls[w].m_adjoin[1] != 0xffff )
		{
			pTopSec = (Sector_2_5D *)Sectors[pCurSec->m_Walls[w].m_adjoin[1]];
			if ( pTopSec->m_ZRangeCur.y < pNextSec->m_ZRangeCur.x )
			{
				//we need to swap...
				Sector_2_5D *pTemp = pTopSec;
				pTopSec = pNextSec;
				pNextSec = pTemp;
			}
		}

		//ok we have to get the floor heights in both sectors to do this comparison correctly...
		f32 z0 = pCurSec->GetZ_Floor(worldPos[0].x, worldPos[0].y, Sectors);
		f32 z1 = pCurSec->GetZ_Floor(worldPos[1].x, worldPos[1].y, Sectors);
		f32 z2 = pNextSec->GetZ_Floor(worldPos[0].x, worldPos[0].y, Sectors);
		f32 z3 = pNextSec->GetZ_Floor(worldPos[1].x, worldPos[1].y, Sectors);

		//draw bottom - only if floor is higher in the next sector.
		if ( z2 > z0 || z3 > z1 )
		{
			hTex = pCurSec->m_Walls[w].m_textures[ Wall::WALL_TEX_BOT ];

			Vector3 posList[4];
			Vector2 uvList[4];
			posList[0].Set(worldPos[0].x, worldPos[0].y, z0);
			posList[1].Set(worldPos[1].x, worldPos[1].y, z1);
			posList[2].Set(worldPos[1].x, worldPos[1].y, z3);
			posList[3].Set(worldPos[0].x, worldPos[0].y, z2);

			f32 fOODZ = 1.0f/dz;
			f32 v0 = (posList[0].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v1 = (posList[1].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v2 = (posList[2].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v3 = (posList[3].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;

			uvList[0].Set( uv0.x, uv0.y + v0*(uv1.y - uv0.y) );
			uvList[1].Set( uv1.x, uv0.y + v1*(uv1.y - uv0.y) );
			uvList[2].Set( uv1.x, uv0.y + v2*(uv1.y - uv0.y) );
			uvList[3].Set( uv0.x, uv0.y + v3*(uv1.y - uv0.y) );

			bool bForceZWrite = false;
			if ( pNextSec->m_uFlags&Sector_2_5D::SEC_FLAGS_SKYFLOOR )
			{
				bForceZWrite = true;
				color.w = 0.0f;
			}

			RenderQue::AddQuad(hTex, posList, uvList, color, bForceZWrite);
		}

		//ok we have to get the floor heights in both sectors to do this comparison correctly...
		z0 = pCurSec->GetZ_Ceil(worldPos[0].x, worldPos[0].y, Sectors);
		z1 = pCurSec->GetZ_Ceil(worldPos[1].x, worldPos[1].y, Sectors);
		z2 = MAX(z2, pTopSec->GetZ_Ceil(worldPos[0].x, worldPos[0].y, Sectors));
		z3 = MAX(z3, pTopSec->GetZ_Ceil(worldPos[1].x, worldPos[1].y, Sectors));

		//if this sector has a vertical adjoin, we have to clip the bottom...
		#if 0
		if ( pTopSec )
		{
			z0 = MIN( z0, pTopSec->GetZ_Floor(worldPos[0].x+pCurSec->m_vAdjOffset[1].x, worldPos[0].y+pCurSec->m_vAdjOffset[1].y)+pCurSec->m_vAdjOffset[1].z );
			z1 = MIN( z1, pTopSec->GetZ_Floor(worldPos[1].x+pCurSec->m_vAdjOffset[1].x, worldPos[1].y+pCurSec->m_vAdjOffset[1].y)+pCurSec->m_vAdjOffset[1].z );
			z2 = MIN( z2, pTopSec->GetZ_Floor(worldPos[0].x+pCurSec->m_vAdjOffset[1].x, worldPos[0].y+pCurSec->m_vAdjOffset[1].y)+pCurSec->m_vAdjOffset[1].z );
			z3 = MIN( z3, pTopSec->GetZ_Floor(worldPos[1].x+pCurSec->m_vAdjOffset[1].x, worldPos[1].y+pCurSec->m_vAdjOffset[1].y)+pCurSec->m_vAdjOffset[1].z );
		}
		#endif

		//draw the top - only if the ceiling is lower in the next sector.
		if ( z2 < z0 || z3 < z1 )
		{
			hTex = pCurSec->m_Walls[w].m_textures[ Wall::WALL_TEX_TOP ];
			f32 dz = pCurSec->m_ZRangeCur.y - pCurSec->m_ZRangeCur.x;

			Vector3 posList[4];
			Vector2 uvList[4];
			posList[0].Set(worldPos[0].x, worldPos[0].y, z0);
			posList[1].Set(worldPos[1].x, worldPos[1].y, z1);
			posList[2].Set(worldPos[1].x, worldPos[1].y, z3);
			posList[3].Set(worldPos[0].x, worldPos[0].y, z2);

			f32 fOODZ = 1.0f/dz;
			f32 v0 = (posList[0].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v1 = (posList[1].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v2 = (posList[2].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v3 = (posList[3].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;

			uvList[0].Set( uv0.x, uv0.y + v0*(uv1.y - uv0.y) );
			uvList[1].Set( uv1.x, uv0.y + v1*(uv1.y - uv0.y) );
			uvList[2].Set( uv1.x, uv0.y + v2*(uv1.y - uv0.y) );
			uvList[3].Set( uv0.x, uv0.y + v3*(uv1.y - uv0.y) );

			bool bForceZWrite = false;
			if ( pTopSec->m_uFlags&Sector_2_5D::SEC_FLAGS_EXTERIOR )
			{
				bForceZWrite = true;
				color.w = 0.0f;
			}

			RenderQue::AddQuad(hTex, posList, uvList, color, bForceZWrite);
		}

		//Finally draw the "middle" connecting piece, if necessary.
		if ( pTopSec != pNextSec )
		{
			//ok we have to get the floor heights in both sectors to do this comparison correctly...
			z0 = pNextSec->GetZ_Ceil(worldPos[0].x, worldPos[0].y, Sectors);
			z1 = pNextSec->GetZ_Ceil(worldPos[1].x, worldPos[1].y, Sectors);
			z2 = pTopSec->GetZ_Floor(worldPos[0].x, worldPos[0].y, Sectors);
			z3 = pTopSec->GetZ_Floor(worldPos[1].x, worldPos[1].y, Sectors);

			hTex = pCurSec->m_Walls[w].m_textures[ Wall::WALL_TEX_MID ];
			f32 dz = pCurSec->m_ZRangeCur.y - pCurSec->m_ZRangeCur.x;

			Vector3 posList[4];
			Vector2 uvList[4];
			posList[0].Set(worldPos[0].x, worldPos[0].y, z0);
			posList[1].Set(worldPos[1].x, worldPos[1].y, z1);
			posList[2].Set(worldPos[1].x, worldPos[1].y, z3);
			posList[3].Set(worldPos[0].x, worldPos[0].y, z2);

			f32 fOODZ = 1.0f/dz;
			f32 v0 = (posList[0].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v1 = (posList[1].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v2 = (posList[2].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;
			f32 v3 = (posList[3].z - pCurSec->m_ZRangeCur.x-s_vCurrentOffs.z) * fOODZ;

			uvList[0].Set( uv0.x, uv0.y + v0*(uv1.y - uv0.y) );
			uvList[1].Set( uv1.x, uv0.y + v1*(uv1.y - uv0.y) );
			uvList[2].Set( uv1.x, uv0.y + v2*(uv1.y - uv0.y) );
			uvList[3].Set( uv0.x, uv0.y + v3*(uv1.y - uv0.y) );

			RenderQue::AddQuad(hTex, posList, uvList, color, false);
		}
	}
}

bool ClipAgainstFrustum(const Vector2& in_v0, const Vector2& in_v1, Vector2& out_v0, Vector2& out_v1, bool& bVisible)
{
	bool bClip = false;

	//Is the wall inside the current frustum?
	Vector2 A, B;
	A.Set(in_v0.x-_cpos.x, in_v0.y-_cpos.y); B.Set(in_v1.x-_cpos.x, in_v1.y-_cpos.y);
	if ( ((Nl.Dot(A) < 0.0f && Nl.Dot(B) < 0.0f)) || ((Nr.Dot(A) < 0.0f && Nr.Dot(B) < 0.0f)) )
	{ bVisible = false; return false; }

	out_v0 = in_v0;
	out_v1 = in_v1;

	//Split walls by the frustum.
	//split by left plane?
	if ( Nl.Dot(A) < 0.0f && Nl.Dot(B) > 0.0f )
	{
		float s, t;
		Vector2 start(_fL.x-1000.0f*Dl.x, _fL.y-1000.0f*Dl.y);
		Vector2 end(_fL.x+1000.0f*Dl.x, _fL.y+1000.0f*Dl.y);
		if ( Math::LineIntersect2D(start, end, out_v0, out_v1, s, t) )
		{
			out_v0 = out_v0 + (out_v1-out_v0)*t;
			bClip = true;
		}
	}
	else if ( Nl.Dot(A) > 0.0f && Nl.Dot(B) < 0.0f )
	{
		float s, t;
		Vector2 start(_fL.x-1000.0f*Dl.x, _fL.y-1000.0f*Dl.y);
		Vector2 end(_fL.x+1000.0f*Dl.x, _fL.y+1000.0f*Dl.y);
		if ( Math::LineIntersect2D(start, end, out_v0, out_v1, s, t) )
		{
			out_v1 = out_v0 + (out_v1-out_v0)*t;
			bClip = true;
		}
	}
	//split by right plane?
	A.Set(out_v0.x-_cpos.x, out_v0.y-_cpos.y); B.Set(out_v1.x-_cpos.x, out_v1.y-_cpos.y);
	if ( Nr.Dot(A) < 0.0f && Nr.Dot(B) < 0.0f )
	{ bVisible = false; return false; }

	if ( Nr.Dot(A) < 0.0f && Nr.Dot(B) > 0.0f )
	{
		float s, t;
		Vector2 start(_fR.x-1000.0f*Dr.x, _fR.y-1000.0f*Dr.y);
		Vector2 end(_fR.x+1000.0f*Dr.x, _fR.y+1000.0f*Dr.y);
		if ( Math::LineIntersect2D(start, end, out_v0, out_v1, s, t) )
		{
			out_v0 = out_v0 + (out_v1-out_v0)*t;
			bClip = true;
		}
	}
	else if ( Nr.Dot(A) > 0.0f && Nr.Dot(B) < 0.0f )
	{
		float s, t;
		Vector2 start(_fR.x-1000.0f*Dr.x, _fR.y-1000.0f*Dr.y);
		Vector2 end(_fR.x+1000.0f*Dr.x, _fR.y+1000.0f*Dr.y);
		if ( Math::LineIntersect2D(start, end, out_v0, out_v1, s, t) )
		{
			out_v1 = out_v0 + (out_v1-out_v0)*t;
			bClip = true;
		}
	}

#if 0
	//clip against the near plane.
	A.Set(out_v0.x-_nearZ.x, out_v0.y-_nearZ.y); B.Set(out_v1.x-_nearZ.x, out_v1.y-_nearZ.y);
	f32 dotA = _nearZ_Nrml.Dot(A);
	f32 dotB = _nearZ_Nrml.Dot(B);
	if ( dotA < 0.0f && dotB < 0.0f )
	{ bVisible = false; return false; }

	if ( dotA < 0.0f && dotB > 0.0f )
	{
		f32 s = -dotA / (dotB - dotA);
		if ( s > 0.0f && s < 1.0f )
		{
			out_v0 = out_v0 + (out_v1-out_v0)*s;
			bClip = true;
		}
	}
	else if ( dotA > 0.0f && dotB < 0.0f )
	{
		f32 s = -dotA / (dotB - dotA);
		if ( s > 0.0f && s < 1.0f )
		{
			out_v1 = out_v0 + (out_v1-out_v0)*s;
			bClip = true;
		}
	}
#endif

	return bClip;
}

void Sector_2_5D::_SetupCameraParameters(const Vector3& cPos, const Vector3& cDir, Vector2 fL, Vector2 fR)
{
	Nl.Set( -(fL.y-cPos.y), fL.x-cPos.x );
	Nl.Normalize();

	Nr.Set( (fR.y-cPos.y), -(fR.x-cPos.x) );
	Nr.Normalize();

	Nz.Set( (fR.y-fL.y), -(fR.x-fL.x) );
	Nz.Normalize();

	Dl.Set(fL.x-cPos.x, fL.y-cPos.y);
	Dr.Set(fR.x-cPos.x, fR.y-cPos.y);
	Dz.Set(fR.x-fL.x, fR.y-fL.y);
	Dl.Normalize(); Dr.Normalize(); Dz.Normalize();

	_fL = fL;
	_fR = fR;
	_nearZ = _fL;
	_nearZ_Nrml.Set( (fR.y - fL.y), -(fR.x - fL.x) );
	_nearZ_Nrml.Normalize();
	_cpos = cPos;
}

void Sector_2_5D::WallRasterizer(const Vector3& cPos, u32 uStartX, u32 uEndX, Sector_2_5D *pCurSec, const vector<Sector *>& Sectors)
{
	//transform all vertices into 2D projected space (x, 1/w)
	m_Camera2D.TransformPointsSS_2D(pCurSec->m_uVertexCount, pCurSec->m_pVertexCur, ssPos, Vector2::Zero);

	//we have to restore the depth after vertical adjoins.
	for (u32 x=uStartX; x<=uEndX; x++)
	{
		//if ( fDepth[x] > 0 ) fDepthPrev[x] = fDepth[x];
		fDepth[x] = 0.0f;
		sBuffer[x] = 0;
		idBuffer[x] = 0;
	}

	//1. Figure out which walls may be visible.
	for (u32 w=0; w<pCurSec->m_uWallCount; w++)
	{
		u16 i0 = pCurSec->m_Walls[w].m_idx[0];
		u16 i1 = pCurSec->m_Walls[w].m_idx[1];

		const Vector2& v0 = pCurSec->m_pVertexCur[i0];
		const Vector2& v1 = pCurSec->m_pVertexCur[i1];

		//this wall is on or behind the near plane.
		if ( ssPos[i0].y <= 0.0f && ssPos[i1].y <= 0.0f )
			continue;

		//is this wall degenerate?
		Vector2 offs = v1 - v0;
		f32 l = offs.Dot(offs);
		if ( l < 0.00001f )
			continue;

		Vector2 wD = v1 - v0, wN;
		wN.x = wD.y; wN.y = -wD.x;	//normal is flipped due to winding. N = -perp. perp = (-dy,dx), so N = (dy,-dx)
		//wN.Normalize();	--no need to normalize unless I'm lighting.

		//Is this wall facing toward the camera?
		Vector2 D( cPos.x-v0.x, cPos.y-v0.y );
		f32 dp = D.Dot(wN);

		if ( dp < 0.0f )
			continue;

		Vector2 ws[2];
		Vector2 ss[2];
		bool bVisible = true;
		if ( ClipAgainstFrustum(v0, v1, ws[0], ws[1], bVisible) )
		{
			m_Camera2D.TransformPointsSS_2D(2, ws, ss, Vector2::Zero);
		}
		else
		{
			if ( bVisible == false )
				continue;

			ss[0] = ssPos[i0];
			ss[1] = ssPos[i1];
		}

		//finally rasterize to the 1D depth buffer.
		s32 a=0, b=1;
		if ( ss[0].x > ss[1].x )
		{
			b = 0; a = 1;
		}
		f32 fx0 = (ss[a].x*0.5f+0.5f)*(fbwidth-1.0f);
		f32 fx1 = (ss[b].x*0.5f+0.5f)*(fbwidth-1.0f);
		f32 z0 = ss[a].y;
		f32 z1 = ss[b].y;

		s32 x0 = (s32)fx0;
		s32 x1 = (s32)(fx1+0.5f);

		if ( x1 - x0 < 1 )
			continue;

		s32 x0s = x0;
		s32 x1s = x1;

		//now figure out the z values at the pixel centers.
		f32 dz = z1 - z0;
		f32 z0s = z0, z1s = z1;
		f32 s0 = ((f32)x0 - fx0) / (fx1 - fx0);
		f32 s1 = ((f32)x1 - fx0) / (fx1 - fx0);
		z0 = z0s + s0*(z1s - z0s);
		z1 = z0s + s1*(z1s - z0s);

		if ( x0 > (s32)uEndX || x1 < (s32)uStartX )
			continue;

		//f32 z0s = z0, z1s = z1;
		z0s = z0; z1s = z1;
		//f32 s0 = 0.0f, s1 = 1.0f;
		s0 = 0.0f; s1 = 1.0f;
		if ( x0 < (s32)uStartX ) 
		{
			f32 s = ( (f32)uStartX-(f32)x0s ) / ( (f32)x1s - (f32)x0s );
			z0 = s * (z1s - z0s) + z0s;
			x0 = uStartX;
		}
		if ( x1 > (s32)uEndX ) 
		{
			f32 s = ( (f32)uEndX-(f32)x0s ) / ( (f32)x1s - (f32)x0s );
			z1 = s * (z1s - z0s) + z0s;
			x1 = uEndX;
		}
		f32 fOORange = (x1-x0>0) ? 1.0f/(f32)(x1-x0) : 0.0f;
		//f32 dz = (z1-z0) * fOORange;
		dz = (z1-z0) * fOORange;
		f32 z = z0;

		const f32 eps = 0.00001f;
		for (s32 x=x0; x<=x1; x++, z+=dz)
		{
			if ( z > fDepth[x] )//&& z < fDepthPrev[x]+eps )
			{
				fDepth[x] = z;
				idBuffer[x] = w;
				sBuffer[x] = pCurSec->m_uID;
			}
		}
	}

	//return;

	//now figure out the adjoins and add new sectors to the render list.
	Vector2 worldPos[2];
	s32 x0=uStartX;
	u16 id = idBuffer[uStartX];
	u16 sec = sBuffer[uStartX];
	Sector_2_5D *pNextSec = NULL;
	s32 x=(s32)uStartX+1;
	for (; x<=(s32)uEndX; x++)
	{
		if ( idBuffer[x] != id || sBuffer[x] != sec )
		{
			//draw the wall...
			if ( sec == pCurSec->m_uID )
			{
				s32 x1 = x-1;

				f32 z0 = fDepth[x0];
				f32 z1 = fDepth[x1];

				m_Camera2D.InverseTransformPointsSS_2D((f32)(x0-0.5f)/fbwidth * 2.0f - 1.0f, z0, worldPos[0]);
				m_Camera2D.InverseTransformPointsSS_2D((f32)(x1+0.5f)/fbwidth * 2.0f - 1.0f, z1, worldPos[1]);

				//draw the wall
				pNextSec = pCurSec->m_Walls[id].m_adjoin[0] == 0xffff ? NULL : (Sector_2_5D *)Sectors[pCurSec->m_Walls[id].m_adjoin[0]];

				//add the next buffer.
				//now make sure that the wall opening is visible...
				if ( pNextSec && pNextSec->m_ZRangeCur.y <= pNextSec->m_ZRangeCur.x )
					pNextSec = NULL;
				if ( pNextSec && x1-x0 > 1 )
				{
					VisStack_Push(worldPos[0], worldPos[1], x0, x1, pNextSec);
				}
			}

			//start again.
			x0 = x;
			id = idBuffer[x];
			sec = sBuffer[x];
		}
	}
	if ( sec == pCurSec->m_uID )
	{
		//the last wall
		s32 x1 = uEndX;

		m_Camera2D.InverseTransformPointsSS_2D((f32)(x0-0.5f)/fbwidth * 2.0f - 1.0f, fDepth[x0], worldPos[0]);
		m_Camera2D.InverseTransformPointsSS_2D((f32)(x1+0.5f)/fbwidth * 2.0f - 1.0f, fDepth[x1], worldPos[1]);

		//draw the wall
		pNextSec = pCurSec->m_Walls[id].m_adjoin[0] == 0xffff ? NULL : (Sector_2_5D *)Sectors[pCurSec->m_Walls[id].m_adjoin[0]];

		//add the next buffer.
		//now make sure that the wall opening is visible...
		if ( pNextSec && pNextSec->m_ZRangeCur.y <= pNextSec->m_ZRangeCur.x )
			pNextSec = NULL;
		if ( pNextSec && x1-x0 > 1 )
		{
			VisStack_Push(worldPos[0], worldPos[1], x0, x1, pNextSec);
		}
	}
}

void Sector_2_5D::Visibility2D(const Vector3& cPos, Vector2 fL, Vector2 fR, u32 uStartX, u32 uEndX, Sector_2_5D *pCurSec, const vector<Sector *>& Sectors, bool bUsePortalClip, IDriver3D *pDriver)
{
	//insert vadjoins first?
	if ( _bAllowVAdjoin )
	{
		if ( pCurSec->m_vAdjoin[0] != 0xffff )
		{
			Sector_2_5D *pNextSec = (Sector_2_5D *)Sectors[pCurSec->m_vAdjoin[0]];
			if ( cPos.z > pCurSec->GetZ_Floor(cPos.x, cPos.y, Sectors) )
			{
				//add the next buffer.
				if ( pNextSec )
				{
					if ( m_visStackCnt_VAdjoin < _MAX_RECURSION_LEVEL )
					{
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fL = fL;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fR = fR;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uStartX = uStartX;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uEndX   = uEndX;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].pNext = pNextSec;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].bUsePortalClip = true;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].offset = pCurSec->m_vAdjOffset[0]+s_vCurrentOffs;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth = &_VAdjoinBuffer[_uAdjoinBufferLoc];
						if ( _uAdjoinBufferLoc < _VADJOIN_BUFFER_SIZE-(uEndX-uStartX+2) )
						{
							memcpy( m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth, &fDepth[uStartX], sizeof(f32)*(uEndX-uStartX+1));
							_uAdjoinBufferLoc += (uEndX-uStartX+1);
							m_visStackCnt_VAdjoin++;
						}
					}
				}
			}
		}
		if ( pCurSec->m_vAdjoin[1] != 0xffff )
		{
			Sector_2_5D *pNextSec = (Sector_2_5D *)Sectors[pCurSec->m_vAdjoin[1]];
			if ( cPos.z < pCurSec->GetZ_Ceil(cPos.x, cPos.y, Sectors) )
			{
				//add the next buffer.
				if ( pNextSec )
				{
					if ( m_visStackCnt_VAdjoin < _MAX_RECURSION_LEVEL )
					{
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fL = fL;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fR = fR;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uStartX = uStartX;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uEndX   = uEndX;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].pNext = pNextSec;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].bUsePortalClip = true;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].offset = pCurSec->m_vAdjOffset[1]+s_vCurrentOffs;
						m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth = &_VAdjoinBuffer[_uAdjoinBufferLoc];
						if ( _uAdjoinBufferLoc < _VADJOIN_BUFFER_SIZE-(uEndX-uStartX+2) )
						{
							memcpy( m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth, &fDepth[uStartX], sizeof(f32)*(uEndX-uStartX+1));
							_uAdjoinBufferLoc += (uEndX-uStartX+1);
							m_visStackCnt_VAdjoin++;
						}
					}
				}
			}
		}
	}

	//transform all vertices into 2D projected space (x, 1/w)
	m_Camera2D.TransformPointsSS_2D(pCurSec->m_uVertexCount, pCurSec->m_pVertexCur, ssPos, Vector2(s_vCurrentOffs.x, s_vCurrentOffs.y));

	//we have to restore the depth after vertical adjoins.
	//static f32 fDepth_vAdjoin[4096];
	if ( pCurSec != _pStartSec || uStartX != 0 || uEndX != bwidth-1 )
	{
		for (u32 x=uStartX; x<=uEndX; x++)
		{
			fDepthPrev[x] = (bUsePortalClip) ? fDepth[x] : 1000000.0f;
			//fDepth_vAdjoin[x] = fDepth[x];
			fDepth[x] = 0.0f;
		}
	}

	//is this sector degenerate? If so then we can treat the near plane clipping in a special way to avoid
	//floating point precision issues.
	bool bDegenerate = false;
	if ( pCurSec->m_uWallCount <= 4 )
	{
		u16 i0 = pCurSec->m_Walls[0].m_idx[0];
		u16 i1 = pCurSec->m_Walls[0].m_idx[1];

		const Vector2& v0 = pCurSec->m_pVertexCur[i0] + Vector2(s_vCurrentOffs.x, s_vCurrentOffs.y);
		const Vector2& v1 = pCurSec->m_pVertexCur[i1] + Vector2(s_vCurrentOffs.x, s_vCurrentOffs.y);

		i0 = pCurSec->m_Walls[pCurSec->m_uWallCount-1].m_idx[0];
		i1 = pCurSec->m_Walls[pCurSec->m_uWallCount-1].m_idx[1];

		const Vector2& v2 = pCurSec->m_pVertexCur[i0] + Vector2(s_vCurrentOffs.x, s_vCurrentOffs.y);
		const Vector2& v3 = pCurSec->m_pVertexCur[i1] + Vector2(s_vCurrentOffs.x, s_vCurrentOffs.y);

		Vector2 A = v1 - v0;
		Vector2 B = v3 - v2;
		A.Normalize();
		B.Normalize();
		float dAngle = fabsf( A.Dot(B) );
		if ( dAngle > 0.999999f )
		{
			bDegenerate = true;
		}
	}

	//0. Make a render order, solid first then adjoins.
	static u16 auRenderOrder[65536];
	u32 uCurRenderIndex = 0;
	u32 uAdjoinStart = 65536;
	for (u32 w=0; w<pCurSec->m_uWallCount; w++)
	{
		if ( pCurSec->m_Walls[w].m_adjoin[0] == 0xffff )
			continue;

		auRenderOrder[ uCurRenderIndex++ ] = w;
	}
	uAdjoinStart = uCurRenderIndex;
	for (u32 w=0; w<pCurSec->m_uWallCount; w++)
	{
		if ( pCurSec->m_Walls[w].m_adjoin[0] < 0xffff )
			continue;

		auRenderOrder[ uCurRenderIndex++ ] = w;
	}

	//1. Figure out which walls may be visible.
	for (u32 ww=0; ww<pCurSec->m_uWallCount; ww++)
	{
		f32 fDepthBias = 0.0f;//(ww<uAdjoinStart) ? 0.0f : 0.001f;

		u32 w = auRenderOrder[ww];
		u16 i0 = pCurSec->m_Walls[w].m_idx[0];
		u16 i1 = pCurSec->m_Walls[w].m_idx[1];

		const Vector2& v0 = pCurSec->m_pVertexCur[i0] + Vector2(s_vCurrentOffs.x, s_vCurrentOffs.y);
		const Vector2& v1 = pCurSec->m_pVertexCur[i1] + Vector2(s_vCurrentOffs.x, s_vCurrentOffs.y);

		//this wall is on or behind the near plane.
		if ( ssPos[i0].y <= 0.0f && ssPos[i1].y <= 0.0f )
			continue;

		//is this wall degenerate?
		Vector2 offs = v1 - v0;
		f32 l = offs.Dot(offs);
		if ( l < 0.00001f )
			continue;

		Vector2 wD = v1 - v0, wN;
		wN.x = wD.y; wN.y = -wD.x;	//normal is flipped due to winding. N = -perp. perp = (-dy,dx), so N = (dy,-dx)
		//wN.Normalize();	--no need to normalize unless I'm lighting.

		//Is this wall facing toward the camera?
		Vector2 D( cPos.x-v0.x, cPos.y-v0.y );
		f32 dp = D.Dot(wN);

		if ( dp < 0.0f )
			continue;

		//bool bAdjoinWall = pCurSec->m_Walls[w].m_adjoin[0] < 0xffff ? true : false;

		Vector2 ws[2];
		Vector2 ss[2];
		bool bVisible = true;
		if ( ClipAgainstFrustum(v0, v1, ws[0], ws[1], bVisible) )
		{
			m_Camera2D.TransformPointsSS_2D(2, ws, ss, Vector2(0.0f, 0.0f));
		}
		else
		{
			if ( bVisible == false )
				continue;

			ss[0] = ssPos[i0];
			ss[1] = ssPos[i1];
		}

		//finally rasterize to the 1D depth buffer.
		s32 a=0, b=1;
		if ( ss[0].x > ss[1].x )
		{
			b = 0; a = 1;
		}
		f32 fx0 = (ss[a].x*0.5f+0.5f)*(fbwidth-1.0f);
		f32 fx1 = (ss[b].x*0.5f+0.5f)*(fbwidth-1.0f);
		f32 z0 = ss[a].y;
		f32 z1 = ss[b].y;

		s32 x0 = (s32)fx0;
		s32 x1 = (s32)(fx1+0.5f);

		if ( x1 - x0 < 1 )
			continue;

		s32 x0s = x0;
		s32 x1s = x1;

		if ( x0 > (s32)uEndX || x1 < (s32)uStartX )
			continue;

		f32 z0s = z0, z1s = z1;
		f32 s0 = 0.0f, s1 = 1.0f;
		if ( x0 < (s32)uStartX ) 
		{
			f32 s = ( (f32)uStartX-(f32)x0s ) / ( (f32)x1s - (f32)x0s );
			z0 = s * (z1s - z0s) + z0s;
			x0 = uStartX;
		}
		if ( x1 > (s32)uEndX ) 
		{
			f32 s = ( (f32)uEndX-(f32)x0s ) / ( (f32)x1s - (f32)x0s );
			z1 = s * (z1s - z0s) + z0s;
			x1 = uEndX;
		}
		f32 fOORange = (x1-x0>0) ? 1.0f/(f32)(x1-x0) : 0.0f;
		f32 dz = (z1-z0) * fOORange;
		f32 z = z0;

		//const f32 eps = 0.0005f;
		for (s32 x=x0; x<=x1; x++, z+=dz)
		{
			if ( z > fDepth[x]+fDepthBias && ( z <= fDepthPrev[x] || bDegenerate ) )
			{
				fDepth[x] = z;
				idBuffer[x] = w;
				sBuffer[x] = pCurSec->m_uID;
			}
		}
	}

	//now generate wall segments and flat quads from the 1D depth and ID buffer.
	Vector2 a(cPos.x, cPos.y);
	Vector2 n0(fL.x, fL.y), n1(fR.x, fR.y);

	Vector2 worldPos[2];
	s32 x0=uStartX;
	u16 id = idBuffer[uStartX];
	u16 sec = sBuffer[uStartX];
	Sector_2_5D *pNextSec = NULL;
	s32 x=(s32)uStartX+1;
	Sector_2_5D *pBotSec = pCurSec->m_vAdjoin[0] == 0xffff ? NULL : (Sector_2_5D *)Sectors[pCurSec->m_vAdjoin[0]];
	Sector_2_5D *pTopSec = pCurSec->m_vAdjoin[1] == 0xffff ? NULL : (Sector_2_5D *)Sectors[pCurSec->m_vAdjoin[1]];
	for (; x<=(s32)uEndX; x++)
	{
		if ( idBuffer[x] != id || sBuffer[x] != sec )
		{
			//draw the wall...
			if ( sec == pCurSec->m_uID )
			{
				s32 x1 = x-1;

				m_Camera2D.InverseTransformPointsSS_2D((f32)(x0-0.5f)/fbwidth * 2.0f - 1.0f, fDepth[x0], worldPos[0]);
				m_Camera2D.InverseTransformPointsSS_2D((f32)(x1+0.5f)/fbwidth * 2.0f - 1.0f, fDepth[x1], worldPos[1]);

				//draw the wall
				pNextSec = pCurSec->m_Walls[id].m_adjoin[0] == 0xffff ? NULL : (Sector_2_5D *)Sectors[pCurSec->m_Walls[id].m_adjoin[0]];
				_DrawWall(pDriver, pCurSec, pNextSec, pBotSec, pTopSec, id, worldPos, Sectors);

				//draw the floor
				_DrawFloor(pDriver, pCurSec, worldPos, a, n0, n1, Sectors);

				//add the next buffer.
				//now make sure that the wall opening is visible...
				if ( pNextSec && pNextSec->m_ZRangeCur.y <= pNextSec->m_ZRangeCur.x )
					pNextSec = NULL;
				if ( pNextSec && x1-x0 > 1 )
				{
					VisStack_Push(worldPos[0], worldPos[1], x0, x1, pNextSec);
				}

				if ( pCurSec->m_Walls[id].m_adjoin[1] != 0xffff )
				{
					pNextSec = (Sector_2_5D *)Sectors[pCurSec->m_Walls[id].m_adjoin[1]];
					//add the next buffer.
					if ( pNextSec )
					{
						if ( m_visStackCnt_VAdjoin < _MAX_RECURSION_LEVEL )
						{
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fL = worldPos[0];
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fR = worldPos[1];
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uStartX = x0;
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uEndX   = x1;
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].pNext = pNextSec;
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].bUsePortalClip = true;
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].offset = s_vCurrentOffs;
							m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth = &_VAdjoinBuffer[_uAdjoinBufferLoc];
							if ( _uAdjoinBufferLoc < _VADJOIN_BUFFER_SIZE-(uEndX-uStartX+2) )
							{
								memcpy( m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth, &fDepth[x0], sizeof(f32)*(x1-x0+1));
								_uAdjoinBufferLoc += (x1-x0+1);
								m_visStackCnt_VAdjoin++;
							}
						}
					}
				}
			}

			//start again.
			x0 = x;
			id = idBuffer[x];
			sec = sBuffer[x];
		}
	}
	if ( sec == pCurSec->m_uID )
	{
		//the last wall
		s32 x1 = uEndX;

		m_Camera2D.InverseTransformPointsSS_2D((f32)(x0-0.5f)/fbwidth * 2.0f - 1.0f, fDepth[x0], worldPos[0]);
		m_Camera2D.InverseTransformPointsSS_2D((f32)(x1+0.5f)/fbwidth * 2.0f - 1.0f, fDepth[x1], worldPos[1]);

		//draw the wall
		pNextSec = pCurSec->m_Walls[id].m_adjoin[0] == 0xffff ? NULL : (Sector_2_5D *)Sectors[pCurSec->m_Walls[id].m_adjoin[0]];
		_DrawWall(pDriver, pCurSec, pNextSec, pBotSec, pTopSec, id, worldPos, Sectors);

		//draw the floor
		_DrawFloor(pDriver, pCurSec, worldPos, a, n0, n1, Sectors);

		//add the next buffer.
		//now make sure that the wall opening is visible...
		if ( pNextSec && pNextSec->m_ZRangeCur.y <= pNextSec->m_ZRangeCur.x )
			pNextSec = NULL;
		if ( pNextSec && x1-x0 > 1 )
		{
			VisStack_Push(worldPos[0], worldPos[1], x0, x1, pNextSec);
		}
		if ( pCurSec->m_Walls[id].m_adjoin[1] != 0xffff )
		{
			pNextSec = (Sector_2_5D *)Sectors[pCurSec->m_Walls[id].m_adjoin[1]];
			//add the next buffer.
			if ( pNextSec )
			{
				if ( m_visStackCnt_VAdjoin < _MAX_RECURSION_LEVEL )
				{
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fL = worldPos[0];
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].fR = worldPos[1];
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uStartX = x0;
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].uEndX   = x1;
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].pNext = pNextSec;
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].bUsePortalClip = true;
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].offset = s_vCurrentOffs;
					m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth = &_VAdjoinBuffer[_uAdjoinBufferLoc];
					if ( _uAdjoinBufferLoc < _VADJOIN_BUFFER_SIZE-(uEndX-uStartX+2) )
					{
						memcpy( m_visStack_VAdjoin[m_visStackCnt_VAdjoin].depth, &fDepth[x0], sizeof(f32)*(x1-x0+1));
						_uAdjoinBufferLoc += (x1-x0+1);
						m_visStackCnt_VAdjoin++;
					}
				}
			}
		}
	}

	const f32 fOO255 = (1.0f/255.0f);
	f32 fI;

	s32 shade = Math::clamp( (s32)pCurSec->m_uAmbientFloor + pCurSec->m_aLightFX[0], 0, 255 );
	fI = fOO255 * (f32)shade;

	//now add objects to the list...
	vector<u32>::iterator iObj = pCurSec->m_Objects.begin();
	vector<u32>::iterator eObj = pCurSec->m_Objects.end();
	for (; iObj != eObj; ++iObj)
	{
		u32 uObjID = *iObj;
		Object *pObj = ObjectManager::GetObjectFromID(uObjID);
		if ( pObj->GetRenderKey() != _uRenderKey )
		{
			AddObjectToRender(pObj, fI, s_vCurrentOffs);
			pObj->SetRenderKey(_uRenderKey);
		}
	}

	/*
	if ( bUsePortalClip == false )
	{
		//in the case of vadjoins, restore the depth when we're done...
		for (u32 x=uStartX; x<=uEndX; x++)
		{
			fDepth[x] = fDepth_vAdjoin[x];
		}
	}
	*/

#if 0
	if ( pCurSec->m_vAdjoin[0] != 0xffff )
	{
		pNextSec = (Sector_2_5D *)Sectors[pCurSec->m_vAdjoin[0]];
		if ( cPos.z > pCurSec->m_ZRangeCur.x )
		{
			//add the next buffer.
			if ( pNextSec )
			{
				VisStack_Push(fL, fR, uStartX, uEndX, pNextSec, false, pCurSec->m_vAdjOffset[0]+s_vCurrentOffs);
			}
		}
	}
	if ( pCurSec->m_vAdjoin[1] != 0xffff )
	{
		pNextSec = (Sector_2_5D *)Sectors[pCurSec->m_vAdjoin[1]];
		if ( cPos.z < pCurSec->m_ZRangeCur.y )
		{
			//add the next buffer.
			if ( pNextSec )
			{
				VisStack_Push(fL, fR, uStartX, uEndX, pNextSec, false, pCurSec->m_vAdjOffset[1]+s_vCurrentOffs);
			}
		}
	}
#endif
}
