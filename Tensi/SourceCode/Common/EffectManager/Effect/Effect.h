#pragma once
#include "..\..\..\Global.h"

// 警告についてのコード分析を無効にする.4005：再定義.
#pragma warning( disable:4005 )

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <mutex>

//-----------------------------------------------.
//	先にしておくヘッダー、ライブラリの読み込み.
//-----------------------------------------------.
#include <stdio.h>

#include <d3d11.h>
#pragma comment( lib, "d3d11.lib" )

//-----------------------------------------------.
//	ここからEffekseer関係 ヘッダー、ライブラリの読み込み.
//-----------------------------------------------.
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>

#ifdef _DEBUG
	#pragma comment( lib, "Debug\\Effekseer.lib" )
	#pragma comment( lib, "Debug\\EffekseerRendererDX11.lib" )
#else	// #ifdef _DEBUG.
	#pragma comment( lib, "Release\\Effekseer.lib" )
	#pragma comment( lib, "Release\\EffekseerRendererDX11.lib" )
#endif	// #ifdef _DEBUG.

// 「D3DX～」使用するのに必要.
// 注意：Effekseer関係より後に読み込むこと.
#include <d3dx10.h>
#pragma comment( lib, "d3dx10.lib" )

/************************************************
*	フリーソフト Effekseerのデータを使うためのクラス.
**/
class CEffect final
{
public:
	CEffect();
	CEffect( const std::string& fileName );
	~CEffect();

	// 初期化.
	HRESULT Init( const std::string& FileName );

	// 構築関数.
	HRESULT Create();
	// 描画関数.
	HRESULT Render( const Effekseer::Handle& handle );

	// データ読込関数.
	HRESULT LoadData( const std::string& FileName );

	//----------------------------------------------.
	//	制御系関数.
	//----------------------------------------------.
	// 再生関数.
	inline Effekseer::Handle Play( const D3DXPOSITION3& Pos ) const
	{
		return m_pManager->Play( m_pEffect, Pos.x, Pos.y, Pos.z );
	}
	// 停止関数.
	inline void Stop( const Effekseer::Handle& handle )
	{
		if( handle == -1 ) return; 
		m_pManager->StopEffect( handle );
	} 
	// 徐々に停止させる関数.
	inline void StopRoot( const Effekseer::Handle& handle )
	{
		if ( handle == -1 ) return;
		m_pManager->StopRoot( handle );
	}
	// 全て停止関数.
	inline void StopAll()
	{
		if ( m_pManager == nullptr ) return;
		m_pManager->StopAllEffects();
	}
	// 一時停止もしくは再開する.
	inline void Pause( const Effekseer::Handle& handle, bool Flag )
	{
		if( handle == -1 ) return;
		m_pManager->SetPaused( handle, Flag );
	}
	// すべてを一時停止もしくは再開する.
	inline void AllPause( const bool& Flag )
	{
		m_pManager->SetPausedToAllEffects( Flag );
	};
	// 位置を指定する.
	inline void SetLocation( const Effekseer::Handle& handle, const D3DXPOSITION3& Pos )
	{
		if( handle == -1 ) return;
		m_pManager->SetLocation( handle, ConvertVector3ToEfk( Pos ) );
	}
	inline void SetLocation( const Effekseer::Handle& handle, const float& x, const float& y, const float& z )
	{
		if( handle == -1 ) return;
		m_pManager->SetLocation( handle, x, y, z );
	}
	// サイズを指定する.
	inline void SetScale( const Effekseer::Handle& handle, const D3DXSCALE3& Scale )
	{
		if( handle == -1 ) return;
		m_pManager->SetScale( handle, Scale.x, Scale.y, Scale.z );
	}
	// 回転を指定する.
	inline void SetRotation( const Effekseer::Handle& handle, const D3DXROTATION3& Rot )
	{
		if( handle == -1 ) return;
		m_pManager->SetRotation( handle, Rot.x, Rot.y, Rot.z );
	}
	// 回転を指定する(軸とラジアン).
	inline void SetRotation( const Effekseer::Handle& handle, const D3DXVECTOR3& Axis, float Radian )
	{
		if( handle == -1 ) return;
		m_pManager->SetRotation( handle, ConvertVector3ToEfk( Axis ), Radian );
	}
	// 再生速度を設定する.
	inline void SetSpeed( const Effekseer::Handle& handle, float Speed )
	{
		if( handle == -1 ) return;
		m_pManager->SetSpeed( handle, Speed );
	}
	// 色を設定する.
	inline void SetColor( const Effekseer::Handle& handle, D3DXCOLOR4 color )
	{
		if ( handle == -1 ) return;
		m_pManager->SetAllColor( handle, ConvertColorToEfk( color ) );
	}
	// 存在しているか確認.
	inline bool IsExists( const Effekseer::Handle& handle )
	{
		if( handle == -1 ) return false;
		return m_pManager->Exists( handle );
	}
	// 描画しているか確認.
	inline bool IsShow( const Effekseer::Handle& handle )
	{
		if( handle == -1 ) return false;
		return m_pManager->GetShown( handle );
	}
	// エフェクトに使用されているインスタンス数を取得.
	inline int GetTotalInstanceCount()
	{
		return m_pManager->GetTotalInstanceCount();
	}

private:
	//----------------------------------------------.
	//	変換用関数.
	//----------------------------------------------.
	// ベクター変換関数.
	::Effekseer::Vector3D ConvertVector3ToEfk( const D3DXVECTOR3& pSrcVec3Dx );
	D3DXVECTOR3 ConvertVector3ToDx( const ::Effekseer::Vector3D& pSrcVec3Efk );
	// 行列変換関数.
	::Effekseer::Matrix44 ConvertMatrixToEfk( const D3DXMATRIX& pSrcMatDx );
	D3DXMATRIX ConvertMatrixToDx( const ::Effekseer::Matrix44& pSrcMatEfk );
	// 色変換関数.
	::Effekseer::Color ConvertColorToEfk( const D3DXCOLOR4& pColor );
	D3DXCOLOR4 ConvertColorToDx( const ::Effekseer::Color& pColor );

	// ビュー行列を設定.
	void SetViewMatrix( D3DXMATRIX& mView );
	// 射影行列を設定.
	void SetProjectionMatrix( D3DXMATRIX mProj );

private:
	// エフェクトを動作させるために必要.
	::Effekseer::ManagerRef					m_pManager;
	::EffekseerRendererDX11::RendererRef	m_pRenderer;
	::Effekseer::EffectRef					m_pEffect;

	std::mutex								m_Mutex;
};
