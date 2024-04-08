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
*	�T�E���h�}�l�[�W���[�N���X.
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

	// �I�[�f�B�I�C���^�[�t�F�[�X�̍\�z.
	static HRESULT Create();
	// �T�E���h�̓ǂݍ���.
	static HRESULT SoundLoad( HWND hWnd );

	// ���X�i�[��ݒ�.
	static void	SetListener( CObject* pObj ) { GetInstance()->m_pListener = pObj; }
	// �I�΂��ԍ���ݒ�.
	static void SetSelectNo( const int No ) { GetInstance()->m_SelectNo = No; }
	// �I�΂��ԍ���������.
	static void ResetSelectNo() { GetInstance()->m_SelectNo = -1; }

	// �s�[�N�n�_�̎擾.
	static float GetPeakValue();

	// �Đ�.
	static void	PlaySE(				std::string Name,							float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // SE���Đ�����.
	static void	PlaySELoop(			std::string Name,																				 bool IsMsgStop = false ); // SE���Đ�����.
	static void	PlaySE3D(			std::string Name, D3DXPOSITION3 PlayPos,	float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // 3D��SE���Đ�����.
	static void	PlaySELoop3D(		std::string Name, D3DXPOSITION3 PlayPos,														 bool IsMsgStop = false ); // 3D��SE���Đ�����.
	static void	PlayBGM(			std::string Name,							bool	OldBGMStop	= true,							 bool IsMsgStop = false ); // BGM���Đ�����.
	static void	PlayFadeBGM(		std::string Name,							int		FadeSpeed	= 1,					 		 bool IsMsgStop = false ); // BGM���t�F�[�h�Ő؂�ւ��čĐ�����.
	static void	PlayBGM3D(			std::string Name, D3DXPOSITION3 PlayPos,	bool	OldBGMStop	= true, 						 bool IsMsgStop = false ); // 3D��BGM���Đ�����.

	// �t�Đ�.
	static void	ReversePlaySE(		std::string Name,							float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // SE���Đ�����.
	static void	ReversePlaySE3D(	std::string Name, D3DXPOSITION3 PlayPos,	float	Interval	= 0.0f, bool IsStopPlay = false, bool IsMsgStop = false ); // 3D��SE���Đ�����.
	static void	ReversePlayBGM(		std::string Name,							bool	OldBGMStop	= true,							 bool IsMsgStop = false ); // BGM���Đ�����.
	static void	ReversePlayFadeBGM(	std::string Name,							int		FadeSpeed	= 1,					 		 bool IsMsgStop = false ); // BGM���t�F�[�h�Ő؂�ւ��čĐ�����.
	static void	ReversePlayBGM3D(	std::string Name, D3DXPOSITION3 PlayPos,	bool	OldBGMStop	= true, 						 bool IsMsgStop = false ); // 3D��BGM���Đ�����.

	// ��~.
	static void	StopSE(		std::string Name,	bool IsMsgStop = false ); // �w���SE���~������.
	static void	StopBGM(	std::string Name,	bool IsMsgStop = false ); // �w���BGM���~������.
	static void	BGMAllStop(						bool IsMsgStop = false ); // BGM��S�Ē�~������.
	static void	SetIssLoopStop( bool Flag,		bool IsMsgStop = false ); // ���[�v�Đ����~�����邩.

	// �ꎞ��~.
	static void	PauseSE(	std::string Name,	bool IsMsgStop = false ); // �w���SE���ꎞ��~������.
	static void	PauseBGM(	std::string Name,	bool IsMsgStop = false ); // �w���BGM���ꎞ��~������.
	static void	BGMAllPause(					bool IsMsgStop = false ); // BGM��S�Ĉꎞ��~������.

	// ���E�̃I�[�f�B�I��ON/OFF.
	static void SetIsBGMAllON( std::string Name, bool Flag );
	static void SetIsSEAllON(  std::string Name, bool Flag );
	// ���I�[�f�B�I��ON/OFF.
	static void SetIsBGMLeftON( std::string Name, bool Flag );
	static void SetIsSELeftON(  std::string Name, bool Flag );
	// �E�I�[�f�B�I��ON/OFF.
	static void SetIsBGMRightON( std::string Name, bool Flag );
	static void SetIsSERightON(  std::string Name, bool Flag );

	// ���ʂ̐ݒ�.
	//	0 ~ 1000(�ʏ�).
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


	// �Đ����x�̐ݒ�.
	//	0 ~ 1000(�ʏ�) ~ 2000.
	static void ResetSEPlaySpeed(	std::string Name );
	static void SetSEPlaySpeed(		std::string Name, int Speed );
	static void AddSEPlaySpeed(		std::string Name, int Speed );
	static void ResetBGMPlaySpeed(	std::string Name );
	static void SetBGMPlaySpeed(	std::string Name, int Speed );
	static void AddBGMPlaySpeed(	std::string Name, int Speed );

private:
	// �C���X�^���X�̎擾.
	static SoundManager* GetInstance();

	// �T�E���h�t�@�C�����J��.
	static void OpenSound(
		std::vector<std::shared_ptr<CSound>>* List,	const HWND&			hWnd,
		const int			PlayMax,				const int			MaxVolume,
		const std::string&	FileName,				const std::wstring& wFilePath );

private:
	IMMDeviceEnumerator*	m_pEnumerator;	// �s�[�N�n�_�̎擾�Ŏg�p.
	IMMDevice*				m_pDevice;		// �s�[�N�n�_�̎擾�Ŏg�p.
	IAudioMeterInformation* m_pMeterInfo;	// �s�[�N�n�_�̎擾�Ŏg�p.
	CObject*				m_pListener;	// ���X�i�[(3D�Đ��Ŏg�p����).
	Sound_map				m_BGMList;		// �T�E���h(BGM)���X�g.
	Sound_map				m_SEList;		// �T�E���h(SE)���X�g.
	Sound_Time_map			m_PlayTimeList;	// �O��Đ����Ă���ǂ񂾂��o������.
	Sound_List				m_BGMNames;		// BGM���O���X�g.
	Sound_List				m_SENames;		// SE���O���X�g.
	std::string				m_NowBGMName;	// ���݂�BGM��.
	std::mutex				m_Mutex;
	bool					m_IsLoadEnd;	// �ǂݍ��݂��I�������.
	bool					m_LoopStopFlag;	// ���[�v�̒�~������.
	int						m_SelectNo;		// �I��������ԍ�.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	SoundManager( const SoundManager& )				= delete;
	SoundManager& operator = ( const SoundManager& )	= delete;
	SoundManager( SoundManager&& )					= delete;
	SoundManager& operator = ( SoundManager&& )		= delete;
};

