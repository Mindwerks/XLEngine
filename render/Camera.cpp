#include "Camera.h"
#include "IDriver3D.h"
#include "../os/Input.h"
#include "../math/Math.h"
#include "../world/Sector.h"

const f32 dtor = 0.0174532925f;
bool Camera::s_bUpdateFrustum=true;

Camera::Camera()
{
	m_bComputeViewMtx = true;
	m_bComputeProjMtx = true;

	m_vLoc.Set(0.0f, 0.0f, 0.0f);
	m_vDir.Set(0.0f, 1.0f, 0.0f);

	m_fFOV    = 65.0f * dtor;		//FOV in radians.
	m_fAspect = 1.0f;
	m_fNearZ  = 0.1f;
	m_fFarZ   = 1000.0f;
	m_fFrustumWidth = 1.0f;
	m_fSkew = 0.0f;
	m_fMaxRenderDist = 400.0f;

	m_worldPosX = 0;
	m_worldPosY = 0;

	m_uSector = INVALID_SECTOR;
}

Camera::~Camera()
{
}

void Camera::SetLoc(Vector3& loc)
{
	m_vLoc = loc;
	m_bComputeViewMtx = true;
}

void Camera::SetDir(Vector3& dir)
{
	m_vDir = dir;
	m_bComputeViewMtx = true;
}

void Camera::SetFOV(float fovInDegrees, float aspect)
{
	m_fFOV = fovInDegrees*dtor;
	m_fAspect = aspect;
	m_bComputeProjMtx = true;
}

void Camera::SetZRange(float fNearZ, float fFarZ)
{
	m_fNearZ = fNearZ;
	m_fFarZ  = fFarZ;
	m_bComputeProjMtx = true;
}

void Camera::Compute(float fMaxZCos, bool bForce)
{
	bool bComputeViewProj = false;
	if ( m_bComputeViewMtx || bForce )
	{
		Vector3 up(0.0f, 0.0f, 1.0f);
		Vector3 dir = m_vDir;
		while ( fMaxZCos < 1.0f && dir.z > fMaxZCos )
		{
			dir.z  = fMaxZCos;
			dir.Normalize();
		}
		Vector3 at = m_vLoc + dir;
		m_ViewMtx.LookAt( m_vLoc, at, up );
		m_bComputeViewMtx = false;
		bComputeViewProj = true;
	}

	if ( m_bComputeProjMtx )
	{
		m_ProjMtx.ProjPersp( m_fFOV, m_fAspect, m_fNearZ, m_fFarZ, m_fSkew );
		m_fFrustumWidth = m_fNearZ*tanf(m_fFOV)*m_fAspect;
		m_bComputeProjMtx = false;
		bComputeViewProj = true;
	}

	if ( bComputeViewProj )
	{
		m_ViewProj = m_ProjMtx.MatMul(m_ViewMtx);
		if ( s_bUpdateFrustum )
		{
			BuildWorldSpaceFrustumPlanes(m_FrustumPlanes);
		}
	}
}

void Camera::Set(IDriver3D *pDriver3D, bool bCompute)
{
	if ( bCompute )
	{
		Compute();
	}

	pDriver3D->SetViewMatrix( &m_ViewMtx, &m_vLoc, &m_vDir );
	pDriver3D->SetProjMtx( &m_ProjMtx );

	//used for per-polygon culling in software.
	pDriver3D->SetCamera(this);
}

