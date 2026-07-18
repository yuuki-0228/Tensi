#include "LoadManager.h"
#include "..\MeshResource\MeshResource.h"
#include "..\SpriteResource\SpriteResource.h"
#include "..\EffectResource\EffectResource.h"
#include "..\FontResource\FontResource.h"
#include "..\..\Common\XAudio2\SoundManager.h"
#include "..\..\Utility\MasterData\MasterDataAccesser.h"
#include "..\..\Utility\CashManager\CashManager.h"

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
#ifdef ENABLE_SOUND
	SoundManager::CreateSoundData();
#endif

	// 読み込み関数.
	auto Load = [&]( HWND hWnd ) {
		std::unique_lock<std::mutex> lock( m_Mutex );

		Log::PushLogInfo( "------ スレッドロード開始 ------" );
#ifdef ENABLE_SPRITE
		if ( FAILED( SpriteResource::SpriteLoad() ) ){
			m_isLoadFailed = true;
			return;
		}
#endif
#ifdef ENABLE_EFFEKSEER
		if ( FAILED( EffectResource::EffectLoad() ) ) {
			m_isLoadFailed = true;
			return;
		}
#endif
#ifdef ENABLE_FONT
		if ( FAILED( FontResource::FontLoad() ) ) {
			m_isLoadFailed = true;
			return;
		}
#endif
#ifdef ENABLE_MESH
		if ( FAILED( MeshResource::MeshLoad() ) ) {
			m_isLoadFailed = true;
			return;
		}
#endif
#ifdef ENABLE_MASTER_DATA
		if ( FAILED( MasterDataAccesser::Load() ) ) {
			m_isLoadFailed = true;
			return;
		}
#endif
#ifdef ENABLE_CASH
		if (FAILED(CashManager::Load())) {
			m_isLoadFailed = true;
			return;
		}
#endif
		m_isLoadEnd = true;
		Log::PushLogInfo( "------ スレッドロード終了 ------" );
		Log::PushLogInfo( "------ メインループ開始 ------" );
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