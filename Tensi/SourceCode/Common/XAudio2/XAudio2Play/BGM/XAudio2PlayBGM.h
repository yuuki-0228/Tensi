#pragma once
#include "..\..\..\..\SystemSettings.h"
#ifdef ENABLE_SOUND
#include <vector>
#include <mutex>
#include "..\..\XAudio2.h"
#include "..\..\XAudio2Master\XAudio2MasterVoice.h"
#include "..\..\OggLoad\OggLoad.h"
#include "..\..\XAudio2CallBack\VoiceCallBack.h"
#include "..\..\..\..\Utility\FileManager\FileManager.h"

struct IXAudio2SourceVoice;

/************************************************
*	XAudio2でのBGMストリーミング再生を目的としたクラス.
**/
class CXAudio2PlayBGM
{
public:
	CXAudio2PlayBGM();
	~CXAudio2PlayBGM();

	// Oggからソースボイスを作成する.
	HRESULT CreateOggSound( std::shared_ptr<COggLoad> pOggData, json audioData, const char* filename );
	// SoundSourceを破壊する関数.
	void DestoroySource();

	// サウンド再生、前に一時停止されていた場合は記憶位置から再開.
	bool Play( std::shared_ptr< COggLoad> pWavData, const char* filename, bool& isEnd );
	// BGMサウンドを完全停止.(位置を保持しない).
	bool Stop();

	// サウンド一時停止.
	bool Pause();
	// サウンド再生を再開.
	bool PlayStart();

	// ピッチ設定関数.
	bool SetPitch( float value );
	// ピッチを取得関数.
	const float GetPitch();
	
	// 再生してるかどうか取得関数.
	const bool IsPlaying();

//----------------------------.
// 基本音量操作系.
//----------------------------.
	// BGMの音量を設定します.
	bool SetBGMVolume( float value );
	bool SetBGMVolume();
	// BGMの音量を取得します.
	const float GetVolume();
	// 現在の音量から加算または減算.
	bool AdjustVolume( float value );

	// フェードアウトフラグのセッター.
	void SetFadeOutFlag( bool BGMFadeOutFlag ) { m_FadeOutStartFlag = BGMFadeOutFlag; }
	// フェードアウトフラグのゲッター.
	bool GetFadeOutFlag() { return m_FadeOutStartFlag; }

	// フェードインフラグのセッター.
	void SetFadeInFlag( bool BGMFadeInFlag ) { m_FadeInStartFlag = BGMFadeInFlag; }
	// フェードインフラグのゲッター.
	bool GetFadeInFlag() { return m_FadeInStartFlag; }

protected:
	// BGMをフェードアウトする関数.
	bool FadeOutBGM( float value, const bool& isGameEnd );
	// BGMをフェードインする関数.
	bool FadeInBGM( float value, const bool& isGameEnd );
	// 音源データをストリームに流し込む.
	HRESULT OggSubmit( std::shared_ptr<COggLoad> pOggData, const char* filename );

private:
	// フェードアウト用のスレッドを起動する.
	void CallFadeOutBGM( bool& isEnd );
	// フェードイン用のスレッドを起動する.
	void CallFadeInBGM( bool& isEnd );

protected:
	std::mutex				m_mtx;
	VoiceCallback			m_Callback;				// コールバック.
	XAUDIO2_BUFFER			m_Buffer;				// オーディオバッファー
	int						m_Buff_len;				// バッファレンジ.
	BYTE**					m_Buff;					// バッファサイズ.
	int						m_Len;					// レンジ.
	int						m_Buff_cnt;				// バッファカウント.
	int						m_Size;
	IXAudio2SourceVoice*	m_pSourceVoice;			// サウンドを再生するストリーム.
	std::thread				m_FadeInThread;			// ボリュームフェードイン用スレッド.
	std::thread				m_FadeOutThread;		// ボリュームフェードアウト用スレッド.
	bool					m_isMoveFadeInThread;	// フェードインスレッドが動いているか.
	bool					m_isMoveFadeOutThread;	// フェードアウトスレッドが動いているか.
	float					m_fSoundVolume;			// 音量
	float					m_fMaxPitch;			// 最大ピッチ.
	float					m_fPitch;				// ピッチ.
	bool					m_FadeOutStartFlag;		// フェードアウト開始フラグ.
	bool					m_FadeInStartFlag;		// フェードイン開始フラグ.
	float					m_fFadeInValue;			// フェードインの値.
	float					m_fFadeOutValue;		// フェードアウトの値.
	bool					m_FirstPlayFlag;		// 初回再生かどうか.
	bool					m_isAudioStop;			// 停止中か
};
#endif // ENABLE_SOUND
