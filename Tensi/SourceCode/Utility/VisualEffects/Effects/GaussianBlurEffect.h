#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "VisualEffectBase.h"

/************************************************
*	ガウスブラー( 横→縦の2パス ).
**/
class CGaussianBlurEffect final
	: public CVisualEffectWithParam<SGaussianBlurParam>
{
public:
	virtual const char* GetName() const override { return "GaussianBlur"; }
	virtual void Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV ) override;
	virtual void DebugParamEdit() override;

protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual bool Init() override;
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
