#include "XAudio2PlayBGM.h"
#ifdef ENABLE_SOUND
#include "XAudio2.h"
#include <vector>
#include <process.h>
#include <thread>
#include "..\..\XAudio2Master\XAudio2MasterVoice.h"
#include "..\..\SoundManager.h"
#include "..\..\..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"

namespace {
	constexpr float	MAX_PITCH			= 5.0f;		// 最大ピッチ(最高5.0f).
	constexpr float	INIT_FADE_VALUE		= -100.0f;	// フェードする値を1回だけ計算するための初期化値.
	constexpr int	DIVISION_FADE_VALUE	= 3000;		// フェードする値を計算するための割り算の値.
}

CXAudio2PlayBGM::CXAudio2PlayBGM()
	: m_pSourceVoice			( nullptr )
	, m_fMaxPitch				( MAX_PITCH )
	, m_fPitch					( 1.0f )
	, m_fSoundVolume			( 1.0f )
	, m_fFadeInValue			( -100.0f )
	, m_fFadeOutValue			( -100.0f )
	, m_isMoveFadeInThread		( false )
	, m_isMoveFadeOutThread		( false )
	, m_FadeInStartFlag			( false )
	, m_FadeOutStartFlag		( false )
	, m_FirstPlayFlag			( true )
	, m_isAudioStop				( false )
{
	m_Buffer	= { 0 };
	m_Buff_len	= 0;
	m_Buff		= nullptr;
	m_Len		= 0;
	m_Buff_cnt	= 0;
	m_Size		= 0;
}

CXAudio2PlayBGM::~CXAudio2PlayBGM()
{
	if ( m_FadeInThread.joinable()	== true ) m_FadeInThread.join();
	if ( m_FadeOutThread.joinable() == true ) m_FadeOutThread.join();
}

//----------------------------.
// 音源データをストリームに流し込む.
//----------------------------.
HRESULT CXAudio2PlayBGM::OggSubmit( std::shared_ptr<COggLoad> pOggData, const char* filename )
{
	// オーディオバッファーを用意
	// ストリーム再生するのでバッファは2ついる.
	m_Buff_len = 2;
	m_Buff = new BYTE * [m_Buff_len];
	m_Len  = pOggData->GetFormat()->nAvgBytesPerSec;

	for ( int i = 0; i < m_Buff_len; i++ ) {
		m_Buff[i] = new BYTE[m_Len];
	}

	// 最初のバッファーへデータを読み込む
	HRESULT ret = pOggData->ReadChunk( m_Buff, m_Buff_cnt, m_Len, &m_Size );
	if ( FAILED( ret ) )
	{
		_ASSERT_EXPR( "error SubmitSourceBuffer ret=%d\n", ret );
		return E_FAIL;
	}

	m_Buffer.AudioBytes = m_Size;
	m_Buffer.pAudioData = m_Buff[m_Buff_cnt];
	if ( 0 < m_Size )
	{
		ret = m_pSourceVoice->SubmitSourceBuffer( &m_Buffer );
		if ( FAILED( ret ) ) {
			_ASSERT_EXPR( "error SubmitSourceBuffer ret=%d\n", ret );
			return E_FAIL;
		}
	}

	if ( m_Buff_len <= ++m_Buff_cnt ) m_Buff_cnt = 0;
	return S_OK;
}

//----------------------------.
// フェードアウト用のスレッドを起動する.
//----------------------------.
void CXAudio2PlayBGM::CallFadeOutBGM( bool& isEnd )
{
	if ( m_FadeOutStartFlag == true ) {
		if ( m_isMoveFadeOutThread == true ) return;

		m_isMoveFadeOutThread = true;
		auto BGM = [&]()
		{
			// 1度だけフェードする値を計算.
			if ( m_fFadeOutValue <= INIT_FADE_VALUE ) {
				m_fFadeOutValue = GetVolume() / DIVISION_FADE_VALUE;
			}
			FadeOutBGM( -m_fFadeOutValue, isEnd );
			m_isMoveFadeOutThread	= false;
			m_FadeOutStartFlag		= false;
			m_fFadeOutValue			= INIT_FADE_VALUE;
		};
		m_FadeOutThread = std::thread( BGM );
		m_FadeOutThread.detach();
	}
}

//----------------------------.
// フェードイン用のスレッドを起動する.
//----------------------------.
void CXAudio2PlayBGM::CallFadeInBGM( bool& isEnd )
{
	if ( m_FadeInStartFlag == true ) {
		if ( m_isMoveFadeInThread == true ) return;

		m_isMoveFadeInThread = true;
		auto BGM = [&]()
		{
			// 1度だけフェードする値を計算.
			if ( m_fFadeInValue <= INIT_FADE_VALUE ) {
				const auto& volume = SoundManager::GetSoundVolume();
				m_fFadeInValue = ( m_fSoundVolume * volume.BGM * volume.Master ) / DIVISION_FADE_VALUE;
			}
			FadeInBGM( m_fFadeInValue, isEnd );	// フェードイン関数.
			m_isMoveFadeInThread	= false;	// スレッド解放.
			m_FadeInStartFlag		= false;	// ここに来たらフェードインが終わってるのでフラグを下す.
			m_fFadeInValue			= INIT_FADE_VALUE;
		};
		m_FadeInThread = std::thread( BGM );
		m_FadeInThread.detach();
	}
}


