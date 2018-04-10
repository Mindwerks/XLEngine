#include "MeshLoader_Daggerfall.h"
#include "ArchiveManager.h"
#include "DFFaceTex.h"
#include "../memory/ScratchPad.h"
#include "../render/TextureCache.h"
#include "../render/IDriver3D.h"
#include "../fileformats/TextureTypes.h"
#include "../ui/XL_Console.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../math/Math.h"
#include "../render/VertexBuffer.h"
#include "../render/IndexBuffer.h"

#include <cassert>
#include <cfloat>
#include <cstdio>

#define STO_TERRAIN          2
#define STO_RUINS            7
#define STO_CASTLE           9
#define STO_CITYA           12
#define STO_CITYB           14
#define STO_CITYWALLS       17
#define STO_FARM            26
#define STO_FARM_INT        28
#define STO_FENCES          29
#define STO_MAGESGUILD      35
#define STO_MAGESGUILD_INT  37
#define STO_MANOR           38
#define STO_MANOR_INT       40
#define STO_MARBLE_FLOOR    41
#define STO_MERCHANTHOMES   42
#define STO_MERCHHOMES_INT  44
#define STO_PAINTINGS       48
#define STO_TAVERNEXTERIORS 58
#define STO_TAVERNINTERIORS 60
#define STO_TEMPLEEXTERIORS 61
#define STO_TEMPLEINTERIORS 63
#define STO_VILLAGE         64
#define STO_VILLAGE_INT     66
#define STO_ROOFS           69
#define STO_DOORS           74

MeshLoader_Daggerfall::MeshLoader_Daggerfall()
{
}

MeshLoader_Daggerfall::~MeshLoader_Daggerfall()
{
}

bool MeshLoader_Daggerfall::Load(IDriver3D *pDriver, Mesh *pMesh, MeshCollision *pMeshCol, char *ID, int region, int type)
{
    bool bSuccess = false;
    uint32_t uMeshID = (ID[0]-'0')*10000 + (ID[1]-'0')*1000 + (ID[2]-'0')*100 + (ID[3]-'0')*10 + (ID[4]-'0');

    if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "ARCH3D.BSA", uMeshID) )
    {
        ScratchPad::StartFrame();
        
        //Read out the file contents.
        uint32_t uLength = ArchiveManager::GameFile_GetLength();
        char *pData = (char *)ScratchPad::AllocMem( uLength );
        ArchiveManager::GameFile_Read(pData, uLength);
        ArchiveManager::GameFile_Close();

        //Build the mesh.
        bSuccess = LoadMesh(pDriver, pMesh, pMeshCol, pData, uLength, region, type);

        //Free memeory.
        ScratchPad::FreeFrame();
    }

    return bSuccess;
}

#pragma pack(push)
#pragma pack(1)

struct ObjectHeader
{
    char version[4];
    int nPointCount;
    int nPlaneCount;
    int nUnknown;
    long long nUnknown64;
    int nPlaneDataOffs;
    int nObjectDataOffs;
    int nObjectDataCnt;
    int nUnknown2;
    long long nUnknown64_2;
    int nPointListOffs;
    int NormalListOffs;
    int nUnknown3;
    int nPlaneListOffs;
};

struct IVec3
{
    int x, y, z;
};

struct PlaneHeader
{
    char nPlanePointCount;
    char nUnknown1;
    unsigned short uTexture;
    int nUnknown2;
};

struct PlanePoint
{
    int nPointOffset;
    short U;
    short V;
};

#pragma pack(pop)

struct SM_Polygon
{
    PlaneHeader header;
    PlanePoint *points;
    float *tu, *tv;
    TextureHandle hTex;
    short ImageIndex;
    short FileIndex;
    Vector2 vUVScale;
    Vector3 vNrml;

    Vector3 vLocalCen;
    Vector3 vBounds[2];
    float fLocalRadius;

    bool bDoor;
    bool bDungeonEntrance;
    int matIdx;
};

struct SM_Material
{
    TextureHandle hTex;
    std::vector<short> PolyList;

    short ImageIndex;
    short FileIndex;
};

struct MeshVtx
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

