#pragma once
#include "SpriteStruct.h"
#include "..\..\Utility\FileManager\FileManager.h"

/************************************************
*	�X�v���C�g�N���X.
**/
class CSprite final
{
public:
	using Sampler = unsigned long long;

public:
	CSprite();
	~CSprite();

	// ������.
	HRESULT Init( const std::string& FilePath );
	// �w�肵���V�[���ŃX�v���C�g�̏�������������.
	void InitSceneSpriteState( const std::string& SceneName, const int& No );

	// UI�Ń����_�����O.
	void RenderUI( SSpriteRenderState* pRenderState = nullptr );
	// 3D�Ń����_�����O.
	void Render3D( SSpriteRenderState* pRenderState = nullptr, const bool IsBillBoard = false );

	// UI�G�f�B�^��ImGui�̕`��.
	void UIEdtorImGuiRender( const bool IsAutoSave, SSpriteRenderState* pRenderState = nullptr );
	// �}�E�X�̂��ݔ���̕`��.
	void HitBoxRender( const D3DXVECTOR4& Color = Color4::Red, SSpriteRenderState* pRenderState = nullptr );

	// �}�E�X�̃h���b�O�̍X�V.
	bool DragUpdate( const bool IsDrag, const bool IsAutoSave, SSpriteRenderState* pRenderState = nullptr );
	// �}�E�X�ɏd�Ȃ��Ă��邩�擾����.
	bool GetIsOverlappingTheMouse( SSpriteRenderState* pRenderState );

	// �X�v���C�g����ۑ�����.
	HRESULT SpriteStateDataSave( SSpriteRenderState* pRenderState = nullptr );
	// �X�v���C�g���̎擾.
	HRESULT SpriteStateDataLoad();
	// �X�v���C�g���̕ۑ����čēǂݍ���.
	HRESULT SpriteStateSaveReload();

	// �\�����Ă���X�v���C�g�������̃V�[�����폜.
	HRESULT DispSpriteDeleteScene( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState = nullptr );
	// �\�����Ă���X�v���C�g�̐������炷.
	HRESULT DispSpriteDeleteNum( const bool IsAutoSave, const std::string& DispScene, const int No, SSpriteRenderState* pRenderState = nullptr );

	// �\������X�v���C�g�̒ǉ�.
	HRESULT AddDispSprite( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState = nullptr );

	// �\������V�[���̎擾.
	std::vector<std::string> GetDispSceneList() const { return m_SpriteState.DispSceneList; }
	// �\��������̎擾.
	int GetSceneDispNum( const std::string& SceneName ) { return m_SpriteState.SceneDispNum[SceneName]; }

	// �e�N�X�`���̎擾.
	inline ID3D11ShaderResourceView* GetTexture() const { return m_pTexture; }

	// �`����W�̎擾.
	inline D3DXVECTOR3 GetRenderPos() const { return m_SpriteRenderState.Transform.Position; }
	// �X�v���C�g���̎擾.
	inline SSpriteState GetSpriteState() const { return m_SpriteState; }
	// �摜�̃A�j���[�V���������擾.
	inline SSpriteAnimState GetAnimState() const { return m_SpriteRenderState.AnimState; }
	// �摜�̕`������擾.
	inline SSpriteRenderState GetRenderState() const { return m_SpriteRenderState; }
	// �X�v���C�g�̍���̍��W���擾����.
	D3DXVECTOR3 GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState );

	// ���O�t�@�C�����쐬���邩��ݒ�.
	void SetIsCreaterLog( bool* pFlag ) { m_pIsCreaterLog = pFlag; }
	// ���O���X�g�̐ݒ�.
	void SetLogList( std::vector<std::string>* pLogList ) { m_pLogList = pLogList; }

	// �`�悷��͈͂̐ݒ�.
	inline void SetRenderArea( const D3DXVECTOR4& Area ) { m_SpriteRenderState.RenderArea = Area; }
	inline void SetRenderArea( const float x, const float y, const float w, const float h ) { m_SpriteRenderState.RenderArea = { x, y, w, h }; }

	// �e�N�X�`���̐ݒ�.
	inline void SetTexture( ID3D11ShaderResourceView* pTexture ) { m_pTexture = pTexture; }

	// �A�j���[�V�������s�����̐ݒ�.
	inline void SetIsAnimPlay( const bool Flag ) { m_IsAnimPlay = Flag; }
	// ���̉摜��S�ĕ\�����邩�̐ݒ�.
	inline void SetIsAllDisp( const bool Flag ) { m_IsAllDisp = Flag; }

