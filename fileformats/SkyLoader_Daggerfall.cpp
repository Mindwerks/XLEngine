#include "SkyLoader_Daggerfall.h"
#include "../EngineSettings.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/ArchiveManager.h"
#include "../render/TextureCache.h"
#include "../memory/ScratchPad.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

SkyLoader_Daggerfall::SkyLoader_Daggerfall() : SkyLoader()
{
    for (int32_t i=0; i<MAX_REGION_COUNT; i++)
    {
        m_aSkyData[i].bLoaded = false;
    }
}

SkyLoader_Daggerfall::~SkyLoader_Daggerfall()
{
}

bool SkyLoader_Daggerfall::LoadSky(int32_t regionID)
{
    if ( m_aSkyData[regionID].bLoaded )
         return true;

    bool bSuccess = false;
    char szFile[256];
    sprintf(szFile, "SKY%02d.DAT", regionID);

    if ( ArchiveManager::File_Open(szFile) )
    {
        ScratchPad::StartFrame();

        uint32_t uLen  = ArchiveManager::File_GetLength();
        uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );
        assert(pData);

        if ( pData )
        {
            ArchiveManager::File_Read(pData, 0, uLen);
        }
        ArchiveManager::File_Close();

        if ( pData )
        {
            SkyData *skyData = &m_aSkyData[regionID];

            int32_t index = 0;
            int32_t fileSize;
            int16_t formatID;
            int16_t formatVersion;
            for (int32_t p=0; p<32; p++)
            {
                fileSize = READ_int32_t(pData, index);
                formatID = READ_int16_t(pData, index);
                formatVersion = READ_int16_t(pData, index);
                
                for (int32_t c=0; c<256; c++)
                {
                    skyData->aPalettes[p].colors[c*3+0] = READ_uint8_t(pData, index);
                    skyData->aPalettes[p].colors[c*3+1] = READ_uint8_t(pData, index);
                    skyData->aPalettes[p].colors[c*3+2] = READ_uint8_t(pData, index);
                }
            }
            for (int32_t p=0; p<32; p++)
            {
                READ_DATA(&skyData->aColormaps[p].data, index, 16384);
                skyData->aColormaps[p].lightLevels = 64;
            }

            //copy the colormap for the windows to the proper map...
            for (int32_t p=0; p<32; p++)
            {
                for (int32_t l=0, lOffs=0; l<64; l++, lOffs+=256)
                {
                    skyData->aColormaps[p].data[255 + lOffs] = skyData->aColormaps[p].data[96+(p>>1) + lOffs];
                }
            }

            //west images.
            uint8_t img_data[512*220];
            for (int32_t s=0; s<32; s++)
            {
                READ_DATA(img_data, index, 512*220);
                char szName[256];
                sprintf(szName, "SkyWest_%d_%d", regionID, s);
                skyData->ahTexWest[s] = TextureCache::LoadTextureFromMem_Pal(img_data, 7, 512, 220, szName, false);
            }

            //east images.
            for (int32_t s=0; s<32; s++)
            {
                READ_DATA(img_data, index, 512*220);
                char szName[256];
                sprintf(szName, "SkyEast_%d_%d", regionID, s);
                skyData->ahTexEast[s] = TextureCache::LoadTextureFromMem_Pal(img_data, 7, 512, 220, szName, false);
            }

            m_aSkyData[regionID].bLoaded = true;
            bSuccess = true;
        }
        ScratchPad::FreeFrame();
    }

    return bSuccess;
}

void *SkyLoader_Daggerfall::GetSkyData(int32_t regionID)
{
    void *pData = NULL;
    if ( m_aSkyData[regionID].bLoaded )
    {
        pData = &m_aSkyData[regionID];
    }

    return pData;
}
