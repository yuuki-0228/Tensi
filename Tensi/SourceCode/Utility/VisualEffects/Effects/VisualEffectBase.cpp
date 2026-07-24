#include "VisualEffectBase.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\VisualEffectManager.h"
#include "..\..\..\Common\DirectX\DirectX11.h"

CVisualEffectBase::CVisualEffectBase()
	: m_pPixelShaders	()
	, m_Intensity		( 1.0f )
	, m_AlphaLift		( 0.0f )
	, m_Area			( 0.0f, 0.0f, 0.0f, 0.0f )
	, m_IsInit			( false )
	, m_IsInitFailed	( false )
{
}

CVisualEffectBase::~CVisualEffectBase()
{
	for ( auto& [Entry, pPS] : m_pPixelShaders ) {
		SAFE_RELEASE( pPS );
	}
	m_pPixelShaders.clear();
}

//----------------------------.
// 初期化( 未初期化なら初期化する ).
//----------------------------.
bool CVisualEffectBase::EnsureInit()
{
	if ( m_IsInit )			return true;
	if ( m_IsInitFailed )	return false;

	if ( Init() == false || InitResource() == false ) {
		// 失敗を記録して以後は適用をスキップする.
		m_IsInitFailed = true;
		Log::PushLogError( std::string( "視覚効果の初期化失敗 : " ) + GetName() );
		return false;
	}
	m_IsInit = true;
	return true;
}

//----------------------------.
// 初期化本体.
//----------------------------.
bool CVisualEffectBase::Init()
{
	const char* pEntry = GetMainEntry();
	if ( pEntry == nullptr ) return false;
	return CompilePS( GetShaderPath(), pEntry ) != nullptr;
}

//----------------------------.
// 適用( 既定 : 単パス ).
//----------------------------.
void CVisualEffectBase::Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV )
{
	D3DXVECTOR4 Params[PARAM_VECTOR_NUM] = {};
	PackParam( Params );
	VisualEffectManager::DrawPass( GetPS( GetMainEntry() ), pDstRTV, pSrcSRV, nullptr, Params, PARAM_VECTOR_NUM );
}

//----------------------------.
// ピクセルシェーダのコンパイル.
//----------------------------.
ID3D11PixelShader* CVisualEffectBase::CompilePS( const TCHAR* pShaderPath, const char* pEntry )
{
	// コンパイル済みならそれを返す.
	auto Itr = m_pPixelShaders.find( pEntry );
	if ( Itr != m_pPixelShaders.end() ) return Itr->second;

	UINT CompileFlag = 0;
#ifdef _DEBUG
	CompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG

	ID3DBlob* pCompiledShader	= nullptr;
	ID3DBlob* pErrors			= nullptr;
	if ( FAILED( DirectX11::MutexD3DX11CompileFromFile(
		pShaderPath, nullptr, nullptr, pEntry, "ps_5_0",
		CompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr ) ) )
	{
		if ( pErrors != nullptr ) {
			Log::PushLogError( std::string( "視覚効果シェーダのコンパイル失敗 : " ) + pEntry + " : "
				+ static_cast<char*>( pErrors->GetBufferPointer() ) );
		}
		SAFE_RELEASE( pErrors );
		return nullptr;
	}
	SAFE_RELEASE( pErrors );

	ID3D11PixelShader* pPS = nullptr;
	if ( FAILED( DirectX11::MutexDX11CreatePixelShader( pCompiledShader, nullptr, &pPS ) ) ) {
		SAFE_RELEASE( pCompiledShader );
		Log::PushLogError( std::string( "視覚効果ピクセルシェーダの作成失敗 : " ) + pEntry );
		return nullptr;
	}
	SAFE_RELEASE( pCompiledShader );

	m_pPixelShaders[pEntry] = pPS;
	return pPS;
}

//----------------------------.
// コンパイル済みピクセルシェーダの取得.
//----------------------------.
ID3D11PixelShader* CVisualEffectBase::GetPS( const char* pEntry ) const
{
	if ( pEntry == nullptr ) return nullptr;
	auto Itr = m_pPixelShaders.find( pEntry );
	return Itr != m_pPixelShaders.end() ? Itr->second : nullptr;
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
