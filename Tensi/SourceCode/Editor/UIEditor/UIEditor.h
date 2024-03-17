#pragma once
#include "..\EditorBase.h"
#include "..\..\Scene\SceneManager\SceneManager.h"

class CSceneWidget;

/************************************************
*	UI�G�f�B�^.
*		���c�F�P.
**/
class CUIEditor final
	: public CEditorBase
{
public:
	CUIEditor();
	virtual ~CUIEditor();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// ImGui�`��.
	virtual void ImGuiRender() override;
	// ���f���̕`��.
	virtual void ModelRender() override {};
	// UI�̕`��.
	virtual void UIRender() override;
	// �G�t�F�N�g�`��.
	virtual void EffectRneder() override {};

private:
	// �\������V�[���̕ύX.
	void ChangeDispScene( const bool IsSaveStop = false );

	// ���O�t�@�C���̓ǂݍ���.
	HRESULT ZLogLoad();

	// ���O�t�@�C���̓ǂݍ���.
	HRESULT YLogLoad();

	// �ۑ�.
	void Save();
	// ���ɖ߂�.
	void Undo();
	// ��蒼��.
	void Redu();

private:
	std::unique_ptr<CSceneWidget>	m_pUI;				// UI.
	ESceneList						m_DispScene;		// �\�����Ă���V�[��.
	ESceneList						m_UIScene;			// UI�̕\������V�[��.

	const D3DXCOLOR4				m_OldBackColor;		// �̂̔w�i�摜.
	D3DXCOLOR4						m_BackColor;		// �w�i�J���[.
	D3DXCOLOR4						m_HitBoxColor;		// �����蔻��J���[.

	int								m_GridSpace;		// �O���b�h���̊Ԋu.
	CSprite*						m_pGrid;			// �O���b�g��.
	SSpriteRenderState				m_GridState;		// �O���b�h���.

	std::string						m_SelectUI;			// �I�����Ă���UI.
	std::vector<std::string>		m_UndoLogList;		// ���ɖ߂����O���X�g.
	std::vector<std::string>		m_ReduLogList;		// ��蒼�����O���X�g.
	int								m_LogMax;			// ���O�̏��.

	bool							m_IsAutoSave;		// �����ŕۑ����邩.
	bool							m_IsCreaterLog;		// ���O���쐬���邩.
	bool							m_IsDrag;			// �}�E�X�ł��߂�.
	bool							m_IsDispHitBox;		// �����蔻���`�悷�邩.
	bool							m_IsWidgetUpdate;	// UI�̍X�V���s����.
	bool							m_IsFileDelete;		// �t�@�C�����폜���邩.
};