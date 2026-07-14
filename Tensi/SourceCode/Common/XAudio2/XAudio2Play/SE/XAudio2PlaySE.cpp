#include "XAudio2PlaySE.h"
#ifdef ENABLE_SOUND
#include "XAudio2.h"
#include <vector>
#include <process.h>
#include <thread>
#include "..\..\XAudio2Master\XAudio2MasterVoice.h"
#include "..\..\SoundManager.h"

namespace {
	constexpr float	MAX_PITCH = 5.0f;		// 最大ピッチ(最高5.0f).
}

CXAudio2PlaySE::CXAudio2PlaySE()
	: m_pOggData				( nullptr )
	, m_CheckNonePlaySeVoice	( 0 )
	, m_RepeatPlaySe			( 0 )
	, m_fMaxPitch				( MAX_PITCH )
	, m_fPitch					( 1.0f )
	, m_fSEVolume				( 1.0f )
{
	m_pSourceVoice[0] = nullptr;
}

CXAudio2PlaySE::~CXAudio2PlaySE()
{
}

//----------------------------.
// 再生されていないソースボイスを探す.
//----------------------------.
int CXAudio2PlaySE::CheckPlaySeBuffer()
{
	XAUDIO2_VOICE_STATE state;

	// 再生していないSEVoiceを探す.
	for ( int Array = 0; Array < SE_VOICE_SIZE; Array++ ) {
		m_pSourceVoice[Array]->GetState( &state );
		// バッファが0より小さい.
		if ( state.BuffersQueued < 0 ) return Array;	// 再生されていない配列の値を返す.
	}

	// すべてサウンド再生途中なので、リピート再生する.
	if ( m_RepeatPlaySe < SE_VOICE_SIZE ) m_RepeatPlaySe++;
	// 最大数と同じになったら0にする.
	if ( m_RepeatPlaySe == SE_VOICE_SIZE ) m_RepeatPlaySe = 0;

	return m_RepeatPlaySe;
}

//----------------------------.
// 指定された配列番号のSEが再生されているか.
//----------------------------.
const bool CXAudio2PlaySE::IsPlayingSE( size_t ArrayNum )
{
	// SoundSourceがない場合リターン 0(再生していないときと同じ).
	if ( m_pSourceVoice[ArrayNum] == nullptr ) return false;

	// SoundSourceのステート取得.
	XAUDIO2_VOICE_STATE xState;
	m_pSourceVoice[ArrayNum]->GetState( &xState );

	return xState.BuffersQueued != 0;
}

//----------------------------.
// サウンド再生.
//----------------------------.
bool CXAudio2PlaySE::Play( std::shared_ptr<COggLoad> pWavData )
{
	if ( m_pSourceVoice[0] == nullptr ) return false;

	// 再生されていないソースボイスの配列番号を取得.
	int Array = CheckPlaySeBuffer();

	// SEの音量セット.
	SetPitch( m_fPitch );
	SetSEVolume( Array );

	m_pSourceVoice[Array]->Stop();					// ソースボイス停止.
	m_pSourceVoice[Array]->FlushSourceBuffers();	// ソースバッファをクリア.
	m_pSourceVoice[Array]->Start();					// ソースボイス再生.

	// バッファーを生成する
	XAUDIO2_BUFFER seBuffer = { 0 };
	seBuffer.pAudioData = pWavData->GetSEWaveBuffer();
	seBuffer.AudioBytes = pWavData->GetSEFileSize();

	// キューにバッファーを投入する
	HRESULT hr;
	if ( FAILED( hr = m_pSourceVoice[Array]->SubmitSourceBuffer( &seBuffer ) ) )
	{
		return false;
	}
	return true;
}

//----------------------------.
// SEを多重再生しないで再生.
// #もとからなっているSEを止めて一つにするものではない.
//----------------------------.
bool CXAudio2PlaySE::NoMultiplePlay( std::shared_ptr<COggLoad> pWavData )
{
	// このSEが再生されていなければ中に入り再生する.
	if ( IsPlayingSE( 0 ) == false ) {
		if ( m_pSourceVoice[0] == nullptr ) return true;
		
		// ソースボイス再生.
		m_pSourceVoice[0]->Start();	
	}

	// SEの音量セット.
	SetPitch( m_fPitch );
	SetSEVolume( 0 );

	XAUDIO2_VOICE_STATE state;
	m_pSourceVoice[0]->GetState( &state );
	if ( state.BuffersQueued > 0 )	// バッファが0より大きい.
	{
		m_pSourceVoice[0]->Stop();					// ソースボイス停止.
		m_pSourceVoice[0]->FlushSourceBuffers();	// ソースバッファをクリア.
		m_pSourceVoice[0]->Start();					// ソースボイス再生.
	}

	// バッファーを生成する
	XAUDIO2_BUFFER seBuffer = { 0 };
	seBuffer.pAudioData = pWavData->GetSEWaveBuffer();
	seBuffer.AudioBytes = pWavData->GetSEFileSize();

	// キューにバッファーを投入する
	HRESULT hr;
	if ( FAILED( hr = m_pSourceVoice[0]->SubmitSourceBuffer( &seBuffer ) ) )
	{
		return false;
	}
	return true;
}

