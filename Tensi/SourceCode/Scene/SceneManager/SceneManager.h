#pragma once
#include "..\..\Global.h"
#include "..\FadeManager\FadeManager.h"

class CSceneBase;

// �V�[�����X�g.
enum class enSceneList : unsigned char
{
	None,

	GameMain,

	UIEdit,
	Max
} typedef ESceneList;

/************************************************
*	�V�[���}�l�[�W���[.
**/
class SceneManager final
{
public:
	SceneManager();
	~SceneManager();

	// �X�V.
	static void	Update( const float& DeltaTime );
	// �`��.
	static void	Render();
	// �T�u�E�B���h�E�̕`��.
	static void	SubRender();

	// �V�[���̕ύX.
	static void SceneChange( const ESceneList NewScene, const EFadeType FadeType = EFadeType::None, const float FadeSpeed = 0.05f );

	// �V�[�����~�����邩�ݒ�.
	static void SetIsSceneStop( const bool Flag ) { GetInstance()->m_IsSceneStop = Flag; }
	// �|�[�Y���Ă��邩�ݒ�.
	static void SetIsPause( const bool Flag ) { GetInstance()->m_IsPause = Flag; }

	// �V�[�����~�����Ă��邩�擾.
	static bool GetIsSceneStop() { return GetInstance()->m_IsSceneStop; }
	// �|�[�Y���Ă��邩�擾.
	static bool GetIsPause() { return GetInstance()->m_IsPause; }

private:
	// �C���X�^���X�̎擾.
	static SceneManager* GetInstance();

	// �V�[���̕ύX�̍X�V.
	static void SceneChangeUpdate();

private:
	HWND						m_hWnd;				// �E�B���h�E�n���h��.
	std::unique_ptr<CSceneBase> m_pScene;			// �V�[��.
	ESceneList					m_NowScene;			// ���݂̃V�[��.
	ESceneList					m_ChangeScene;		// �ύX����V�[��.
	EFadeType					m_FadeType;			// �t�F�[�h�^�C�v.
	float						m_FadeSpeed;		// �t�F�[�h���x.
	bool						m_IsSceneChange;	// �V�[���ύX���Ă��邩.
	bool						m_IsFadeOut;		// �t�F�[�h�A�E�g��.
	bool						m_UpdateStopFlag;	// �X�V���~������.
	bool						m_IsSceneStop;		// �V�[�����~�����Ă��邩.
	bool						m_IsPause;			// �|�[�Y����.
	CBool						m_IsUIRender;		// UI��\�����邩.
	float						m_AutoSaveCnt;		// �I�[�g�Z�[�u�̃J�E���g.
};
