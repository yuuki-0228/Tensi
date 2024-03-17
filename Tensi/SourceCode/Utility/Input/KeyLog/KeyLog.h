#pragma once
#include "..\..\..\Global.h"
#include <string>
#include <vector>

/************************************************
*	キーの入力ログクラス.
*		﨑田友輝.
**/
class CKeyLog final
{
public:
	CKeyLog	();
	CKeyLog	( const bool IsPlay );
	~CKeyLog();

	// ログを出力する.
	void Output();

	// 押されたキーを保存する.
	void AddKeyState( const int KeyNo );

	// 全てのキーログを読み込む.
	void ReadKeyLog();

	// このフレームのキーログを読み込む.
	int ReadFrameKey();

	// 読み込むフレームのカウントを増やす.
	void AddFrameCnt();

private:
	std::vector<std::string> m_KeyLog;	// キーログを保存.
	int		m_OldFrameKeyStatus;		// 前回の押したキーを保存する.
	int		m_FrameKeyStatus;			// 1フレームの押したキーを保存する.
	int		m_RoadNumCnt;				// キーログの何個目を取得するかのカウント.
	int		m_Chain;					// 連続する個数.
	bool	m_KeyLogPlay;				// キーログを再生中か.
};