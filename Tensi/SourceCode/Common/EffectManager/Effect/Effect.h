#pragma once
#include "..\..\..\Global.h"

// �x���ɂ��ẴR�[�h���͂𖳌��ɂ���.4005�F�Ē�`.
#pragma warning( disable:4005 )

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <mutex>

//-----------------------------------------------.
//	��ɂ��Ă����w�b�_�[�A���C�u�����̓ǂݍ���.
//-----------------------------------------------.
#include <stdio.h>

#include <d3d11.h>
#pragma comment( lib, "d3d11.lib" )

//-----------------------------------------------.
//	��������Effekseer�֌W �w�b�_�[�A���C�u�����̓ǂݍ���.
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

// �uD3DX�`�v�g�p����̂ɕK�v.
// ���ӁFEffekseer�֌W����ɓǂݍ��ނ���.
#include <d3dx10.h>
#pragma comment( lib, "d3dx10.lib" )

/************************************************
*	�t���[�\�t�g Effekseer�̃f�[�^���g�����߂̃N���X.
**/
class CEffect final
{
public:
	CEffect();
	CEffect( const std::string& fileName );
	~CEffect();

	// ������.
	HRESULT Init( const std::string& FileName );

	// �\�z�֐�.
	HRESULT Create();
	// �`��֐�.
	HRESULT Render( const Effekseer::Handle& handle );

	// �f�[�^�Ǎ��֐�.
	HRESULT LoadData( const std::string& FileName );

	//----------------------------------------------.
	//	����n�֐�.
	//----------------------------------------------.
	// �Đ��֐�.
	inline Effekseer::Handle Play( const D3DXPOSITION3& Pos ) const
	{
		return m_pManager->Play( m_pEffect, Pos.x, Pos.y, Pos.z );
	}
	// ��~�֐�.
	inline void Stop( const Effekseer::Handle& handle )
	{
		if( handle == -1 ) return; 
		m_pManager->StopEffect( handle );
	} 
	// ���X�ɒ�~������֐�.
	inline void StopRoot( const Effekseer::Handle& handle )
	{
		if ( handle == -1 ) return;
		m_pManager->StopRoot( handle );
	}
	// �S�Ē�~�֐�.
	inline void StopAll()
	{
		if ( m_pManager == nullptr ) return;
		m_pManager->StopAllEffects();
	}
	// �ꎞ��~�������͍ĊJ����.
	inline void Pause( const Effekseer::Handle& handle, bool Flag )
	{
		if( handle == -1 ) return;
		m_pManager->SetPaused( handle, Flag );
	}
	// ���ׂĂ��ꎞ��~�������͍ĊJ����.
	inline void AllPause( const bool& Flag )
	{
		m_pManager->SetPausedToAllEffects( Flag );
	};
	// �ʒu���w�肷��.
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
	// �T�C�Y���w�肷��.
	inline void SetScale( const Effekseer::Handle& handle, const D3DXSCALE3& Scale )
	{
		if( handle == -1 ) return;
		m_pManager->SetScale( handle, Scale.x, Scale.y, Scale.z );
	}
	// ��]���w�肷��.
	inline void SetRotation( const Effekseer::Handle& handle, const D3DXROTATION3& Rot )
	{
		if( handle == -1 ) return;
		m_pManager->SetRotation( handle, Rot.x, Rot.y, Rot.z );
	}
	// ��]���w�肷��(���ƃ��W�A��).
	inline void SetRotation( const Effekseer::Handle& handle, const D3DXVECTOR3& Axis, float Radian )
	{
		if( handle == -1 ) return;
		m_pManager->SetRotation( handle, ConvertVector3ToEfk( Axis ), Radian );
	}
	// �Đ����x��ݒ肷��.
	inline void SetSpeed( const Effekseer::Handle& handle, float Speed )
	{
		if( handle == -1 ) return;
		m_pManager->SetSpeed( handle, Speed );
	}
	// �F��ݒ肷��.
	inline void SetColor( const Effekseer::Handle& handle, D3DXCOLOR4 color )
	{
		if ( handle == -1 ) return;
		m_pManager->SetAllColor( handle, ConvertColorToEfk( color ) );
	}
	// ���݂��Ă��邩�m�F.
	inline bool IsExists( const Effekseer::Handle& handle )
	{
		if( handle == -1 ) return false;
		return m_pManager->Exists( handle );
	}
	// �`�悵�Ă��邩�m�F.
	inline bool IsShow( const Effekseer::Handle& handle )
	{
		if( handle == -1 ) return false;
		return m_pManager->GetShown( handle );
	}
	// �G�t�F�N�g�Ɏg�p����Ă���C���X�^���X�����擾.
	inline int GetTotalInstanceCount()
	{
		return m_pManager->GetTotalInstanceCount();
	}

private:
	//----------------------------------------------.
	//	�ϊ��p�֐�.
	//----------------------------------------------.
	// �x�N�^�[�ϊ��֐�.
	::Effekseer::Vector3D ConvertVector3ToEfk( const D3DXVECTOR3& pSrcVec3Dx );
	D3DXVECTOR3 ConvertVector3ToDx( const ::Effekseer::Vector3D& pSrcVec3Efk );
	// �s��ϊ��֐�.
	::Effekseer::Matrix44 ConvertMatrixToEfk( const D3DXMATRIX& pSrcMatDx );
	D3DXMATRIX ConvertMatrixToDx( const ::Effekseer::Matrix44& pSrcMatEfk );
	// �F�ϊ��֐�.
	::Effekseer::Color ConvertColorToEfk( const D3DXCOLOR4& pColor );
	D3DXCOLOR4 ConvertColorToDx( const ::Effekseer::Color& pColor );

	// �r���[�s���ݒ�.
	void SetViewMatrix( D3DXMATRIX& mView );
	// �ˉe�s���ݒ�.
	void SetProjectionMatrix( D3DXMATRIX mProj );

private:
	// �G�t�F�N�g�𓮍삳���邽�߂ɕK�v.
	::Effekseer::ManagerRef					m_pManager;
	::EffekseerRendererDX11::RendererRef	m_pRenderer;
	::Effekseer::EffectRef					m_pEffect;

	std::mutex								m_Mutex;
};