void MeshLoader_Daggerfall::BuildTextureName(char *pszTexName, int FileIndex)
{
    //Work arounds for missing records...
    //TEXTURE.036 records do not match TEXTURE.035, work around by setting back to 035
    if ( FileIndex ==  36 ) FileIndex =  35;
    //There is not TEXTURE.436, set back to 435 as a work around.
    if ( FileIndex == 436 ) FileIndex = 435;
    //There is no TEXTURE.441, set back to 440 as a work around.
    if ( FileIndex == 441 ) FileIndex = 440;

    //now we load the texture itself.
    assert(FileIndex>=0 && FileIndex<=1000);
    sprintf(pszTexName, "TEXTURE.%03d", FileIndex);
}

int AdjustTexForRegion(int nRegion, int dungeonType, int FileIndex)
{
    //hack to make textures match DOS.
    if ( dungeonType )
    {
        if ( (dungeonType&0xffff) == 1 )
        {
            if ( FileIndex == 122 )
                FileIndex = 19;
            else if ( FileIndex == 123 )
                FileIndex = 20;
            else if ( FileIndex == 124 )
                FileIndex = 20; //21 doesn't exist...
            else if ( FileIndex == 120 )
                FileIndex = 22;
            else if ( FileIndex == 168 )
                FileIndex = 368;
            else if ( FileIndex == 74 )
                FileIndex = 332;
        }
        else if ( (dungeonType&0xffff) == 2 )
        {
            if ( FileIndex == 122 )
                FileIndex = (dungeonType&0x10000) ? 23 : 19;
            else if ( FileIndex == 123 )
                FileIndex = 19;
            else if ( FileIndex == 124 )
                FileIndex = 19;
            else if ( FileIndex == 120 )
                FileIndex = 23;
            else if ( FileIndex == 168 )
                FileIndex = 368;
            else if ( FileIndex == 74 )
                FileIndex = 332;
        }
        else if ( (dungeonType&0xffff) == 3 )
        {
            if ( FileIndex == 122 )
                FileIndex = (dungeonType&0x10000) ? 23 : 22;
            else if ( FileIndex == 123 )
                FileIndex = 19;
            else if ( FileIndex == 124 )
                FileIndex = 23;
            else if ( FileIndex == 120 )
                FileIndex = 23;
            else if ( FileIndex == 168 )
                FileIndex = 368;
            else if ( FileIndex == 74 )
                FileIndex = 332;
        }
    }
    else
    {
        // Determine whether to apply climate offset
        int nClimate = 0;
        //if ( WEATHER_WINTER == m_dwWeather && TERRAIN_DESERT != nRegion ) 
        //{
        //  nClimate = 1;
        //}

        // Find base index index of this texture
        int nBase = FileIndex / 100;
        int nTexOffs = 0;
        /*
        if ( World::GetEnvType() == World::ENV_TYPE_DUNGEON )
        {
            if ( nBase*100 == nRegion )
                nRegion = 0;
        }
        */

        // Process region against texture archive
        int nIndex = FileIndex - (nBase * 100);
        switch ( nIndex )
        {
            case STO_TERRAIN:
            case STO_RUINS:
            case STO_CASTLE:
            case STO_CITYA:
            case STO_CITYB:
            case STO_CITYWALLS:
            case STO_FARM:
            case STO_FENCES:
            case STO_MAGESGUILD:
            case STO_MANOR:
            case STO_MARBLE_FLOOR:
            case STO_MERCHANTHOMES:
            case STO_TAVERNEXTERIORS:
            case STO_TEMPLEEXTERIORS:
            case STO_VILLAGE:
            case STO_ROOFS:
                //if ( World::GetEnvType() != World::ENV_TYPE_DUNGEON )
                    FileIndex = nIndex + nRegion + nClimate;
                break;

            case STO_FARM_INT:
            case STO_MAGESGUILD_INT:
            case STO_MANOR_INT:
            case STO_MERCHHOMES_INT:
            case STO_PAINTINGS:
            case STO_TAVERNINTERIORS:
            case STO_TEMPLEINTERIORS:
            case STO_VILLAGE_INT:
            case STO_DOORS:
                //if ( nIndex == STO_DOORS || World::GetEnvType() != World::ENV_TYPE_DUNGEON )
                    FileIndex = nIndex + nRegion + nClimate;
                break;

            //case 8:
            case 19:
            case 20:
            case 22:
            case 23:
            case 24:
            case 25:
            //case 45:
            //case 46:
            //case 47:
                if ( nTexOffs == 0 )
                {
                    if ( nIndex == 19 || nIndex == 20 )
                    {
                        if ( dungeonType == 0 )
                            nIndex = 19;
                        else
                            nIndex = 20;
                    }
                    else if ( nIndex >= 22 && nIndex <= 24 )
                    {
                        nIndex = 22 + dungeonType;
                    }
                    FileIndex = nIndex + nRegion;
                }
                if ( FileIndex >= 219 && FileIndex <= 225 )
                    FileIndex -= 200;
                break;

            default:
                break;
        };
    }
    return FileIndex;
}

