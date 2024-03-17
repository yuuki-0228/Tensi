#pragma once
#include "..\..\Global.h"
#include <thread>
#include <mutex>
#include <memory>
#include <vector>

/************************************************
*	���[�h�}�l�[�W���[.
*		���c�F�P.
**/
class CLoadManager final
{
public:
	CLoadManager();
	~CLoadManager();

	// �ǂݍ���.
	void LoadResource( HWND hWnd );

	// �X���b�h�̉��.
	bool ThreadRelease();

	// ���[�h���s������.
	inline bool IsLoadFailed() const { return m_isLoadFailed; }
	// ���[�h���I��������.
	inline bool	IsLoadEnd() const { return m_isThreadJoined; }

private:
	std::thread m_Thread;		// �X���b�h.
	std::mutex	m_Mutex;
	bool m_isLoadEnd;			// ���[�h���I��������.
	bool m_isThreadJoined;		// �X���b�h��������ꂽ��.
	bool m_isLoadFailed;		// ���[�h���s.
};
