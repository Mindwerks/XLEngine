#include "ImageLoader.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#define IL_USE_PRAGMA_LIBS
#include <IL/il.h>

#define MAX_IMAGE_WIDTH 2048

ImageLoader::ImageLoader(void)
{
	//we're going to allocate a buffer for image loading.
	m_pImageData      = (u8 *)xlMalloc(MAX_IMAGE_WIDTH*MAX_IMAGE_WIDTH*4);
	m_pImageData_Work = (u8 *)xlMalloc(MAX_IMAGE_WIDTH*MAX_IMAGE_WIDTH*4);
	m_uWidth = 0;
	m_uHeight = 0;

	// Initialize IL
	ilInit();

	/* We want all images to be loaded in a consistent manner */
	ilEnable(IL_ORIGIN_SET);
}

ImageLoader::~ImageLoader(void)
{
	if ( m_pImageData )
	{
		xlFree(m_pImageData);
		m_pImageData = NULL;
	}
	if ( m_pImageData_Work )
	{
		xlFree(m_pImageData_Work);
		m_pImageData_Work = NULL;
	}
}

void ImageLoader::FreeImageData()
{
	m_uWidth = 0;
	m_uHeight = 0;
}

void ImageLoader::SetPath(const char *pszPath)
{
	strcpy(m_szPath, pszPath);
}

bool ImageLoader::Save_ImageRGBA(const char *pszImage, u8 *pData, u32 uWidth, u32 uHeight)
{
	ILuint handle;

	/* In the next section, we load one image */
	ilGenImages(1, &handle);
	ilBindImage(handle);

	ilTexImage(uWidth, uHeight, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pData);
	ilEnable(IL_FILE_OVERWRITE);
	ILboolean bSave = ilSaveImage(pszImage);

	/* Finally, clean the mess! */
	ilDeleteImages(1, &handle);

	return bSave ? true : false;
}

bool ImageLoader::Load_Image(const char *pszImage)
{
	FreeImageData();

	char szExt[4];
	memset(szExt, 0, 4);
	//1. what is the extension?
	size_t l = strlen(pszImage);
	szExt[0] = pszImage[l-3];
	szExt[1] = pszImage[l-2];
	szExt[2] = pszImage[l-1];
	szExt[3] = 0;
	//2. create the final path...
	char szFilePath[260];
	sprintf(szFilePath, "%s%s", m_szPath, pszImage);

	//now let's switch over to using devIL...
	ILuint handle;

	/* In the next section, we load one image */
	ilGenImages(1, &handle);
	ilBindImage(handle);
	ILboolean loaded = ilLoadImage(szFilePath);

	if (loaded == IL_FALSE)
		return false; /* error encountered during loading */

	/* Let’s spy on it a little bit */
	m_uWidth  = (u32)ilGetInteger(IL_IMAGE_WIDTH); // getting image width
	m_uHeight = (u32)ilGetInteger(IL_IMAGE_HEIGHT); // and height

	/* make sure our buffer is big enough. */
	assert( m_uWidth <= 2048 && m_uHeight <= 2048 );

	/* finally get the image data */
	ilCopyPixels(0, 0, 0, m_uWidth, m_uHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, m_pImageData_Work);

	//now flip the image for OpenGL...
	u32 uStride = 4*m_uWidth;
	for (u32 y=0; y<m_uHeight; y++)
	{
		memcpy(&m_pImageData[y*uStride], &m_pImageData_Work[(m_uHeight-1-y)*uStride], uStride);
	}

	/* Finally, clean the mess! */
	ilDeleteImages(1, &handle);

	return true;
}