void Camera::BuildWorldSpaceFrustumPlanes(Plane *planes)
{
	//Left clipping plane
	planes[0].a = m_ViewProj.m[3]  + m_ViewProj.m[0];
	planes[0].b = m_ViewProj.m[7]  + m_ViewProj.m[4];
	planes[0].c = m_ViewProj.m[11] + m_ViewProj.m[8];
	planes[0].d = m_ViewProj.m[15] + m_ViewProj.m[12];
	//Right clipping plane
	planes[1].a = m_ViewProj.m[3]  - m_ViewProj.m[0];
	planes[1].b = m_ViewProj.m[7]  - m_ViewProj.m[4];
	planes[1].c = m_ViewProj.m[11] - m_ViewProj.m[8];
	planes[1].d = m_ViewProj.m[15] - m_ViewProj.m[12];
	//Top clipping plane
	planes[2].a = m_ViewProj.m[3]  - m_ViewProj.m[1];
	planes[2].b = m_ViewProj.m[7]  - m_ViewProj.m[5];
	planes[2].c = m_ViewProj.m[11] - m_ViewProj.m[9];
	planes[2].d = m_ViewProj.m[15] - m_ViewProj.m[13];
	//Bottom clipping plane
	planes[3].a = m_ViewProj.m[3]  + m_ViewProj.m[1];
	planes[3].b = m_ViewProj.m[7]  + m_ViewProj.m[5];
	planes[3].c = m_ViewProj.m[11] + m_ViewProj.m[9];
	planes[3].d = m_ViewProj.m[15] + m_ViewProj.m[13];

	planes[0].Normalize();
	planes[1].Normalize();
	planes[2].Normalize();
	planes[3].Normalize();

	m_vCullDir = m_vDir;
	m_vCullLoc = m_vLoc;
}

int Camera::SphereInsideFrustum(Vector3& vCen, float fRadius)
{
	float fDistance;
	int i, np;

	Plane *p = m_FrustumPlanes;
	np = 4;

	float zDist = m_vCullDir.Dot(vCen - m_vCullLoc);
	if ( zDist < -fRadius || zDist > m_fMaxRenderDist+fRadius )
		return FRUSTUM_OUT;

	// calculate the distance to each of the planes.
	for (i=0; i<np; i++)
	{
		//find the distance to this plane
		fDistance = p[i].Distance(vCen);
		//if the distance is < -radius, then it's outside.
		if ( fDistance < -fRadius )
		{
			return FRUSTUM_OUT;
		}
		//else if the distance is between +/- radius, then it intersects.
		if ( fabsf(fDistance) < fRadius )
		{
			return FRUSTUM_INTERSECT;
		}
	}

#if 0
	if ( -vCen.z + fRadius < 0.0f ) 
	{
		return FRUSTUM_OUT;
	}
	else if ( -vCen.z - fRadius < 0.0f )
	{
		return FRUSTUM_INTERSECT;
	}
#endif

	return FRUSTUM_IN;
}

#define SIGN(x) (x)>0 ? 1.0f : (x)<0 ? -1.0f : 0.0f

int Camera::AABBInsideFrustum(Vector3& vMin, Vector3& vMax, int32_t worldX, int32_t worldY)
{
	Plane *p = m_FrustumPlanes;
	const int np = 4;

	Vector3 vCen = (vMin + vMax)*0.5f;
	Vector3 vExt =  vMax - vCen;

	float fOffsetX = (worldX - m_worldPosX)*1024.0f;
	float fOffsetY = (worldY - m_worldPosY)*1024.0f;
	vCen.x += fOffsetX;
	vCen.y += fOffsetY;

	Vector3 signFlip(SIGN(m_vCullDir.x), SIGN(m_vCullDir.y), SIGN(m_vCullDir.z));
	float zDist = m_vCullDir.Dot( vCen - m_vCullLoc + (vExt*signFlip) );
	if ( zDist < 0.0f )
		return FRUSTUM_OUT;

	zDist = m_vCullDir.Dot( vCen - m_vCullLoc - (vExt*signFlip) );
	if ( zDist > m_fMaxRenderDist )
		return FRUSTUM_OUT;

	int result = FRUSTUM_INTERSECT;
	for (int i=0; i<np; i++)
	{
		Vector3 signFlip(SIGN(p[i].a), SIGN(p[i].b), SIGN(p[i].c));
        Vector3 arg = vCen + vExt*signFlip;

		if ( p[i].Dot(arg) < -p[i].d )
		{
			result = FRUSTUM_OUT;
			break;
		}
	}

	return result;
}

