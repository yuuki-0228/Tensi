#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "VisualEffectBase.h"

/************************************************
*	“Ü‚è( ‚·‚èƒKƒ‰ƒX. ‚Ú‚©‚µ + ‚´‚ç‚Â‚«˜c‚Ý + ”’‚Ý ).
**/
class CFrostEffect final
	: public CVisualEffectWithParam<SFrostParam>
{
public:
	virtual const char* GetName() const override { return "Frost"; }
	virtual void Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV ) override;
	virtual void DebugParamEdit() override;

protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual bool Init() override;
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