//----------------------------.
// 再生中かどうかを取得する関数.
//----------------------------.
const bool CXAudio2PlayBGM::IsPlaying()
{
	if ( m_pSourceVoice == nullptr ) return 0;

	// SoundSourceのステート取得.
	XAUDIO2_VOICE_STATE xState;
	m_pSourceVoice->GetState( &xState );

	// 0じゃなければ再生中(true)を返す.
	return xState.BuffersQueued != 0;
}

//----------------------------.
// 再生.
//----------------------------.
bool CXAudio2PlayBGM::Play( std::shared_ptr<COggLoad> pWavData, const char* filename, bool& isEnd )
{
	// ロック.
	std::unique_lock<std::mutex> lock( m_mtx );
	if ( m_pSourceVoice == nullptr ) return false;

	// ピッチ設定.
	SetPitch( m_fPitch );
	SetBGMVolume();

	// SoundSourceを再生.
	m_isAudioStop = false;
	m_pSourceVoice->Start();

	// Submitは初回再生時のみ実行する.
	if ( m_FirstPlayFlag == true ) {
		OggSubmit( pWavData, filename );
		m_FirstPlayFlag = false;
	}

	// 再生ループ
	do {
		if ( isEnd == true ) break;
		if ( m_isAudioStop == false ) {
			// ファイルからデータを読み取り
			pWavData->ReadChunk( m_Buff, m_Buff_cnt, m_Len, &m_Size );
			// 最後まで読み切った.
			if ( m_Size <= 0 ) {
				// 終了(停止)フラグが立っていない.
				if ( isEnd == false ) {
					// ループの間隔
	//				Sleep(20);

					// データの読み取るポインタをリセット.
					pWavData->ResetFile();
					pWavData->ReadChunk( m_Buff, m_Buff_cnt, m_Len, &m_Size );// チャンク読み込み.
					if ( !SoundManager::GetIsSoundLogStop() ) MessageWindow::PushMessage( std::string( u8"BGM「" ) + filename + u8"」をループさせました", Color4::Green );
				} else {
					// 終了のためwhileを抜ける.
					break;
				}
			}

			// バッファー生成
			m_Buffer.AudioBytes = m_Size;
			m_Buffer.pAudioData = m_Buff[m_Buff_cnt];
			// バッファーをキューに追加
			HRESULT ret = m_pSourceVoice->SubmitSourceBuffer( &m_Buffer );
			if ( FAILED( ret ) ) {
				_ASSERT_EXPR( "error SubmitSourceBuffer ret=%d\n", false );
				return true;
			}
			// 2つ確保しているバッファーを交互に使うようにカウンタチェックとリセット
			if ( m_Buff_len <= ++m_Buff_cnt ) m_Buff_cnt = 0;

			// フェードインフラグが立っていれば、フェードイン関数を呼ぶスレッドを立てる.
			CallFadeInBGM( isEnd );
			// フェードアウトフラグが立っていれば、フェードアウト関数を呼ぶスレッドを立てる.
			CallFadeOutBGM( isEnd );
		}
	} while ( m_isAudioStop || WaitForSingleObject( m_Callback.event, INFINITE ) == WAIT_OBJECT_0 );	// バッファ終了までwhileループ

	if ( m_pSourceVoice == nullptr ) return true;

	XAUDIO2_VOICE_STATE xState;
	m_pSourceVoice->GetState( &xState );
	while ( IsPlaying() == true ) {
		Stop();
		xState.BuffersQueued = 0;
	}
	pWavData->ResetFile(); //データの読み取るポインタをリセット.

	return true;
}

//----------------------------.
// サウンド完全停止.
//----------------------------.
bool CXAudio2PlayBGM::Stop()
{
	if ( m_pSourceVoice == nullptr ) return true;

	// バッファフレッシュ.
	m_isAudioStop = true;
	m_pSourceVoice->Stop( 0 );
	m_pSourceVoice->FlushSourceBuffers();
	return true;
}

//----------------------------.
// サウンド一時停止.
//----------------------------.
bool CXAudio2PlayBGM::Pause()
{
	if ( m_pSourceVoice == nullptr ) return true;

	m_isAudioStop = true;
	m_pSourceVoice->Stop( 0 );
	return true;
}

//----------------------------.
// 停止中のサウンドを再度再生.
//----------------------------.
bool CXAudio2PlayBGM::PlayStart()
{
	if ( m_pSourceVoice == nullptr	) return true;
	if ( IsPlaying()	== false	)  return true;

	m_isAudioStop = false;
	m_pSourceVoice->Start();
	return true;
}

