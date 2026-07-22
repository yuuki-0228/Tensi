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
*	XAudio2でのVoice再生を目的としたクラス.
**/
class CXAudio2PlayVoice
{
private:
	static const int VOICE_VOICE_SIZE = 3;	// Voiceの予備の数.

public:
	CXAudio2PlayVoice();
	~CXAudio2PlayVoice();

	// サウンドの作成
	HRESULT CreateOggSound( std::shared_ptr<COggLoad> pOggData, json audioData, const char* filename );
	// SoundSourceを破壊する関数.
	void DestoroySource();

	// 多重再生可能なサウンド再生.
	virtual bool Play( std::shared_ptr<COggLoad> pWavData );
	// 多重再生しないサウンド再生.
	bool NoMultiplePlay( std::shared_ptr<COggLoad> pWavData );

	// Voiceサウンド停止.
	bool VoiceStop( const size_t ArrayNum );
	// 作成してる予備分のSEも停止.
	bool AllVoiceStop();

	// ピッチ設定関数.
	bool SetPitch( float value );
	// ピッチを取得関数.
	const float GetPitch();

	// Voiceが再生中かを返す.
	const bool IsPlayingVoice( const size_t ArrayNum );

//----------------------------.
//	基本音量操作系.
//----------------------------.
	// Voiceの音量を設定.
	bool SetVoiceVolume( const float& fVolume, size_t Array );
	bool SetVoiceVolume( size_t Array );
	bool SetVoiceVolume();

	// Voiceの音量を取得します.
	const float GetVoiceVolume( size_t Array );

protected:
	int CheckPlayVoiceBuffer();

protected:
	COggLoad*				m_pOggData;							// Oggデータロードクラス.

	IXAudio2SourceVoice*	m_pSourceVoice[VOICE_VOICE_SIZE];	// 同じVoiceを同時にならせるように配列にする.
	int						m_CheckNonePlayVoiceVoice;			// 再生していないVoiceを洗い出す用.
	int						m_RepeatPlayVoice;					// 全部が再生中の時に次に巻き戻して再生する配列番号を返す.		
	float					m_fMaxPitch;						// 最大ピッチ.
	float					m_fPitch;							// ピッチ.

	float					m_fVoiceVolume;						// Voice音量.
};
#endif // ENABLE_SOUND
