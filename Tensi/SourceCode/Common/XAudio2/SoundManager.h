#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_SOUND
#include <unordered_map>
#include <iostream>
#include <thread>
#include "XAudio2Play\BGM\XAudio2PlayBGM.h"
#include "XAudio2Play\SE\XAudio2PlaySE.h"
#include "XAudio2Play\Voice\XAudio2PlayVoice.h"
#include "OggLoad/OggLoad.h"
#include "..\..\Utility\FileManager\FileManager.h"
#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#endif

 /************************************************
 *	XAudio2サウンドマネージャークラス.
 **/
class SoundManager
{
public:
	// サウンド音量構造体.
	struct stSoundVolume
	{
		float Master	= 1.0f;
		float BGM		= 1.0f;
		float SE		= 1.0f;
		float Voice		= 1.0f;
	} typedef SSoundVolume;

public:
	SoundManager();
	~SoundManager();

	// サウンドデータ作成.
	static void CreateSoundData();
	// デバッグの更新
	static void DebugUpdate();
	// 解放処理関数.
	static void Release();

	// サウンド音量データを保存
	static void SaveSoundVolumeData();

//----------------------------.
// 音量.
//----------------------------.
	// ゲーム全体のBGMの最大音量を取得.
	inline static const SSoundVolume GetSoundVolume() { return GetInstance()->m_SoundVolume; }

	// マスター音量セット.
	static void SetSoundVolume( const SSoundVolume& volume );
	// 音量を変更できるスレッドのステート変更.
	static void StateChangeVolumeThread( const bool& bFlag );
	// オプション用スレッド動作済みかどうかのフラグ取得関数.
	inline static const bool GetCreateOptionThread() { return GetInstance()->m_isCreateThread; }
	// 音量の更新
	static void SoundVolumeUpdate();

//----------------------------.
// BGM.
//----------------------------.
	// スレッドを作ってBGMを再生する関数(外部呼出).
	static void PlayBGM( const std::string& Name, const bool& LoopFlag = true );
	// BGM一時停止関数.
	static void PauseBGM( const std::string Name );
	// BGM再再生関数
	static void AgainPlayBGM( const std::string Name );
	// BGM停止関数.
	static void StopBGM( const std::string Name );
	// BGMフェードアウト関数.
	static void FadeOutBGM( const std::string Name );
	// 指定BGMの音量のフェードアウト実行中かを返す.
	static const bool GetFadeOutBGM( const std::string Name );
	// BGMフェードイン関数.
	static void FadeInBGM( const std::string Name );
	// 指定BGMの音量のフェードイン実行中かを返す.
	static const bool GetFadeInBGM( const std::string Name );
	// 指定した名前のBGMスレッドが再生中かどうかを返す : 再生中ならtrue.
	static const bool GetIsPlayBGM( const std::string BGMName );
	// BGMのピッチ設定.
	static void SetBGMPitch( const std::string Name, const float Value );
	// BGMを全て止めて、スレッドを放棄する関数.
	static void StopAllBGM();

//----------------------------.
// SE.
//----------------------------.
	// SE再生関数.
	static void PlaySE( const std::string& Name );
	// 多重再生しないSE再生関数.
	static void NoMultipleSEPlay( const std::string& Name );
	// SE停止関数.
	static void StopSE( const std::string Name, const size_t ArrayNum );
	// 同じ音源の全てのSEソース停止関数.
	static void StopAllSE();
	static void StopAllSE( const std::string Name );
	// 指定した名前のSEが再生中かどうかを返す : 再生中ならtrue.
	static bool GetIsPlaySE( const std::string Name, const size_t ArrayNum );

//----------------------------.
// Voice.
//----------------------------.
	// Voice再生関数.
	static void PlayVoice( const std::string& Name );
	// 多重再生しないVoice再生関数.
	static void NoMultipleVoicePlay( const std::string& Name );
	// Voice停止関数.
	static void StopVoice( const std::string Name, const size_t ArrayNum );
	// 同じ音源の全てのVoiceソース停止関数.
	static void StopAllVoice();
	static void StopAllVoice( const std::string Name );
	// 指定した名前のVoiceが再生中かどうかを返す : 再生中ならtrue.
	static bool GetIsPlayVoice( const std::string Name, const size_t ArrayNum );

//----------------------------.
// 設定.
//----------------------------.
	// サウンドログを送らなくする
	inline static void SoundLogStop() { GetInstance()->m_LogStopFlag = true; }
	// サウンドログを送るようにする
	inline static void SoundLogPlay() { GetInstance()->m_LogStopFlag = false; }
	// サウンドログを送るか
	inline static bool GetIsSoundLogStop() { return GetInstance()->m_LogStopFlag; }

//----------------------------.
// その他.
//----------------------------.
#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
	// ピーク地点の取得.
	static float GetPeakValue();
#endif

private:
	// インスタンスの取得.
	static SoundManager* GetInstance();

