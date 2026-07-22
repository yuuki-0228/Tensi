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
*	XAudio2でのSE再生を目的としたクラス.
**/
class CXAudio2PlaySE
{
private:
	static const int SE_VOICE_SIZE = 10;	// SEの予備の数.

public:
	CXAudio2PlaySE();
	~CXAudio2PlaySE();

	// サウンドの作成
	HRESULT CreateOggSound( std::shared_ptr<COggLoad> pOggData, json audioData, const char* filename );
	// SoundSourceを破壊する関数.
	void DestoroySource();

	// 多重再生可能なサウンド再生.
	virtual bool Play( std::shared_ptr<COggLoad> pWavData );
	// 多重再生しないサウンド再生.
	bool NoMultiplePlay( std::shared_ptr<COggLoad> pWavData );

	// SEサウンド停止.
	bool SeStop( const size_t ArrayNum );
	// 作成してる予備分のSEも停止.
	bool AllSeStop();

	// ピッチ設定関数.
	bool SetPitch( float value );
	// ピッチを取得関数.
	const float GetPitch();

	// SEが再生中かを返す.
	const bool IsPlayingSE( const size_t ArrayNum );

//----------------------------.
//	基本音量操作系.
//----------------------------.
	// SEの音量を設定.
	bool SetSEVolume( const float& fVolume, size_t Array );
	bool SetSEVolume( size_t Array );
	bool SetSEVolume();

	// SEの音量を取得します.
	const float GetSEVolume( size_t Array );

protected:
	int CheckPlaySeBuffer();

protected:
	COggLoad*				m_pOggData;							// Oggデータロードクラス.

	IXAudio2SourceVoice*	m_pSourceVoice[SE_VOICE_SIZE];		// 同じSEを同時にならせるように配列にする.
	int						m_CheckNonePlaySeVoice;				// 再生していないSEを洗い出す用.
	int						m_RepeatPlaySe;						// 全部が再生中の時に次に巻き戻して再生する配列番号を返す.		
	float					m_fMaxPitch;						// 最大ピッチ.
	float					m_fPitch;							// ピッチ.

	float					m_fSEVolume;						// SE音量.
};
#endif // ENABLE_SOUND
