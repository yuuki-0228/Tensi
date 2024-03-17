#include "EffectManager.h"
#include "..\..\Resource\EffectResource\EffectResource.h"

CEffectManager::CEffectManager()
	: m_pEffect		( nullptr )
	, m_Handle		( -1 )
	, m_Transform	()
{
}

CEffectManager::~CEffectManager()
{
}

//------------------------------.
// エフェクトの設定.
//------------------------------.
bool CEffectManager::SetEffect( const std::string& Name )
{
	// エフェクトを取得.
	m_pEffect = EffectResource::GetEffect( Name );

	return true;
}

//------------------------------.
// 描画.
//	エフェクトを設定していない場合は処理は行わない.
//------------------------------.
void CEffectManager::Render()
{
	if( m_pEffect	== nullptr	) return;
	if( m_Handle	== -1		) return;

	m_pEffect->Render( m_Handle );

	// エフェクトを描画しているか.
	if( m_pEffect->IsShow( m_Handle ) == false ) m_Handle = -1;
}

//------------------------------.
// 再生.
//	エフェクトを設定していない場合は処理は行わない.
//------------------------------.
bool CEffectManager::Play( const STransform& Trans )
{
	return Play( Trans.Position, Trans.Rotation, Trans.Scale );
}
bool CEffectManager::Play( const D3DXPOSITION3& Pos, const D3DXROTATION3& Rot, const D3DXSCALE3& Scale )
{
	if( m_pEffect	== nullptr	) return false;
	if( m_Handle	!= -1		) return false;

	m_Handle				= m_pEffect->Play( Pos );
	m_Transform.Position	= Pos;

	SetRotation( Rot );	// 回転の設定.
	SetScale( Scale );	// サイズの設定.

	return true;
}

//------------------------------.
// 停止.
//------------------------------.
void CEffectManager::Stop()
{
	m_pEffect->Stop( m_Handle );
}

//------------------------------.
// 徐々に停止させる.
//------------------------------.
void CEffectManager::StopRoot()
{
	m_pEffect->StopRoot( m_Handle );
}

//------------------------------.
// 全て停止.
//------------------------------.
void CEffectManager::StopAll()
{
	m_pEffect->StopAll();
}

//------------------------------.
// 一時停止.
//------------------------------.
void CEffectManager::Pause( const bool& Flag )
{
	m_pEffect->Pause( m_Handle, Flag ); 
}

//------------------------------.
// すべてを一時停止もしくは再開する.
//------------------------------.
void CEffectManager::AllPause( const bool& Flag )
{
	m_pEffect->AllPause( Flag );
}

//------------------------------.
// 位置を指定する.
//------------------------------.
void CEffectManager::SetPosition( const D3DXPOSITION3& Pos )
{
	m_pEffect->SetLocation( m_Handle, Pos );
	m_Transform.Position = Pos;
}

//------------------------------.
// 回転を指定する.
//------------------------------.
void CEffectManager::SetRotation( const D3DXROTATION3& Rot )
{
	m_pEffect->SetRotation( m_Handle, Rot );
	m_Transform.Rotation = { Rot };
}

//------------------------------.
// サイズを指定する.
//------------------------------.
void CEffectManager::SetScale( const D3DXSCALE3& Scale )
{
	m_pEffect->SetScale( m_Handle, Scale );
	m_Transform.Scale = Scale;
}
void CEffectManager::SetScale( const float& scale )
{
	m_pEffect->SetScale( m_Handle, { scale, scale, scale } );
	m_Transform.Scale = { scale, scale, scale };
}

//------------------------------.
// 再生速度を設定する.
//------------------------------.
void CEffectManager::SetSpeed( const float& Speed )
{
	m_pEffect->SetSpeed( m_Handle, Speed );
}

//------------------------------.
// 色の設定.
//------------------------------.
void CEffectManager::SetColor( const D3DXCOLOR3& Color )
{
	SetColor( Color4::RGBA( Color ) );
}
void CEffectManager::SetColor( const D3DXCOLOR4& Color )
{
	m_pEffect->SetColor( m_Handle, Color );
}

//------------------------------.
// アルファ値の設定.
//------------------------------.
void CEffectManager::SetAlpha( const float& Alpha )
{
	SetColor( Color4::RGBA( Color::White, Alpha ) );
}

//------------------------------.
// 存在しているか.
//------------------------------.
bool CEffectManager::IsShow()
{
	return m_pEffect->IsShow( m_Handle );
}

//------------------------------.
// エフェクトに使用されているインスタンス数を取得.
//------------------------------.
int CEffectManager::GetTotalInstanceCount()
{
	return m_pEffect->GetTotalInstanceCount();
}