	// BGMを止めて、スレッドを放棄する関数.
	static const bool StopBGMThread( const std::string BGMName );
	// 音量を変更するスレッドを立てる.
	static void SetChangeVolumeThread();
	// 音量初期設定関数.
	static void VolumeInit();
	// 存在などのチェック.
	static bool CheckBGMDataIsTrue( const std::string Name );

	// 音量を変更するためのスレッドを立ち上げる.
	static void CreateChangeSoundVolumeThread();
	// 音量を変更するためのスレッドを解放する.
	static bool ReleaseChangeSoundVolumeThread();

	// サウンドデータの読み込み
	static json LoadSoundDataFile( const std::string filePath );
	// サウンドデータの作成
	static HRESULT CreateSoundDataFile( const std::string filePath );

#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
	// オーディオインターフェースの初期化
	static HRESULT AudioInterfaceInit();
#endif

private:
	std::unordered_map<std::string, std::shared_ptr<COggLoad>>			m_pOggWavData;		// Oggデータクラスの箱
	std::unordered_map<std::string, std::shared_ptr<CXAudio2PlayBGM>>	m_pBgmSource;		// BGM用PlaySoundクラスの箱.
	std::unordered_map<std::string, std::thread>						m_pBGMThread;		// BGMストリーミング再生用スレッド.
	std::unordered_map<std::string, bool>								m_bisEndThread;		// スレッド内で再生しているBGMを停止するためのフラグ.
	std::unordered_map<std::string, bool>								m_bisThreadRelease;	// スレッドリリース用関数.
	std::unordered_map<std::string, std::thread::id>					m_InThreadID;		// 各スレッドID保持.
	std::vector<std::string>											m_BGMNameList;		// BGMの名前リスト(解放時に必要).
	std::vector<std::string>											m_SENameList;		// SEの名前リスト(解放時に必要).
	std::vector<std::string>											m_VoiceNameList;	// Voiceの名前リスト(解放時に必要).
	std::unordered_map<std::string, std::shared_ptr<CXAudio2PlaySE>>	m_pSeSource;		// SE用PlaySoundクラスの箱.
	std::unordered_map<std::string, std::shared_ptr<CXAudio2PlayVoice>>	m_pVoiceSource;		// Voice用PlaySoundクラスの箱.
	bool																m_EndGameFlag;		// 終了用フラグ.
	bool																m_EndCreateFlag;	// BGM,SEの作成が終わった後かどうか.
	bool																m_LogStopFlag;		// ログを表示しなくするか.
	SSoundVolume														m_SoundVolume;
#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
	IMMDeviceEnumerator*												m_pEnumerator;		// ピーク地点の取得で使用.
	IMMDevice*															m_pDevice;			// ピーク地点の取得で使用.
	IAudioMeterInformation*												m_pMeterInfo;		// ピーク地点の取得で使用.
#endif

//----------------------------.
// 以下オプション画面用.
// 以下二つはスレッドで回さないとストリーミングの入れ込みを待ってからの変更になるためスレッドを回す.
//----------------------------.
	std::thread m_SoundSourceVolume;			// オプション画面で、音量を変更するためのスレッド.
	bool		m_MoveSoundVolumeThreadFlag;	// オプション画面で、BGMを設定と同時に変更するためのスレッド用変数.
	bool		m_isCreateThread;				// 音量変更のスレッドが動いているかどうか.
//----------------------------.
// スレッドロック用.
//----------------------------.
	bool		m_ResumeSoundVolumeThreadFlag;	// 条件変数.
	std::mutex	m_SoundVolumemtx;
	std::condition_variable m_SoundVolumeCv;
//----------------------------.
// デバッグ用.
//----------------------------.
#ifdef _DEBUG
	bool		m_DebugSoundVolumeChange;
	std::string	m_DebugTestBGMName;
	std::string	m_DebugTestVoiceName;
	std::string	m_DebugTestSEName;
#endif

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	SoundManager( const SoundManager& ) = delete;
	SoundManager& operator = ( const SoundManager& ) = delete;
	SoundManager( SoundManager&& ) = delete;
	SoundManager& operator = ( SoundManager&& ) = delete;
};
#endif // ENABLE_SOUND
