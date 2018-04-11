#include "RenderQue.h"
#include "IDriver3D.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <memory.h>
#include <cstdio>
#include <algorithm>

IDriver3D *RenderQue::m_pDriver;
RenderQuad *RenderQue::m_pQuads=nullptr;
uint32_t RenderQue::m_uQuadCnt;

std::vector<MaterialEntry *> RenderQue::m_apRenderBuckets[RBUCKET_COUNT];
std::vector<MaterialEntry>   RenderQue::m_RenderEntryPool;
uint32_t RenderQue::m_uRenderEntryLoc;

int RenderQue::m_nCurLightCnt;
const LightObject **RenderQue::m_pCurLightList;

#define MAX_QUADS 16384

bool RenderQue::Init(IDriver3D *pDriver)
{
    m_pDriver = pDriver;
    m_pQuads = xlNew RenderQuad[MAX_QUADS];
    Reset();

    m_RenderEntryPool.reserve(32768);//2048);

    return (m_pQuads) ? true : false;
}

void RenderQue::Destroy()
{
    if ( m_pQuads )
    {
        xlDelete [] m_pQuads;
    }
    m_pQuads = nullptr;

    m_RenderEntryPool.clear();
}

void RenderQue::Reset()
{
    m_uQuadCnt = 0;
    m_uRenderEntryLoc = 0;
    for (int r=0; r<RBUCKET_COUNT; r++)
    {
        m_apRenderBuckets[r].clear();
    }
}

RenderQuad *RenderQue::GetRenderQuad()
{
    RenderQuad *pQuad = nullptr;
    if ( m_uQuadCnt < MAX_QUADS )
    {
        pQuad = &m_pQuads[m_uQuadCnt];

        pQuad->nLightCnt  = m_nCurLightCnt;
        pQuad->pLightList = m_pCurLightList;

        m_uQuadCnt++;
    }

    return pQuad;
}

void RenderQue::AddQuad(TextureHandle hTex, Vector3 *posList, Vector2 *uvList, const Vector4& color, bool bForceZWrite)
{
    if ( m_uQuadCnt < MAX_QUADS )
    {
        m_pQuads[ m_uQuadCnt ].hTex = hTex;
        memcpy(m_pQuads[ m_uQuadCnt ].posList, posList, sizeof(Vector3)*4);
        memcpy(m_pQuads[ m_uQuadCnt ].uvList, uvList, sizeof(Vector2)*4);
        m_pQuads[ m_uQuadCnt ].color = color;
        m_pQuads[ m_uQuadCnt ].bForceZWrite = bForceZWrite;

        m_uQuadCnt++;
    }
}

MaterialEntry *RenderQue::GetEntry(RenderBuckets bucket)
{
    MaterialEntry *pEntry=nullptr;
    if ( m_uRenderEntryLoc < m_RenderEntryPool.size() )
    {
        pEntry = &m_RenderEntryPool[ m_uRenderEntryLoc ];
    }
    else
    {
        MaterialEntry new_entry;
        m_RenderEntryPool.push_back( new_entry );
        pEntry = &m_RenderEntryPool[ m_uRenderEntryLoc ];
    }

    pEntry->worldX = 0;
    pEntry->worldY = 0;

    pEntry->nLightCnt  = m_nCurLightCnt;
    pEntry->pLightList = m_pCurLightList;

    m_uRenderEntryLoc++;
    m_apRenderBuckets[bucket].push_back(pEntry);

    return pEntry;
}

void RenderQue::Render()
{
    if ( m_uQuadCnt < 1 && m_apRenderBuckets[RBUCKET_OPAQUE].empty() )
        return;

    //Render Material Entries.
    //****RBUCKET_OPAQUE****
    if ( !m_apRenderBuckets[RBUCKET_OPAQUE].empty() )
    {
        //sort based on texture, then vertex buffer 
        if ( m_pDriver->ApplyOpaqueSort() )
        {
            std::sort( m_apRenderBuckets[RBUCKET_OPAQUE].begin(), m_apRenderBuckets[RBUCKET_OPAQUE].end(), SortCB_Opaque );
        }
        //set opaque bucket states.
        m_pDriver->SetBlendMode(IDriver3D::BLEND_NONE);
        m_pDriver->EnableAlphaTest(false);
        m_pDriver->SetColor();
        m_pDriver->EnableCulling(true);
        m_pDriver->SetWorldMatrix(nullptr, 0, 0);
        //go through the list.
        for (const MaterialEntry *pEntry : m_apRenderBuckets[RBUCKET_OPAQUE])
        {
            //Set the world matrix.
            m_pDriver->SetWorldMatrix(pEntry->mWorld, pEntry->worldX, pEntry->worldY);
            //Set the texture.
            m_pDriver->SetTexture(0, pEntry->hTex);
            //Set the lights.
            m_pDriver->SetLights(pEntry->nLightCnt, pEntry->pLightList);
            //Set the Vertex Buffer.
            pEntry->pVB->Set();
            //Render the entry.
            m_pDriver->RenderIndexedTriangles( pEntry->pIB, pEntry->primCount, pEntry->startIndex );
        };
    }
    
    //Render QUADS
    m_pDriver->EnableCulling(false);
    m_pDriver->SetWorldMatrix(&Matrix::s_Identity, 0, 0);

    //m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );
    m_pDriver->EnableAlphaTest(true, 32);// m_pQuads[q].bForceZWrite ? false : true, 32 );
    for (int32_t q=(int32_t)m_uQuadCnt-1; q>=0; q--)
    {
        /*
        if ( m_pQuads[q].color.w < 1.0f )
        {
            m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );
            m_pDriver->EnableAlphaTest( m_pQuads[q].bForceZWrite ? false : true, 32 );
        }
        else
        {
            m_pDriver->SetBlendMode();
            m_pDriver->EnableAlphaTest(false);
        }
        
        //write to the stencil buffer if m_pQuads[q].bForceZWrite == true
        if ( m_pQuads[q].bForceZWrite == true )
            m_pDriver->EnableStencilWriting(true, 1);
        else
            m_pDriver->EnableStencilWriting(m_pQuads[q].color.w < 0 ? false : true, 0);
        */

        //Set the texture.
        m_pDriver->SetTexture( 0, m_pQuads[q].hTex, IDriver3D::FILTER_POINT, false );
        //Set the lights.
        m_pDriver->SetLights(m_pQuads[q].nLightCnt, m_pQuads[q].pLightList);
        //Render
        m_pDriver->RenderWorldQuad(m_pQuads[q].posList, m_pQuads[q].uvList, m_pQuads[q].color, m_pQuads[q].bApplyLighting);
    }
    m_pDriver->SetLights(0, nullptr);

    //Cleanup.
    m_pDriver->SetBlendMode();
    m_pDriver->EnableCulling(true);
    m_pDriver->EnableStencilWriting(false, 0);
    m_pDriver->SetWorldMatrix(&Matrix::s_Identity, 0, 0);
}

bool RenderQue::SortCB_Opaque(MaterialEntry*& d1, MaterialEntry*& d2)
{
    bool bLess=false;

    if ( d1->hTex < d2->hTex )                                                          //<First sort by texture.
        bLess = true;
    else if ( d1->hTex == d2->hTex && d1->pVB < d2->pVB )                               //<Then sort by vertex buffer.
        bLess = true;
    else if ( d1->hTex == d2->hTex && d1->pVB == d2->pVB && d1->mWorld < d2->mWorld )   //<Finally sort by transform.
        bLess = true;

    return bLess;
}
