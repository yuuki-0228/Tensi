#pragma once
#include "..\Widget.h"
#include "..\..\..\..\Scene\SceneManager\SceneManager.h"

/************************************************
*	�V�[����UI�x�[�X�N���X.
*		���c�F�P.
**/
class CSceneWidget
	: public CWidget
{
public:
	using Render_List	= std::vector<std::pair<std::string, int>>;

public:
	CSceneWidget();
	virtual ~CSceneWidget();

	// UI�G�f�B�^�̍X�V.
	void UIEdtorUpdate( const bool IsDrag, const bool IsAutoSave );

	// �`��.
	virtual void Render() final;
	// UI�G�f�B�^��ImGui�̕`��.
	void UIEdtorImGuiRender( const bool& IsAutoSave );

	// UI�X�v���C�g�̒ǉ�.
	void AddUISpreite( const std::string& Name, CSprite* pSprite );

	// �\�����Ă���X�v���C�g�S�ẴX�v���C�g����ۑ�����.
	HRESULT AllSpriteStateDataSave();
	// �\�����Ă���X�v���C�g�S�ẴX�v���C�g����ǂݍ���.
	HRESULT AllSpriteStateDataLoad();

	// �����蔻���`�悷�邩.
	void SetIsDispHitBox( const bool& Flag, const D3DXVECTOR4& Color ) {
		m_IsDispHitBox	= Flag;
		m_HitBoxColor	= Color;
	}
	// ���O�t�@�C�����쐬���邩.
	void SetIsCreaterLog( bool* pFlag ) {
		m_pIsCreaterLog = pFlag;
		for ( auto& [Key, List] : m_SpriteList ) {
			for( auto& s : List ) s->SetIsCreaterLog( pFlag );
		}
	}
	// ���O���X�g�̐ݒ�.
	void SetLogList( std::vector<std::string>* pLogList ) {
		m_pLogList = pLogList;
		for ( auto& [Key, List] : m_SpriteList ) {
			for ( auto& s : List ) s->SetLogList( pLogList );
		}
	}

protected:
	// UI�G�f�B�^�̑O�ɒǉ��ōs���`��.
	virtual void FastAddRender() {};
	// UI�G�f�B�^�̌�ɒǉ��ōs���`��.
	virtual void LateAddRender() {};

	// �V�[�����̐ݒ�.
	//	Init�֐��̏��߂ɌĂ�.
	void SetSceneName( const ESceneList& NowScene );

	// �X�v���C�g���X�g���擾����.
	//	Init�֐��̏��߂ɌĂ�.
	void GetSpriteList();

private:
	// �`�揇�Ԃ̃\�[�g.
	void RenderSort();

protected:
	Sprite_map					m_SpriteList;		// �X�v���C�g���X�g.
	State_map					m_SpriteState;		// �X�v���C�g���.

private:
	std::string					m_SceneName;		// �V�[����.
	Render_List					m_RenderList;		// �`�揇�ԃ��X�g.
	std::vector<std::string>	m_DeleteList;		// �폜�����X�v���C�g���X�g.
	std::vector<std::string>*	m_pLogList;			// ���O���X�g.
	D3DXVECTOR4					m_HitBoxColor;		// �����蔻��̐F.
	bool						m_IsDispHitBox;		// �����蔻���`�悷�邩.
	bool*						m_pIsCreaterLog;	// ���O�t�@�C�����쐬���邩.
};