private:
	// �V�F�[�_�쐬.
	HRESULT CreateShader();
	// ���f���쐬(UI).
	HRESULT CreateModelUI();
	// ���f���쐬(3D).
	HRESULT CreateModel3D();
	// �e�N�X�`���쐬.
	HRESULT CreateTexture();
	// �T���v���쐬.
	HRESULT CreateSampler();
	// ���_���̍쐬.
	HRESULT CreateVertex( const float w, const float h, const float u, const float v );

	// �X�v���C�g���̍쐬.
	HRESULT CreateSpriteState();

	// �e�N�X�`���̔䗦���擾.
	int myGcd( int t, int t2 ) { if ( t2 == 0 ) return t; return myGcd( t2, t % t2 ); }

	// ���S���W�ɕϊ����邽�߂̒ǉ����W�̎擾.
	D3DXPOSITION3 GetAddCenterPosition();

	// UI�A�j���[�V�����̍X�V.
	void UIAnimUpdate( SSpriteRenderState* pRenderState );

	// UI�A�j���[�V�����̏�����.
	void UIAnimInit( SSpriteRenderState* pRenderState );
	// UI�A�j���[�V�������s�������ׂ�.
	bool UIAnimPlayCheck( SSpriteRenderState* pRenderState );
	// �g�kUI�A�j���[�V����.
	void ScaleUIAnimUpdate( SSpriteRenderState* pRenderState );
	// ����/�s����UI�A�j���[�V����.
	void AlphaUIAnimUpdate( SSpriteRenderState* pRenderState );
	// ��]UI�A�j���[�V����.
	void RotateUIAnimUpdate( SSpriteRenderState* pRenderState );
	// �ړ�UI�A�j���[�V����.
	void MoveUIAnimUpdate( SSpriteRenderState* pRenderState );
	// �U��qUI�A�j���[�V����.
	void PendulumUIAnimUpdate( SSpriteRenderState* pRenderState );
	// �A�j���[�V����UI�A�j���[�V����.
	void AnimationUIAnimUpdate( SSpriteRenderState* pRenderState );
	// UV�X�N���[��UI�A�j���[�V����.
	void ScrollUIAnimUpdate( SSpriteRenderState* pRenderState );
	// ���b�Z�[�WUI�A�j���[�V����.
	void MessageUIAnimUpdate( SSpriteRenderState* pRenderState );

	// UI�A�j���[�V�����̕������ϊ�����.
	std::string ConvertUIAnimationString( const std::string& string );

	// ���s�����̉��f�[�^���쐬����.
	void CreateWhenTempData( SSpriteRenderState* pRenderState );
	// ���s�����̏ڍאݒ�.
	void WhenAdvancedSetting( SSpriteRenderState* pRenderState );
	// ����̃L�[��������Ă��鎞�̏ڍאݒ�.
	void KeyAdvancedSetting( SSpriteRenderState* pRenderState );
	// �N���b�N���ꂽ���̏ڍאݒ�.
	void ClickAdvancedSetting( SSpriteRenderState* pRenderState );
	// ���b�Z�[�W���󂯎�������̏ڍאݒ�.
	void GetMessageAdvancedSetting( SSpriteRenderState* pRenderState );
	// �����_���̏ڍאݒ�.
	void RandomAdvancedSetting( SSpriteRenderState* pRenderState );

	// �����s�����̉��f�[�^���쐬����.
	void CreateWhatTempData( SSpriteRenderState* pRenderState );
	// �����s�����̏ڍאݒ�.
	void WhatAdvancedSetting( SSpriteRenderState* pRenderState );
	// ���[�v�Đ��̏ڍאݒ�.
	void LoopAdvancedSetting( SSpriteRenderState* pRenderState );
	// �g�k�̏ڍאݒ�.
	void ScaleAdvancedSetting( SSpriteRenderState* pRenderState );
	// ����/�s�����̓_�ł̏ڍאݒ�.
	void AlphaAdvancedSetting( SSpriteRenderState* pRenderState );
	// ��]�̏ڍאݒ�.
	void RotateAdvancedSetting( SSpriteRenderState* pRenderState );
	// �ړ��̏ڍאݒ�.
	void MoveAdvancedSetting( SSpriteRenderState* pRenderState );
	// �U��q�̏ڍאݒ�.
	void PendulumAdvancedSetting( SSpriteRenderState* pRenderState );
	// �A�j���[�V�����̏ڍאݒ�.
	void AnimationAdvancedSetting( SSpriteRenderState* pRenderState );
	// UV�X�N���[���̏ڍאݒ�.
	void ScrollAdvancedSetting( SSpriteRenderState* pRenderState );
	// ���b�Z�[�W�̏ڍאݒ�.
	void MessageAdvancedSetting( SSpriteRenderState* pRenderState );

	// �V�����ǉ�����X�v���C�g�̉��f�[�^���쐬����.
	void NewDispSpriteTmpData( SSpriteRenderState* pRenderState, const std::string& SceneName, const int No );

private:
	ID3D11Device*					m_pDevice;				// �f�o�C�X�I�u�W�F�N�g.
	ID3D11DeviceContext*			m_pContext;				// �f�o�C�X�R���e�L�X�g.

	ID3D11VertexShader*				m_pVertexShader;		// ���_�V�F�[�_.
	ID3D11InputLayout*				m_pVertexLayout;		// ���_���C�A�E�g.
	std::vector<ID3D11PixelShader*>	m_pPixelShaders;		// �s�N�Z���V�F�[�_���X�g.

	ID3D11Buffer*					m_pVertexBufferUI;		// ���_�o�b�t�@(UI�p).
	ID3D11Buffer*					m_pVertexBuffer3D;		// ���_�o�b�t�@(3D�p).
	ID3D11Buffer*					m_pConstantBuffer;		// �R���X�^���g�o�b�t�@.

	ID3D11ShaderResourceView*		m_pTexture;				// �e�N�X�`��.
	ID3D11SamplerState*				m_pSampleLinears[static_cast<Sampler>( ESamplerState::Max )]; // �T���v��:�e�N�X�`���Ɋe��t�B���^��������.

	SVertex							m_Vertices[4];			// ���_�쐬�p.
	SSpriteState					m_SpriteState;			// �X�v���C�g���.
	SSpriteRenderState				m_SpriteRenderState;	// �X�v���C�g�A�j���[�V�������.

	json							m_SpriteStateData;		// �X�v���C�g���f�[�^.
	std::vector<std::string>*		m_pLogList;				// ���O���X�g.

	bool							m_IsAnimPlay;			// �A�j���[�V�������s����.
	bool							m_IsAllDisp;			// ���̉摜��S�ĕ\�����邩.
	bool*							m_pIsCreaterLog;		// ���O�t�@�C�����쐬���邩.
};