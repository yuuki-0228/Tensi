#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "VisualEffectBase.h"

/************************************************
*	ブルーム( 高輝度抽出 → ぼかし → 加算合成 ).
**/
class CBloomEffect final
	: public CVisualEffectWithParam<SBloomParam>
{
public:
	virtual const char* GetName() const override { return "Bloom"; }
	virtual void Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV ) override;
	virtual void DebugParamEdit() override;

protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual bool Init() override;
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
