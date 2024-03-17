#pragma once
#include "..\..\..\Global.h"

#pragma comment(lib, "winmm.lib") //���y�Đ��ŕK�v.

/************************************************
*	�T�E���h�N���X.
*	midi,mp3,wav�`���t�@�C���̍Đ��E��~���s��.
*		���c�F�P.
**/
class CSound final
{
public:
	static const int STR_BUFF_MAX = 256;//������o�b�t�@.

public:
	CSound();
	~CSound();

	// �����t�@�C�����J��.
	bool Open( LPCTSTR sFileName, LPCTSTR sAlias, HWND hWnd, int Volume = 1000 );

	// �����t�@�C�������.
	bool Close();

	// �������Đ�����.
	bool Play( bool bNotify = false );
	// �������t�Đ�����.
	bool ReversePlay( bool bNotify = false );
	// �������Đ�����(SE�Ŏg��).
	bool PlaySE( bool StopPlay = false, bool IsReverse = false );
	// �������Đ�����(���[�v).
	bool PlayLoop( bool IsReverse = false );

	// �������~����.
	bool Stop();
	// �������ꎞ��~����.
	bool Pause();

	// ��Ԃ��擾����.
	bool GetStatus( LPTSTR sStaus);

	// �����̒�~���m�F����.
	bool IsStopped();

	// �Đ��ʒu���ŏ��ɖ߂�.
	bool SeekToStart();

	// �����ݒ�.
	void SetInitParam( LPCTSTR sAlias, HWND hWnd,int Volume);

	// ���E�̃I�[�f�B�I��ON/OFF.
	bool SetIsAudioAllON( bool Flag );
	// ���I�[�f�B�I��ON/OFF.
	bool SetIsAudioLeftON( bool Flag );
	// �E�I�[�f�B�I��ON/OFF.
	bool SetIsAudioRightON( bool Flag );

	// ���ʂ�ݒ肷��.
	//	0 ~ 1000(�ʏ�).
	void	ResetVolume();
	bool	SetVolume( int Volume );
	bool	AddVolume( int Volume );

	// �Đ����x��ݒ肷��.
	//	0 ~ 1000(�ʏ�) ~ 2000.
	void	ResetPlaySpeed();
	bool	SetPlaySpeed( int Speed );
	bool	AddPlaySpeed( int Speed );

	// ���ʂ��擾.
	int		GetVolume	() { return m_Volume;		}
	int		GetMaxVolume() { return m_MaxVolume;	}

	// �Đ����x�̎擾.
	int		GetPlaySpeed() { return m_PlaySpeed;	}

private:
	HWND	m_hWnd;					//�E�B���h�E�n���h��.
	TCHAR	m_sAlias[STR_BUFF_MAX];	//�G�C���A�X.
	int		m_Volume;				//����(0~1000).
	int		m_MaxVolume;			//�ő剹��Open���ɐݒ�.
	int		m_PlaySpeed;			//�Đ����x.
};