#pragma once
#include "..\..\Global.h"
#include <thread>
#include <mutex>
#include <memory>
#include <vector>

/************************************************
*	ロードマネージャー.
*		﨑田友輝.
**/
class CLoadManager final
{
public:
	CLoadManager();
	~CLoadManager();

	// 読み込み.
	void LoadResource( HWND hWnd );

	// スレッドの解放.
	bool ThreadRelease();

	// ロード失敗したか.
	inline bool IsLoadFailed() const { return m_isLoadFailed; }
	// ロードが終了したか.
	inline bool	IsLoadEnd() const { return m_isThreadJoined; }

private:
	std::thread m_Thread;		// スレッド.
	std::mutex	m_Mutex;
	bool m_isLoadEnd;			// ロードが終了したか.
	bool m_isThreadJoined;		// スレッドが解放されたか.
	bool m_isLoadFailed;		// ロード失敗.
};