//----------------------------.
// Oggからソースボイスを作成する.
//----------------------------.
HRESULT CXAudio2PlayBGM::CreateOggSound( std::shared_ptr<COggLoad> pOggData, json audioData, const char* filename )
{
	if ( pOggData == nullptr ) return E_FAIL;

	// XAudio2マスターボイスのインスタンス取得.
	CXAudio2MasterVoice& xAudioMaster = CXAudio2MasterVoice::GetGlobalSystem();
	IXAudio2*			 pHandle	  = xAudioMaster.GetInterface();

	// サウンドデータの反映
	m_fSoundVolume	= audioData["SoundVolume"];
	m_fMaxPitch		= audioData["MaxPitch"];
	m_fPitch		= audioData["StartPitch"];

	// ソースボイスを作成する.
	HRESULT hr;
	if ( FAILED( hr = pHandle->CreateSourceVoice( &m_pSourceVoice,
		pOggData->GetFormat(), 0, m_fMaxPitch, &m_Callback ) ) )
	{
		// 失敗した.
		_ASSERT_EXPR( "error %#X creating bgm source voice\n", hr );
		return E_FAIL;
	}

	if ( FAILED( hr ) ) return E_FAIL;
	return S_OK;
}

//----------------------------.
// 音量取得.
//----------------------------.
const float CXAudio2PlayBGM::GetVolume()
{
	if ( m_pSourceVoice == nullptr ) return 0.0f;

	float Volume = 0.0f;
	m_pSourceVoice->GetVolume( &Volume );
	return Volume;
}

//----------------------------.
// 音量設定.
//----------------------------.
bool  CXAudio2PlayBGM::SetBGMVolume( float value )
{
	if ( m_pSourceVoice == nullptr ) return false;

	m_fSoundVolume = value;
	if ( m_fSoundVolume > 1.0f		) m_fSoundVolume = 1.0f;
	else if ( m_fSoundVolume < 0.0f ) m_fSoundVolume = 0.0f;

	const auto& volume = SoundManager::GetSoundVolume();
	m_pSourceVoice->SetVolume( m_fSoundVolume * volume.BGM * volume.Master, 0 );
	return true;
}
bool CXAudio2PlayBGM::SetBGMVolume()
{
	return SetBGMVolume( m_fSoundVolume );
}

//----------------------------.
// 現在の音量から加算または減算.
//----------------------------.
bool CXAudio2PlayBGM::AdjustVolume( float value )
{
	if ( m_pSourceVoice == nullptr ) return false;

	float Calc = GetVolume() + value;
	m_pSourceVoice->SetVolume( Calc, 0 );
	return true;
}

//----------------------------.
// ピッチを設定.
//----------------------------.
bool CXAudio2PlayBGM::SetPitch( float value )
{
	m_fPitch = value;
	if ( m_pSourceVoice == nullptr ) return false;

	m_pSourceVoice->SetFrequencyRatio( max( min( m_fPitch, m_fMaxPitch ), XAUDIO2_MIN_FREQ_RATIO ) );
	return true;
}

//----------------------------.
// ピッチを取得.
//----------------------------.
const float CXAudio2PlayBGM::GetPitch()
{
	float pitch = XAUDIO2_MIN_FREQ_RATIO;
	if ( m_pSourceVoice == nullptr ) return pitch;

	m_pSourceVoice->GetFrequencyRatio( &pitch );
	return pitch;
}

//----------------------------.
// BGMをフェードアウトする関数.
// この関数は別スレッドから呼び出す.
//----------------------------.
bool CXAudio2PlayBGM::FadeOutBGM( float value, const bool& isGameEnd )
{
	// 音量が0より大きい間ループ.
	while ( GetVolume() > 0.0f )
	{
		if ( isGameEnd			== true		) return true;
		if ( m_FadeOutStartFlag == false	) return true;

		// 音量調整.
		AdjustVolume( value );
		Sleep( 1 );
	}

	Stop();
	return true;
}

//----------------------------.
// BGMをフェードインする関数.
// この関数は別スレッドから呼び出す.
//----------------------------.
bool CXAudio2PlayBGM::FadeInBGM( float value, const bool& isGameEnd )
{
	// 音量が、最大BGM音量 * 最大Master音量より小さい間ループ.
	const auto& volume = SoundManager::GetSoundVolume();
	while ( GetVolume() < m_fSoundVolume * volume.BGM * volume.Master )
	{
		if ( isGameEnd			== true		) return true;
		if ( m_FadeInStartFlag	== false	) return true;

		// 音量調整.
		AdjustVolume( value );
		Sleep( 1 );
	}

	SetBGMVolume();
	return true;
}

//---------------------------.
// ソースボイスの解放.
//----------------------------.
void CXAudio2PlayBGM::DestoroySource()
{
	if ( m_pSourceVoice != nullptr ) {
		// ソースボイスの解放.
		m_pSourceVoice->DestroyVoice();
		m_pSourceVoice = nullptr;
	}
	else {
		// ソースボイスが存在しないのでリターン.
		return;
	}

	// バッファリリース.
	for ( int i = 0; i < m_Buff_len; i++ ) {
		SAFE_DELETE( m_Buff[i] );
	}
	SAFE_DELETE( m_Buff );
}
#endif // ENABLE_SOUND
