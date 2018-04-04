#ifndef RENDERCOMPONENT_H
#define RENDERCOMPONENT_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"

class IDriver3D;
class Object;

class RenderComponent
{
public:
	RenderComponent(){};
	virtual ~RenderComponent(){};

	virtual void Render(Object *pObj, IDriver3D *pDriver, f32 fIntensity, const Vector3& vOffset) {};
	virtual void GetBounds(Vector3& vMin, Vector3& vMax) {};
	virtual void SetTextureHandle(TextureHandle hTex) {};
	virtual void SetUV_Flip(bool bFlipX, bool bFlipY, bool bFlipAxis=false) {};
};

#endif //RENDERCOMPONENT_H
