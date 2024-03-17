#pragma once
#include "..\..\..\Global.h"
#include <string>
#include <vector>

/************************************************
*	�L�[�̓��̓��O�N���X.
*		���c�F�P.
**/
class CKeyLog final
{
public:
	CKeyLog	();
	CKeyLog	( const bool IsPlay );
	~CKeyLog();

	// ���O���o�͂���.
	void Output();

	// �����ꂽ�L�[��ۑ�����.
	void AddKeyState( const int KeyNo );

	// �S�ẴL�[���O��ǂݍ���.
	void ReadKeyLog();

	// ���̃t���[���̃L�[���O��ǂݍ���.
	int ReadFrameKey();

	// �ǂݍ��ރt���[���̃J�E���g�𑝂₷.
	void AddFrameCnt();

private:
	std::vector<std::string> m_KeyLog;	// �L�[���O��ۑ�.
	int		m_OldFrameKeyStatus;		// �O��̉������L�[��ۑ�����.
	int		m_FrameKeyStatus;			// 1�t���[���̉������L�[��ۑ�����.
	int		m_RoadNumCnt;				// �L�[���O�̉��ڂ��擾���邩�̃J�E���g.
	int		m_Chain;					// �A�������.
	bool	m_KeyLogPlay;				// �L�[���O���Đ�����.
};