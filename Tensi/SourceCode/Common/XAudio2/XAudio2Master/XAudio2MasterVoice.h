#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_SOUND
#include "XAudio2.h"
#define COM_SAFE_RELEASE( p ) { if(p) { (p)->Release(); (p) = nullptr; } }

/************************************************
*	XAduio2の再生に必要なマスターボイスクラス.
**/
class CXAudio2MasterVoice
{
public:
	CXAudio2MasterVoice();
	~CXAudio2MasterVoice();

	// インスタンスの取得.
	static CXAudio2MasterVoice* GetInstance();
	static CXAudio2MasterVoice& GetGlobalSystem();

	IXAudio2* GetInterface() const;

	// マスターボイス取得.
	IXAudio2MasteringVoice* GetMasteringVoice() const;

	// 解放関数.
	static void Release();

protected:
	// 初期化.
	bool Initialize();

	IXAudio2* m_pXAudioInterface;				// XAudio2インターフェース
	IXAudio2MasteringVoice* m_pMasterVoice;		// マスタボイス

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	CXAudio2MasterVoice( const CXAudio2MasterVoice& ) = delete;
	CXAudio2MasterVoice& operator = ( const CXAudio2MasterVoice& ) = delete;
	CXAudio2MasterVoice( CXAudio2MasterVoice&& ) = delete;
	CXAudio2MasterVoice& operator = ( CXAudio2MasterVoice&& ) = delete;
};
#endif // ENABLE_SOUND