bool MeshLoader_Daggerfall::LoadMesh(IDriver3D *pDriver, Mesh *pMesh, MeshCollision *pMeshCol, char *pData, uint32_t uLength, int region, int type)
{
    ObjectHeader header = *(ObjectHeader *)pData;
    Vector3 *points = new Vector3[ header.nPointCount ];
    bool bHasDoor = false;
    bool bHasDungeonEntrance = false;

    int nHeaderSize = sizeof(ObjectHeader);

    const int v2_7 = 0, v2_6 = 1, v2_5 = 2;
    int version;

         if ( header.version[3] == '7' ) version = v2_7;
    else if ( header.version[3] == '6' ) version = v2_6;
    else if ( header.version[3] == '5' ) version = v2_5;
    else { assert(0); }

    int index = header.nPointListOffs;
    float fFP_Scale = 1.0f / 1024.0f;

    pMesh->m_Bounds[0].Set(FLT_MAX, FLT_MAX, FLT_MAX);
    pMesh->m_Bounds[1] = -pMesh->m_Bounds[0];

    for (int i=0; i<header.nPointCount; i++)
    {
        IVec3 ivec = *( (IVec3 *)&pData[index] );

        points[i].x =  (float)ivec.x * fFP_Scale;
        points[i].z = -(float)ivec.y * fFP_Scale;
        points[i].y =  (float)ivec.z * fFP_Scale;

        if ( points[i].x < pMesh->m_Bounds[0].x ) pMesh->m_Bounds[0].x = points[i].x;
        if ( points[i].y < pMesh->m_Bounds[0].y ) pMesh->m_Bounds[0].y = points[i].y;
        if ( points[i].z < pMesh->m_Bounds[0].z ) pMesh->m_Bounds[0].z = points[i].z;

        if ( points[i].x > pMesh->m_Bounds[1].x ) pMesh->m_Bounds[1].x = points[i].x;
        if ( points[i].y > pMesh->m_Bounds[1].y ) pMesh->m_Bounds[1].y = points[i].y;
        if ( points[i].z > pMesh->m_Bounds[1].z ) pMesh->m_Bounds[1].z = points[i].z;

        index += sizeof(IVec3);
    }

    SM_Polygon *polygons = new SM_Polygon[header.nPlaneCount];
    index = header.nPlaneListOffs;

    pMeshCol->SetMaxPolygonCount( header.nPlaneCount );

    for (int i=0; i<header.nPlaneCount; i++)
    {
        //read the header.
        polygons[i].header = *( (PlaneHeader *)&pData[index] );
        index += sizeof(PlaneHeader);

        polygons[i].bDoor = false;
        polygons[i].bDungeonEntrance = false;

        if ( polygons[i].header.nPlanePointCount < 3 )
        {
            polygons[i].points = nullptr;
            polygons[i].header.nPlanePointCount = 0;
            continue;
        }

        uint32_t w, h;
        int32_t ox, oy;
        float fw, fh;
        if ( polygons[i].header.nPlanePointCount >= 3 )
        {
            polygons[i].points = new PlanePoint[ polygons[i].header.nPlanePointCount ];
            int ImageIndex = polygons[i].header.uTexture & 0x7f;
            int FileIndex  = polygons[i].header.uTexture >> 7;
            polygons[i].ImageIndex = ImageIndex;
            polygons[i].FileIndex = FileIndex;

            FileIndex = AdjustTexForRegion(region, type, FileIndex);
            /*if ( _IsDoorTex(FileIndex) )
            {
                polygons[i].bDoor = true;
                bHasDoor = true;
            }
            else if ( _IsDungeonEntrance(FileIndex) )
            {
                polygons[i].bDungeonEntrance = true;
                bHasDungeonEntrance = true;
            }*/

            char szTexName[128];
            BuildTextureName(szTexName, FileIndex);
            polygons[i].hTex = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", szTexName, ImageIndex );
            TextureCache::GetTextureSize(ox, oy, w, h, fw, fh);

            uint32_t wr = Math::RoundNextPow2(w), hr = Math::RoundNextPow2(h);
            if ( polygons[i].hTex == 0 ) 
            { 
                w = 64; h = 64; 
            }
            polygons[i].vUVScale.x = 1.0f / (16.0f * (float)w);
            polygons[i].vUVScale.y = 1.0f / (16.0f * (float)h);
        }
        else
        {
            polygons[i].points = nullptr;
            polygons[i].hTex = 0;
            w = 64; h = 64;
            polygons[i].vUVScale.x = 1.0f / (16.0f * (float)w);
            polygons[i].vUVScale.y = 1.0f / (16.0f * (float)h);
        }

        for (int j=0; j<polygons[i].header.nPlanePointCount; j++)
        {
            polygons[i].points[j] = *((PlanePoint *)&pData[index]);
            polygons[i].points[j].nPointOffset /= sizeof(IVec3);

            if (version == v2_5)
                polygons[i].points[j].nPointOffset /= 3;

            index += sizeof(PlanePoint);

            if ( polygons[i].points[j].nPointOffset >= header.nPointCount || polygons[i].points[j].nPointOffset < 0 )
            {
                //set to first point, will cause odd results but allow the mesh to load.
                if ( j > 0 )
                    polygons[i].points[j].nPointOffset = polygons[i].points[0].nPointOffset;
                else
                    polygons[i].points[j].nPointOffset = 0;
            }
        }

        int s, t;
        polygons[i].vBounds[0].Set(FLT_MAX, FLT_MAX, FLT_MAX);
        polygons[i].vBounds[1] = -polygons[i].vBounds[0];

        for (int j=0; j<polygons[i].header.nPlanePointCount; j++)
        {
            if ( j == 0 )      { s  = polygons[i].points[j].U; t  = polygons[i].points[j].V; }
            else if ( j < 3 )  { s += polygons[i].points[j].U; t += polygons[i].points[j].V; }
            else if ( j == 3 ) { s  = polygons[i].points[j].U; t  = polygons[i].points[j].V; }

            polygons[i].points[j].U = s;
            polygons[i].points[j].V = t;

            Vector3& vtx = points[ polygons[i].points[j].nPointOffset ];
            if ( vtx.x < polygons[i].vBounds[0].x ) polygons[i].vBounds[0].x = vtx.x;
            if ( vtx.y < polygons[i].vBounds[0].y ) polygons[i].vBounds[0].y = vtx.y;
            if ( vtx.z < polygons[i].vBounds[0].z ) polygons[i].vBounds[0].z = vtx.z;

            if ( vtx.x > polygons[i].vBounds[1].x ) polygons[i].vBounds[1].x = vtx.x;
            if ( vtx.y > polygons[i].vBounds[1].y ) polygons[i].vBounds[1].y = vtx.y;
            if ( vtx.z > polygons[i].vBounds[1].z ) polygons[i].vBounds[1].z = vtx.z;
        }
        polygons[i].vLocalCen    = ( polygons[i].vBounds[0] + polygons[i].vBounds[1] ) * 0.5f;
        polygons[i].fLocalRadius = ( polygons[i].vBounds[1] - polygons[i].vLocalCen ).Length();

        /*if ( polygons[i].bDungeonEntrance )
        {
            m_vDungeonEntrance = polygons[i].vLocalCen;
            m_vDungeonEntrance.z = vPolyMin.z;
        }*/

        polygons[i].tu = new float[polygons[i].header.nPlanePointCount];
        polygons[i].tv = new float[polygons[i].header.nPlanePointCount];

        if ( polygons[i].header.nPlanePointCount > 3 )
        {
            DFFaceTex dft;
            ObjVertex uvVtx[4];
            for (int j=0; j<4; j++)
            {
                float x =  points[ polygons[i].points[j].nPointOffset ].x;
                float y = -points[ polygons[i].points[j].nPointOffset ].z;
                float z =  points[ polygons[i].points[j].nPointOffset ].y;
                uvVtx[j].pos.Set(x,y,z);
                uvVtx[j].tu = (float)polygons[i].points[j].U;
                uvVtx[j].tv = (float)polygons[i].points[j].V;
            }
            df3duvmatrix mat;
            if ( dft.ComputeDFFaceTextureUVMatrix( mat, uvVtx ) )
            {
                for (int j=0; j<polygons[i].header.nPlanePointCount; j++)
                {
                    float x =  points[ polygons[i].points[j].nPointOffset ].x;
                    float y = -points[ polygons[i].points[j].nPointOffset ].z;
                    float z =  points[ polygons[i].points[j].nPointOffset ].y;
                    float tu = (x*mat.UA) + (y*mat.UB) + (z*mat.UC) + mat.UD;
                    float tv = (x*mat.VA) + (y*mat.VB) + (z*mat.VC) + mat.VD;

                    polygons[i].tu[j] = tu*polygons[i].vUVScale.x;
                    polygons[i].tv[j] = tv*polygons[i].vUVScale.y;
                }
                if ( polygons[i].header.nPlanePointCount )
                {
                    int nv = polygons[i].header.nPlanePointCount;
                    Vector2 uvMin(FLT_MAX, FLT_MAX), uvMax(-FLT_MAX,-FLT_MAX);
                    for (int j=0; j<nv; j++)
                    {
                        if ( polygons[i].tu[j] < uvMin.x ) uvMin.x = polygons[i].tu[j];
                        if ( polygons[i].tu[j] > uvMax.x ) uvMax.x = polygons[i].tu[j];

                        if ( polygons[i].tv[j] < uvMin.y ) uvMin.y = polygons[i].tv[j];
                        if ( polygons[i].tv[j] > uvMax.y ) uvMax.y = polygons[i].tv[j];
                    }
                    float du = uvMax.x - uvMin.x;
                    float dv = uvMax.y - uvMin.y;
                    if ( (du/dv) > 5.0f || (dv/du) > 5.0f ) //the aspect ratio is too big...
                    {
                        //now let's look at the geometry of the polygon.
                        Vector3 vMinGeo(FLT_MAX, FLT_MAX, FLT_MAX), vMaxGeo(-FLT_MAX, -FLT_MAX, -FLT_MAX);
                        for (int j=0; j<polygons[i].header.nPlanePointCount; j++)
                        {
                            float x = points[ polygons[i].points[j].nPointOffset ].x;
                            float y = points[ polygons[i].points[j].nPointOffset ].y;
                            float z = points[ polygons[i].points[j].nPointOffset ].z;
                            if ( x < vMinGeo.x ) vMinGeo.x = x;
                            if ( x > vMaxGeo.x ) vMaxGeo.x = x;

                            if ( y < vMinGeo.y ) vMinGeo.y = y;
                            if ( y > vMaxGeo.y ) vMaxGeo.y = y;

                            if ( z < vMinGeo.z ) vMinGeo.z = z;
                            if ( z > vMaxGeo.z ) vMaxGeo.z = z;
                        }
                        Vector3 delta = vMaxGeo - vMinGeo;
                        if ( delta.z > 1.0f )
                        {
                            float dx = sqrtf( delta.x*delta.x + delta.y*delta.y );
                            float dy = delta.z;
                            //is the aspect ratio still off?
                            if ( dv > 0.0f && (du/dv) > 2.0f*(dx/dy) && du > 13.9f && du < 14.1f )
                            {
                                float fRescale = 1.0f/7.0f;
                                //ok now lets scale back the maximum...
                                for (int j=0; j<nv; j++)
                                {
                                    if ( polygons[i].tu[j] > uvMin.x )
                                    {
                                        polygons[i].tu[j] = uvMin.x + (polygons[i].tu[j]-uvMin.x)*fRescale;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                //if ( polygons[i].header.nPlanePointCount <= 4 )
                {
                    for (int j=0; j<polygons[i].header.nPlanePointCount; j++)
                    {
                        polygons[i].tu[j] = (float)polygons[i].points[j].U*polygons[i].vUVScale.x;
                        polygons[i].tv[j] = (float)polygons[i].points[j].V*polygons[i].vUVScale.y;
                    }
                    //check again...
                    if ( polygons[i].header.nPlanePointCount == 4 )
                    {
                        Vector2 uvMin, uvMax;
                        uvMin.Set(FLT_MAX, FLT_MAX); uvMax = -uvMin;
                        for (int j=0; j<4; j++)
                        {
                            if ( polygons[i].tu[j] < uvMin.x ) uvMin.x = polygons[i].tu[j];
                            if ( polygons[i].tu[j] > uvMax.x ) uvMax.x = polygons[i].tu[j];

                            if ( polygons[i].tv[j] < uvMin.y ) uvMin.y = polygons[i].tv[j];
                            if ( polygons[i].tv[j] > uvMax.y ) uvMax.y = polygons[i].tv[j];
                        }
                        float du = uvMax.x - uvMin.x;
                        float dv = uvMax.y - uvMin.y;
                        //is the aspect ratio still off?
                        if ( dv > 0.0f && (du/dv) > 5.0f )
                        {
                            //ok now lets scale back the maximum...
                            for (int j=0; j<4; j++)
                            {
                                if ( polygons[i].tu[j] > uvMax.x-0.01f*du )
                                {
                                    polygons[i].tu[j] = uvMin.x + dv;
                                }
                            }
                        }
                        else if ( du > 0.0f && (dv/du) > 5.0f )
                        {
                            //ok now lets scale back the maximum...
                            for (int j=0; j<4; j++)
                            {
                                if ( polygons[i].tv[j] == uvMax.y-0.01f*dv )
                                {
                                    polygons[i].tv[j] = uvMin.y + du;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            for (int j=0; j<polygons[i].header.nPlanePointCount; j++)
            {
                polygons[i].tu[j] = (float)polygons[i].points[j].U*polygons[i].vUVScale.x;
                polygons[i].tv[j] = (float)polygons[i].points[j].V*polygons[i].vUVScale.y;
            }
        }
    }

    Vector3 avPolygonPoints[64];
    index = header.NormalListOffs;
    for (int i=0; i<header.nPlaneCount; i++)
    {
        IVec3 ivec = *( (IVec3 *)&pData[index] );

        polygons[i].vNrml.x = -(float)ivec.x;
        polygons[i].vNrml.z =  (float)ivec.y;
        polygons[i].vNrml.y = -(float)ivec.z;
        polygons[i].vNrml.Normalize();

        //Add collision polygons
        for (int p=0; p<polygons[i].header.nPlanePointCount; p++)
        {
            avPolygonPoints[p] = points[ polygons[i].points[p].nPointOffset ];
        }
        pMeshCol->AddPolygon( polygons[i].header.nPlanePointCount, avPolygonPoints );

        /*if ( polygons[i].bDungeonEntrance )
        {
            m_vDENrml = polygons[i].vNrml;
        }*/

        index += sizeof(IVec3);
    }

    ////////////////////////////////////////////////////////////////
    // Build GPU buffers
    ////////////////////////////////////////////////////////////////
    /*******sort polygons based on texture...********/
    //first figure out how many different textures this model has...
    int32_t nTexCnt = 0;
    TextureHandle matTex[128];
    int16_t matImageIndex[128];
    int16_t matFileIndex[128];
    for (int i=0; i<header.nPlaneCount; i++)
    {
        bool bMatFound = false;
        for (int m=0; m<nTexCnt; m++)
        {
            if ( polygons[i].hTex == matTex[m] )
            {
                polygons[i].matIdx = m;
                bMatFound = true;
                break;
            }
        }
        if ( bMatFound == false )
        {
            polygons[i].matIdx       = nTexCnt;
            matImageIndex[ nTexCnt ] = polygons[i].ImageIndex;
            matFileIndex[ nTexCnt ]  = polygons[i].FileIndex;
            matTex[ nTexCnt++ ]      = polygons[i].hTex;
        }
    }
    assert( nTexCnt <= 128 );
    assert( nTexCnt >  0 );
    pMesh->m_pMaterials = xlNew Mesh::Material[nTexCnt];
    pMesh->m_nMtlCnt    = nTexCnt;
    for (int i=0; i<pMesh->m_nMtlCnt; i++)
    {
        pMesh->m_pMaterials[i].hTex = matTex[i];
        pMesh->m_pMaterials[i].vBounds[0].Set(FLT_MAX, FLT_MAX, FLT_MAX);
        pMesh->m_pMaterials[i].vBounds[1] = -pMesh->m_pMaterials[i].vBounds[0];

    }
    //then create a polygon list per material.
    std::vector<uint16_t> polyList[128];
    for (int i=0; i<header.nPlaneCount; i++)
    {
        int m = polygons[i].matIdx;
        polyList[m].push_back( (uint16_t)i );
        if ( polygons[i].vBounds[0].x < pMesh->m_pMaterials[m].vBounds[0].x ) pMesh->m_pMaterials[m].vBounds[0].x = polygons[i].vBounds[0].x;
        if ( polygons[i].vBounds[0].y < pMesh->m_pMaterials[m].vBounds[0].y ) pMesh->m_pMaterials[m].vBounds[0].y = polygons[i].vBounds[0].y;
        if ( polygons[i].vBounds[0].z < pMesh->m_pMaterials[m].vBounds[0].z ) pMesh->m_pMaterials[m].vBounds[0].z = polygons[i].vBounds[0].z;

        if ( polygons[i].vBounds[1].x > pMesh->m_pMaterials[m].vBounds[1].x ) pMesh->m_pMaterials[m].vBounds[1].x = polygons[i].vBounds[1].x;
        if ( polygons[i].vBounds[1].y > pMesh->m_pMaterials[m].vBounds[1].y ) pMesh->m_pMaterials[m].vBounds[1].y = polygons[i].vBounds[1].y;
        if ( polygons[i].vBounds[1].z > pMesh->m_pMaterials[m].vBounds[1].z ) pMesh->m_pMaterials[m].vBounds[1].z = polygons[i].vBounds[1].z;
    }

    static uint16_t _aIdxCache[32768];
    static MeshVtx _aVtxCache[32768];
    int nTotalVtx = 0, nTotalIdx = 0;
    for (int m=0; m<pMesh->m_nMtlCnt; m++)
    {
        pMesh->m_pMaterials[m].uIndexOffset = nTotalIdx;
        pMesh->m_pMaterials[m].uPrimCount   = 0;
        for (int i=0; i<(int)polyList[m].size(); i++)
        {
            uint16_t t = polyList[m].at(i);
            if ( polygons[t].header.nPlanePointCount < 3 )
                 continue;

            int v0 = nTotalVtx;
            for (int v=0; v<polygons[t].header.nPlanePointCount-2; v++)
            {
                _aIdxCache[nTotalIdx+0] =   v0;
                _aIdxCache[nTotalIdx+1] = v+v0+1;
                _aIdxCache[nTotalIdx+2] = v+v0+2;
                nTotalIdx += 3;
                pMesh->m_pMaterials[m].uPrimCount++;

                assert( nTotalIdx <= 32768 );
            }
            for (int v=0; v<polygons[t].header.nPlanePointCount; v++, nTotalVtx++)
            {
                int vi = polygons[t].points[v].nPointOffset;
                assert(vi >= 0 && vi < header.nPointCount);
                _aVtxCache[nTotalVtx].x  = points[vi].x;
                _aVtxCache[nTotalVtx].y  = points[vi].y;
                _aVtxCache[nTotalVtx].z  = points[vi].z;
                _aVtxCache[nTotalVtx].u  = polygons[t].tu[v];
                _aVtxCache[nTotalVtx].v  = polygons[t].tv[v];
                _aVtxCache[nTotalVtx].nx = polygons[t].vNrml.x;
                _aVtxCache[nTotalVtx].ny = polygons[t].vNrml.y;
                _aVtxCache[nTotalVtx].nz = polygons[t].vNrml.z;
                assert(nTotalVtx <= 32768);
            }
        }
        assert( nTotalIdx > 0 && (nTotalIdx == (nTotalIdx/3)*3) );
    }
    pMesh->m_pVB = xlNew VertexBuffer(pDriver);
    pMesh->m_pVB->Create( sizeof(MeshVtx), nTotalVtx, false, IDriver3D::VBO_HAS_NORMALS | IDriver3D::VBO_HAS_TEXCOORDS );
    pMesh->m_pVB->Fill(_aVtxCache);

    pMesh->m_pIB = xlNew IndexBuffer(pDriver);
    pMesh->m_pIB->Create(nTotalIdx, sizeof(uint16_t), false);
    pMesh->m_pIB->Fill((uint32_t *)_aIdxCache);

    return true;
}
