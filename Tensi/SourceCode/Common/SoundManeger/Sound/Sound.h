#pragma once
#include "..\..\..\Global.h"

#pragma comment(lib, "winmm.lib") //音楽再生で必要.

/************************************************
*	サウンドクラス.
*	midi,mp3,wav形式ファイルの再生・停止を行う.
*		﨑田友輝.
**/
class CSound final
{
public:
	static const int STR_BUFF_MAX = 256;//文字列バッファ.

public:
	CSound();
	~CSound();

	// 音声ファイルを開く.
	bool Open( LPCTSTR sFileName, LPCTSTR sAlias, HWND hWnd, int Volume = 1000 );

	// 音声ファイルを閉じる.
	bool Close();

	// 音声を再生する.
	bool Play( bool bNotify = false );
	// 音声を逆再生する.
	bool ReversePlay( bool bNotify = false );
	// 音声を再生する(SEで使う).
	bool PlaySE( bool StopPlay = false, bool IsReverse = false );
	// 音声を再生する(ループ).
	bool PlayLoop( bool IsReverse = false );

	// 音声を停止する.
	bool Stop();
	// 音声を一時停止する.
	bool Pause();

	// 状態を取得する.
	bool GetStatus( LPTSTR sStaus);

	// 音声の停止を確認する.
	bool IsStopped();

	// 再生位置を最初に戻す.
	bool SeekToStart();

	// 初期設定.
	void SetInitParam( LPCTSTR sAlias, HWND hWnd,int Volume);

	// 左右のオーディオのON/OFF.
	bool SetIsAudioAllON( bool Flag );
	// 左オーディオのON/OFF.
	bool SetIsAudioLeftON( bool Flag );
	// 右オーディオのON/OFF.
	bool SetIsAudioRightON( bool Flag );

	// 音量を設定する.
	//	0 ~ 1000(通常).
	void	ResetVolume();
	bool	SetVolume( int Volume );
	bool	AddVolume( int Volume );

	// 再生速度を設定する.
	//	0 ~ 1000(通常) ~ 2000.
	void	ResetPlaySpeed();
	bool	SetPlaySpeed( int Speed );
	bool	AddPlaySpeed( int Speed );

	// 音量を取得.
	int		GetVolume	() { return m_Volume;		}
	int		GetMaxVolume() { return m_MaxVolume;	}

	// 再生速度の取得.
	int		GetPlaySpeed() { return m_PlaySpeed;	}

private:
	HWND	m_hWnd;					//ウィンドウハンドル.
	TCHAR	m_sAlias[STR_BUFF_MAX];	//エイリアス.
	int		m_Volume;				//音量(0~1000).
	int		m_MaxVolume;			//最大音量Open時に設定.
	int		m_PlaySpeed;			//再生速度.
};