void Camera::GetViewMatrix(Matrix *pViewMtx)
{
	if ( m_bComputeViewMtx )
	{
		Vector3 up(0.0f, 0.0f, 1.0f);
		Vector3 at = m_vLoc + m_vDir;
		m_ViewMtx.LookAt( m_vLoc, at, up );
		m_bComputeViewMtx = false;
	}

	*pViewMtx = m_ViewMtx;
}

void Camera::GetProjMatrix(Matrix *pProjMtx)
{
	if ( m_bComputeProjMtx )
	{
		m_ProjMtx.ProjPersp( m_fFOV, m_fAspect, m_fNearZ, m_fFarZ, m_fSkew );
		m_bComputeProjMtx = false;
	}

	*pProjMtx = m_ProjMtx;
}

void Camera::Update(f32 fDeltaTime)
{
}

//for forward projecting cameras... i.e. the up = (0,0,1) exactly.
void Camera::InverseTransformPointsSS_2D(f32 x, f32 oow, Vector2& worldPos)
{
	const Vector3 xT( m_ViewProj.m[0], m_ViewProj.m[4], m_ViewProj.m[12] );
	const Vector3 wT( m_ViewProj.m[3], m_ViewProj.m[7], m_ViewProj.m[15] );

	if ( oow > 0.0f )
	{
		f32 w = 1.0f/oow;
		f32 fDenom = ( (xT.x*oow)*wT.y - (xT.y*oow)*wT.x );
		assert( Math::abs(fDenom) > 0.0000001f );
		worldPos.x = ( (x - xT.z*oow)*wT.y - (w - wT.z)*(xT.y*oow) ) / fDenom;
		if ( Math::abs(xT.y) > 0.00001f )
			worldPos.y = (x - xT.z*oow - worldPos.x*xT.x*oow) * (w / xT.y);
		else
			worldPos.y = ( w - worldPos.x*wT.x - wT.z ) / wT.y;
	}
	else
	{
		worldPos.Set(0,0);
	}
}

void Camera::TransformPointsSS_2D(uint32_t uCount, const Vector2 *wsPos, Vector2 *ssPos, const Vector2& offset)
{
	const Vector3 xT( m_ViewProj.m[0], m_ViewProj.m[4], m_ViewProj.m[12] );
	const Vector3 wT( m_ViewProj.m[3], m_ViewProj.m[7], m_ViewProj.m[15] );

	f32 fOODepthRange = 1.0f / (m_fFarZ - m_fNearZ);
	for (uint32_t i=0; i<uCount; i++)
	{
		ssPos[i].x = xT.x * (wsPos[i].x+offset.x) + xT.y * (wsPos[i].y+offset.y) + xT.z;
		f32 w = wT.x * (wsPos[i].x+offset.x) + wT.y * (wsPos[i].y+offset.y) + wT.z;
		if ( fabsf(w) > 0.0000001f )
		{
			f32 fOOW = 1.0f / fabsf(w);
			//projected 1D x position.
			ssPos[i].x *= fOOW;
			//linear depth value, ranges from 0.0 at nearZ to 1.0 at FarZ
			ssPos[i].y  = 1.0f/w;//(w - m_fNearZ) * fOODepthRange;
		}
		else
		{
			ssPos[i].Set(0, 0);
		}
	}
}

void Camera::TransformPointsSS(uint32_t uCount, Vector3 *wsPos, Vector3 *ssPos)
{
	for (uint32_t i=0; i<uCount; i++)
	{
		Vector4 in(wsPos[i].x, wsPos[i].y, wsPos[i].z, 1.0f);
		Vector4 out = m_ViewProj.TransformVector(in);
		if ( fabsf(out.w) > 0.0f )
		{
			f32 fOOW = 1.0f/out.w;
			ssPos[i].x = out.x / out.w;
			ssPos[i].y = out.y / out.w;
			ssPos[i].z = out.z / out.w;
		}
	}
}
