#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "VisualEffectBase.h"

/************************************************
*	ソフトフォーカス( ぼかしをスクリーン合成してふんわり光らせる ).
**/
class CSoftFocusEffect final
	: public CVisualEffectWithParam<SSoftFocusParam>
{
public:
	virtual const char* GetName() const override { return "SoftFocus"; }
	virtual void Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV ) override;
	virtual void DebugParamEdit() override;

protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual bool Init() override;
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
