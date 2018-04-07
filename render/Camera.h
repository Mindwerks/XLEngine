#ifndef CAMERA_H
#define CAMERA_H

#include "../Engine.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../math/Matrix.h"
#include "../math/Plane.h"

class IDriver3D;

class Camera
{
	public:
		enum
		{
			FRUSTUM_IN=0,
			FRUSTUM_OUT,
			FRUSTUM_INTERSECT
		};

    public:
        Camera();
        virtual ~Camera();

		void SetLoc(Vector3& loc);
		const Vector3& GetLoc() { return m_vLoc; }

		void SetWorldPos(int32_t x, int32_t y)
		{
			m_worldPosX = x;
			m_worldPosY = y;
		}
		int32_t GetWorldPosX() { return m_worldPosX; }
		int32_t GetWorldPosY() { return m_worldPosY; }

		void SetDir(Vector3& dir);
		const Vector3& GetDir() { return m_vDir; }

		void SetFOV(float fovInDegrees, float aspect);
		void SetZRange(float fNearZ, float fFarZ);
		float GetNearZ() { return m_fNearZ; }
		float GetFarZ()  { return m_fFarZ; }

		void SetSkew(float fSkew) { if ( m_fSkew != fSkew ) m_bComputeProjMtx = true; m_fSkew = fSkew; }
		float GetSkew() { return m_fSkew; }

		void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
		float GetSpeed() { return m_fSpeed; }

		void SetSector(uint16_t uSector) { m_uSector = uSector; }
		uint16_t GetSector() { return m_uSector; }

		void Set(IDriver3D *pDriver3D, bool bCompute=true);
		void GetViewMatrix(Matrix *pViewMtx);
		void GetProjMatrix(Matrix *pProjMtx);
		void Compute(float fMaxZCos=1.0f, bool bForce=false);

		void Update(float fDeltaTime);
		float GetFrustumWidth() { return m_fFrustumWidth; }

		void TransformPointsSS(uint32_t uCount, Vector3 *wsPos, Vector3 *ssPos);
		void TransformPointsSS_2D(uint32_t uCount, const Vector2 *wsPos, Vector2 *ssPos, const Vector2& offset);
		void InverseTransformPointsSS_2D(float x, float oow, Vector2& worldPos);

		int SphereInsideFrustum(Vector3& vCen, float fRadius);
		int AABBInsideFrustum(Vector3& vMin, Vector3& vMax, int32_t worldX, int32_t worldY);

		void SetMaxRenderDistance(float fDist) { m_fMaxRenderDist = fDist; }

		static void EnableCameraUpdating(bool bEnable) { s_bUpdateFrustum = bEnable; }

    protected:
		Plane m_FrustumPlanes[4];

		Matrix m_ViewMtx;
		Matrix m_WorldMtx;
		Matrix m_ProjMtx;
		Matrix m_ViewProj;

		Vector3 m_vLoc;
		Vector3 m_vDir;
		Vector3 m_vCullDir;
		Vector3 m_vCullLoc;

		float m_fFOV;		//FOV in radians.
		float m_fAspect;
		float m_fNearZ;
		float m_fFarZ;
		float m_fFrustumWidth;
		float m_fSkew;
		float m_fSpeed;	//how fast is the camera moving? Can be used for things like motion blur.
		float m_fMaxRenderDist;

		bool m_bComputeViewMtx;
		bool m_bComputeProjMtx;

		uint16_t m_uSector;
		uint16_t m_uPad;

		int32_t m_worldPosX;
		int32_t m_worldPosY;

		static bool s_bUpdateFrustum;

		void InitUpdateVars();
		void BuildWorldSpaceFrustumPlanes(Plane *planes);
    private:
};

#endif // CAMERA_H
