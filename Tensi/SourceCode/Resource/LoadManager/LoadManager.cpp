#include "LoadManager.h"
#include "..\MeshResource\MeshResource.h"
#include "..\SpriteResource\SpriteResource.h"
#include "..\EffectResource\EffectResource.h"
#include "..\FontResource\FontResource.h"
#include "..\..\Common\SoundManeger\SoundManeger.h"
#include "..\..\Utility\MasterData\MasterDataAccesser.h"

CLoadManager::CLoadManager()
	: m_Thread				()
	, m_Mutex				()
	, m_isLoadEnd			( false )
	, m_isThreadJoined		( false )
	, m_isLoadFailed		( false )
{
}

CLoadManager::~CLoadManager()
{
	if ( m_isLoadFailed ) m_Thread.join();
}

//---------------------------.
// 読み込み.
//---------------------------.
void CLoadManager::LoadResource( HWND hWnd )
{
	if ( FAILED( SoundManager::SoundLoad( hWnd ) ) ){
		m_isLoadFailed = true;
		return;
	}

	// 読み込み関数.
	auto Load = [&]( HWND hWnd ) {
		std::unique_lock<std::mutex> lock( m_Mutex );

		Log::PushLog( "------ スレッドロード開始 ------" );
		if ( FAILED( SpriteResource::SpriteLoad() ) ){
			m_isLoadFailed = true;
			return;
		}
		if ( FAILED( EffectResource::EffectLoad() ) ) {
			m_isLoadFailed = true;
			return;
		}
		if ( FAILED( FontResource::FontLoad() ) ) {
			m_isLoadFailed = true;
			return;
		}
		if ( FAILED( MeshResource::MeshLoad() ) ) {
			m_isLoadFailed = true;
			return;
		}
		if ( FAILED( MasterDataAccesser::Load() ) ) {
			m_isLoadFailed = true;
			return;
		}
		m_isLoadEnd = true;
		Log::PushLog( "------ スレッドロード終了 ------" );
		Log::PushLog( "------ メインループ開始 ------" );
	};

	// 読み込み.
	m_Thread = std::thread( Load, hWnd );
}

//---------------------------.
// スレッドの解放.
//---------------------------.
bool CLoadManager::ThreadRelease()
{
	if( m_isThreadJoined == true	) return true;
	if( m_isLoadEnd		 == false	) return false;

	DWORD code = -1;
	GetExitCodeThread( m_Thread.native_handle(),&code );
	if( code == 0xffffffff ){
		m_isThreadJoined = true;
	}
	if ( code == 0 ) {
		m_Thread.join();
		while ( 1 ) {
			if ( m_Thread.joinable() != true ) {
				m_isThreadJoined = true;
				break;
			}
		}
	}
	return true;
}