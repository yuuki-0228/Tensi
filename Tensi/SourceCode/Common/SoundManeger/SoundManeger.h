#pragma once
#include "..\..\Global.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <filesystem>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class CSound;
class CObject;

/************************************************
*	サウンドマネージャークラス.
**/
class SoundManager final
{
public:
	using Sound_map			= std::unordered_map<std::string, std::vector<std::shared_ptr<CSound>>>;
	using Sound_Time_map	= std::unordered_map<std::string, ULONGLONG>;
	using Sound_List		= std::vector<std::string>;

public:
	SoundManager();
	~SoundManager();

	// オーディオインターフェースの構築.
	static HRESULT Create();
	// サウンドの読み込み.
	static HRESULT SoundLoad( HWND hWnd );

	// リスナーを設定.
	static void	SetListener( CObject* pObj ) { GetInstance()->m_pListener = pObj; }
	// 選ばれる番号を設定.
	static void SetSelectNo( const int No ) { GetInstance()->m_SelectNo = No; }
	// 選ばれる番号を初期化.
	static void ResetSelectNo() { GetInstance()->m_SelectNo = -1; }

	// ピーク地点の取得.
	static float GetPeakValue();

	// 再生.
	static void	PlaySE(				std::string Name,							float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // SEを再生する.
	static void	PlaySELoop(			std::string Name,																				 bool IsMsgStop = false ); // SEを再生する.
	static void	PlaySE3D(			std::string Name, D3DXPOSITION3 PlayPos,	float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // 3DでSEを再生する.
	static void	PlaySELoop3D(		std::string Name, D3DXPOSITION3 PlayPos,														 bool IsMsgStop = false ); // 3DでSEを再生する.
	static void	PlayBGM(			std::string Name,							bool	OldBGMStop	= true,							 bool IsMsgStop = false ); // BGMを再生する.
	static void	PlayFadeBGM(		std::string Name,							int		FadeSpeed	= 1,					 		 bool IsMsgStop = false ); // BGMをフェードで切り替えて再生する.
	static void	PlayBGM3D(			std::string Name, D3DXPOSITION3 PlayPos,	bool	OldBGMStop	= true, 						 bool IsMsgStop = false ); // 3DでBGMを再生する.

	// 逆再生.
	static void	ReversePlaySE(		std::string Name,							float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // SEを再生する.
	static void	ReversePlaySE3D(	std::string Name, D3DXPOSITION3 PlayPos,	float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // 3DでSEを再生する.
	static void	ReversePlayBGM(		std::string Name,							bool	OldBGMStop	= true,							 bool IsMsgStop = false ); // BGMを再生する.
	static void	ReversePlayFadeBGM(	std::string Name,							int		FadeSpeed	= 1,					 		 bool IsMsgStop = false ); // BGMをフェードで切り替えて再生する.
	static void	ReversePlayBGM3D(	std::string Name, D3DXPOSITION3 PlayPos,	bool	OldBGMStop	= true, 						 bool IsMsgStop = false ); // 3DでBGMを再生する.

	// 停止.
	static void	StopSE(		std::string Name,	bool IsMsgStop = false ); // 指定のSEを停止させる.
	static void	StopBGM(	std::string Name,	bool IsMsgStop = false ); // 指定のBGMを停止させる.
	static void	BGMAllStop(						bool IsMsgStop = false ); // BGMを全て停止させる.
	static void	SetIssLoopStop( bool Flag,		bool IsMsgStop = false ); // ループ再生を停止させるか.

	// 一時停止.
	static void	PauseSE(	std::string Name,	bool IsMsgStop = false ); // 指定のSEを一時停止させる.
	static void	PauseBGM(	std::string Name,	bool IsMsgStop = false ); // 指定のBGMを一時停止させる.
	static void	BGMAllPause(					bool IsMsgStop = false ); // BGMを全て一時停止させる.

	// 左右のオーディオのON/OFF.
	static void SetIsBGMAllON( std::string Name, bool Flag );
	static void SetIsSEAllON(  std::string Name, bool Flag );
	// 左オーディオのON/OFF.
	static void SetIsBGMLeftON( std::string Name, bool Flag );
	static void SetIsSELeftON(  std::string Name, bool Flag );
	// 右オーディオのON/OFF.
	static void SetIsBGMRightON( std::string Name, bool Flag );
	static void SetIsSERightON(  std::string Name, bool Flag );

	// 音量の設定.
	//	0 ~ 1000(通常).
	static void ResetSEVolume(	std::string Name );
	static void SetSEVolume(	std::string Name, int	Volume );
	static void SetSEVolume(	std::string Name, float	Volume );
	static void AddSEVolume(	std::string Name, int	Volume );
	static void AddSEVolume(	std::string Name, float Volume );
	static void ResetBGMVolume( std::string Name );
	static void SetBGMVolume(	std::string Name, int	Volume );
	static void SetBGMVolume(	std::string Name, float	Volume );
	static void AddBGMVolume(	std::string Name, int	Volume );
	static void AddBGMVolume(	std::string Name, float Volume );


	// 再生速度の設定.
	//	0 ~ 1000(通常) ~ 2000.
	static void ResetSEPlaySpeed(	std::string Name );
	static void SetSEPlaySpeed(		std::string Name, int Speed );
	static void AddSEPlaySpeed(		std::string Name, int Speed );
	static void ResetBGMPlaySpeed(	std::string Name );
	static void SetBGMPlaySpeed(	std::string Name, int Speed );
	static void AddBGMPlaySpeed(	std::string Name, int Speed );

private:
	// インスタンスの取得.
	static SoundManager* GetInstance();

	// サウンドファイルを開く.
	static void OpenSound(
		std::vector<std::shared_ptr<CSound>>* List,	const HWND&			hWnd,
		const int			PlayMax,				const int			MaxVolume,
		const std::string&	FileName,				const std::wstring& wFilePath );

private:
	IMMDeviceEnumerator*	m_pEnumerator;	// ピーク地点の取得で使用.
	IMMDevice*				m_pDevice;		// ピーク地点の取得で使用.
	IAudioMeterInformation* m_pMeterInfo;	// ピーク地点の取得で使用.
	CObject*				m_pListener;	// リスナー(3D再生で使用する).
	Sound_map				m_BGMList;		// サウンド(BGM)リスト.
	Sound_map				m_SEList;		// サウンド(SE)リスト.
	Sound_Time_map			m_PlayTimeList;	// 前回再生してからどんだけ経ったか.
	Sound_List				m_BGMNames;		// BGM名前リスト.
	Sound_List				m_SENames;		// SE名前リスト.
	std::string				m_NowBGMName;	// 現在のBGM名.
	std::mutex				m_Mutex;
	bool					m_IsLoadEnd;	// 読み込みが終わったか.
	bool					m_LoopStopFlag;	// ループの停止させる.
	int						m_SelectNo;		// 選択させる番号.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	SoundManager( const SoundManager& )				= delete;
	SoundManager& operator = ( const SoundManager& )	= delete;
	SoundManager( SoundManager&& )					= delete;
	SoundManager& operator = ( SoundManager&& )		= delete;
};

