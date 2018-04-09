#ifndef WORLDCELL_H
#define WORLDCELL_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"
#include "../render/Camera.h"

#include <vector>

class Sector;
class IDriver3D;
class Camera;

class WorldCell
{
public:
    WorldCell();
    ~WorldCell();

    void SetBounds(const Vector3& rvMin, const Vector3& rvMax);
    void GetBounds(Vector3& rvMin, Vector3& rvMax);

    uint32_t GetSectorCount();
    void AddSector(Sector *pSector);
    Sector *GetSector(uint32_t uIndex);
    const std::vector<Sector *>& GetSectors() { return m_Sectors; }
    int32_t FindSector(const char *pszName);

    void SetWorldPos(int32_t x, int32_t y) { m_nWorldPosX = x; m_nWorldPosY = y; }
    void GetWorldPos(int32_t& x, int32_t& y) { x = m_nWorldPosX; y = m_nWorldPosY; }

    void Render(IDriver3D *pDriver, Camera *pCamera, uint32_t uSectorTypeVis);
    void Update(Camera *pCamera, float dt, uint32_t uSectorTypeVis);

    void LockCamera(Camera *pCamera, bool bLock);
    void Collide(Vector3 *p0, Vector3 *p1, uint32_t& uSector, float fRadius, uint32_t uSectorTypeVis, bool bPassThruAdjoins, int32_t worldX, int32_t worldY);
    void RayCastAndActivate(Vector3 *p0, Vector3 *p1, uint32_t& uSector, uint32_t uSectorTypeVis, int32_t worldX, int32_t worldY);
    bool Raycast(Vector3 *p0, Vector3 *p1, Vector3 *pInter, uint32_t uSectorTypeVis, int32_t worldX, int32_t worldY);

    void SetSkyTex(uint32_t uSkyIndex, TextureHandle hTex, uint32_t uWidth, uint32_t uHeight);
    TextureHandle GetSkyTex(uint32_t uSkyIndex) { return m_aSkyTex[uSkyIndex]; }
    uint32_t GetSkyTexCount() { return m_uSkyTexCnt; }
    void SetSkyTexCount(uint32_t uSkyTexCnt) { m_uSkyTexCnt = uSkyTexCnt; }

    void SetStartLoc(const Vector3& vStartLoc) { m_vStartLoc = vStartLoc; }
    void GetStartLoc(Vector3& vStartLoc) { vStartLoc = m_vStartLoc; }

    //type of cell - is it a town, graveyard, dungeon, etc.
    int  GetType() { return m_nType; }

protected:
    Vector3 m_Bounds[2];    //world space bounds [min,max]
    std::vector<Sector *> m_Sectors;
    int32_t m_nWorldPosX, m_nWorldPosY;
    int32_t m_nType;
    Camera m_LockCamera;
    bool m_bLockCamera;

    TextureHandle m_aSkyTex[16];
    uint32_t m_uSkyWidth, m_uSkyHeight;
    uint32_t m_uSkyTexCnt;
    int32_t m_nCollisionIter;
    Vector3 m_vStartLoc;

    void Render_Sectors_25D(IDriver3D *pDriver, Camera *pCamera, Sector *pStart);
    void Collide_Recursive(Vector3 *p0, Vector3 *p1, uint32_t& uSector, float fRadius, uint32_t uSectorTypeVis, bool bPassThruAdjoins, int32_t worldX, int32_t worldY);
};

#endif //WORLDCELL_H