//----------------------------.
// SE停止.
//----------------------------.
bool CXAudio2PlaySE::SeStop( const size_t ArrayNum )
{
	if ( m_pSourceVoice[ArrayNum] == nullptr ) return true;

	m_pSourceVoice[ArrayNum]->Stop( 0 );
	m_pSourceVoice[ArrayNum]->FlushSourceBuffers();
	return true;
}

//----------------------------.
// 作成してる予備含む全てのSE停止.
//----------------------------.
bool CXAudio2PlaySE::AllSeStop()
{
	if ( m_pSourceVoice[0] == nullptr ) return true;

	for ( size_t Array = 0; Array < SE_VOICE_SIZE; Array++ ) {
		m_pSourceVoice[Array]->Stop( 0 );
		m_pSourceVoice[Array]->FlushSourceBuffers();
	}
	return true;
}

//----------------------------.
// Oggデータ作成.
//----------------------------.
HRESULT CXAudio2PlaySE::CreateOggSound( std::shared_ptr<COggLoad> pOggData, json audioData, const char* filename )
{
	// 一回データが作られていたらリターン.
	if ( m_pOggData != nullptr ) return S_OK;

	m_pOggData = pOggData.get();

	// XAudio2マスターボイスのインスタンス取得.
	CXAudio2MasterVoice& xAudioMaster = CXAudio2MasterVoice::GetGlobalSystem();
	IXAudio2*			 pHandle	  = xAudioMaster.GetInterface();

	// サウンドデータの反映
	m_fSEVolume		= audioData["SoundVolume"];
	m_fMaxPitch		= audioData["MaxPitch"];
	m_fPitch		= audioData["StartPitch"];

	// ソースボイスを作成する.
	HRESULT hr;
	for ( size_t Array = 0; Array < SE_VOICE_SIZE; Array++ ) {
		m_pSourceVoice[Array] = nullptr;
		if ( FAILED( hr = pHandle->CreateSourceVoice(
			&m_pSourceVoice[Array], m_pOggData->GetFormat(), 0, m_fMaxPitch ) ) )
		{
			// 失敗.
			_ASSERT_EXPR( "error %#X creating se source voice\n", hr );
			return E_FAIL;
		}
	}
	if ( FAILED( hr ) ) return E_FAIL;

	return S_OK;
}

//----------------------------.
// SEの音量を設定.
//----------------------------.
bool CXAudio2PlaySE::SetSEVolume( const float& fVolume, size_t Array )
{
	if ( m_pSourceVoice[Array] == nullptr ) return false;

	m_fSEVolume = fVolume;

	const auto& volume = SoundManager::GetSoundVolume();
	m_pSourceVoice[Array]->SetVolume( m_fSEVolume * volume.SE * volume.Master, 0 );
	return true;
}
bool CXAudio2PlaySE::SetSEVolume( size_t Array )
{
	return SetSEVolume( m_fSEVolume, Array );
}
bool CXAudio2PlaySE::SetSEVolume()
{
	bool result = false;
	for ( size_t Array = 0; Array < SE_VOICE_SIZE; Array++ ) {
		if ( SetSEVolume( m_fSEVolume, Array ) ) result = true;
	};
	return result;
}

//----------------------------.
// 音量取得.
//----------------------------.
const float CXAudio2PlaySE::GetSEVolume( size_t Array )
{
	if ( m_pSourceVoice[Array] == nullptr ) return 0.0f;

	float Volume = 0.0f;
	m_pSourceVoice[Array]->GetVolume( &Volume );
	return Volume;
}

//----------------------------.
// ピッチを設定.
//----------------------------.
bool CXAudio2PlaySE::SetPitch( float value )
{
	m_fPitch = value;
	for ( size_t Array = 0; Array < SE_VOICE_SIZE; Array++ ) {
		if ( m_pSourceVoice[Array] == nullptr ) return false;

		m_pSourceVoice[Array]->SetFrequencyRatio( max( min( m_fPitch, m_fMaxPitch ), XAUDIO2_MIN_FREQ_RATIO ) );
	}
	return true;
}

//----------------------------.
// ピッチを取得.
//----------------------------.
const float CXAudio2PlaySE::GetPitch()
{
	float pitch = XAUDIO2_MIN_FREQ_RATIO;
	if ( m_pSourceVoice[0] == nullptr ) return pitch;

	m_pSourceVoice[0]->GetFrequencyRatio( &pitch );
	return pitch;
}

//----------------------------.
// ソースボイス解放.
//----------------------------.
void CXAudio2PlaySE::DestoroySource()
{
	if ( m_pSourceVoice[0] != nullptr ) {
		for ( size_t Array = 0; Array < SE_VOICE_SIZE; Array++ ) {
			m_pSourceVoice[Array]->DestroyVoice();
			m_pSourceVoice[Array] = nullptr;
		}
	}
}
#endif // ENABLE_SOUND
