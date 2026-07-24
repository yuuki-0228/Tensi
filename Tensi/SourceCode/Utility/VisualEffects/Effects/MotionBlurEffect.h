#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "VisualEffectBase.h"
#include <memory>

class CRenderTexture;

/************************************************
*	モーションブラー( 前フレームとの蓄積合成による残像 ).
**/
class CMotionBlurEffect final
	: public CVisualEffectWithParam<SMotionBlurParam>
{
public:
	CMotionBlurEffect();
	virtual ~CMotionBlurEffect();

	virtual const char* GetName() const override { return "MotionBlur"; }
	virtual void Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV ) override;
	virtual void DebugParamEdit() override;

protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Accum"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;

private:
	std::unique_ptr<CRenderTexture>	m_pHistory;	// 前フレームの結果( 履歴 ).
	std::unique_ptr<CRenderTexture>	m_pResult;	// 今フレームの合成結果.
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
