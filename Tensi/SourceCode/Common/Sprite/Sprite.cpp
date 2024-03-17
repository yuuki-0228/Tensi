#include "Sprite.h"
#include "..\DirectX\DirectX11.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\Utility\Input\Input.h"
#include "..\..\Utility\Input\InputList.h"
#include "..\..\Utility\Random\Random.h"
#include "..\..\Utility\Easing\Easing.h"
#include "..\..\Utility\FileManager\ImagSize\ImageSize.h"
#include "..\..\Utility\ImGuiManager\ImGuiManager.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"
#include "..\..\Utility\Message\Message.h"
#include "..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"

namespace {
	// �V�F�[�_�t�@�C����(�f�B���N�g�����܂�).
	constexpr TCHAR SHADER_NAME[]			= _T( "Data\\Shader\\Sprite.hlsl"	);

	// �A���h�D�p�̃��O�t�@�C���̈ړ��ꏊ�̃p�X.
	constexpr char	UNDO_LOG_FILE_PATH[]	= "Data\\UIEditor\\UndoLog\\";
	
	// �s�N�Z���V�F�[�_�[�̃G���g���|�C���g��.
	const std::vector<const char*> PS_SHADER_ENTRY_NAMES = {
		"PS_Main",
		"PS_Mask",
		"PS_Transition"
	};
}

CSprite::CSprite()
	: m_pDevice					( nullptr )
	, m_pContext				( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShaders			()
	, m_pConstantBuffer			( nullptr )
	, m_pVertexBufferUI			( nullptr )
	, m_pVertexBuffer3D			( nullptr )
	, m_pSampleLinears			()
	, m_pTexture				( nullptr )
	, m_pMaskTexture			( nullptr )
	, m_pRuleTexture			( nullptr )
	, m_Vertices				()
	, m_SpriteState				()
	, m_SpriteRenderState		()
	, m_SpriteStateData			()
	, m_pLogList				( nullptr )
	, m_IsAnimPlay				( true )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
	, m_IsAllDisp				( true )
	, m_pIsCreaterLog			( nullptr )
{
}

CSprite::~CSprite()
{
	for ( auto& s : m_pSampleLinears )	SAFE_RELEASE( s );
	SAFE_RELEASE( m_pMaskTexture	);
	SAFE_RELEASE( m_pTexture		);
	SAFE_RELEASE( m_pVertexBuffer3D	);
	SAFE_RELEASE( m_pVertexBufferUI	);
	SAFE_RELEASE( m_pConstantBuffer );
	for ( auto itr = m_pPixelShaders.rbegin(), e = m_pPixelShaders.rend(); itr != e; ++itr ) {
		SAFE_RELEASE( *itr );
	}
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//----------------------------.
// ������.
//----------------------------.
HRESULT CSprite::Init( const std::string& FilePath )
{
	m_pContext	= DirectX11::GetContext();
	m_pDevice	= DirectX11::GetDevice();
	m_SpriteState.FilePath = FilePath;
	m_pPixelShaders.resize( PS_SHADER_ENTRY_NAMES.size(), nullptr );

	// �X�v���C�g���̎擾.
	if( FAILED( SpriteStateDataLoad() ) ) return E_FAIL;

	// ������.
	if ( FAILED( CreateShader()	 ) ) return E_FAIL;
	if ( FAILED( CreateModelUI() ) ) return E_FAIL;
	if ( FAILED( CreateModel3D() ) ) return E_FAIL;
	if ( FAILED( CreateTexture() ) ) return E_FAIL;
	if ( FAILED( CreateSampler() ) ) return E_FAIL;

	Log::PushLog( m_SpriteState.FilePath + " �ǂݍ��� : ����" );
	return S_OK;
}

//----------------------------.
// �w�肵���V�[���ŃX�v���C�g�̏�������������.
//----------------------------.
void CSprite::InitSceneSpriteState( const std::string& SceneName, const int& No )
{
	D3DXPOSITION3*	Pos			= &m_SpriteRenderState.Transform.Position;
	D3DXROTATION3*	Rot			= &m_SpriteRenderState.Transform.Rotation;
	D3DXSCALE3*		Scale		= &m_SpriteRenderState.Transform.Scale;
	D3DXCOLOR4*		Color		= &m_SpriteRenderState.Color;
	D3DXPOSITION3*	StartPos	= &m_SpriteRenderState.UIAnimState.StartTransform.Position;
	D3DXROTATION3*	StartRot	= &m_SpriteRenderState.UIAnimState.StartTransform.Rotation;
	D3DXSCALE3*		StartScale	= &m_SpriteRenderState.UIAnimState.StartTransform.Scale;
	D3DXCOLOR4*		StartColor	= &m_SpriteRenderState.UIAnimState.StartColor;

	// �w�肵������ۑ�.
	m_SpriteRenderState.SceneName	= SceneName;
	m_SpriteRenderState.No			= std::to_string( No );

	// �g�����X�t�H�[���̐ݒ�.
	json TransformData = m_SpriteStateData[m_SpriteRenderState.SceneName][m_SpriteRenderState.No]["Transform"];
	Pos->x			= TransformData["Position"]["x"];
	Pos->y			= TransformData["Position"]["y"];
	Pos->z			= TransformData["Position"]["z"];
	Rot->x			= TransformData["Rotation"]["x"];
	Rot->y			= TransformData["Rotation"]["y"];
	Rot->z			= TransformData["Rotation"]["z"];
	Scale->x		= TransformData["Scale"]["x"];
	Scale->y		= TransformData["Scale"]["y"];
	Scale->z		= TransformData["Scale"]["z"];
	StartPos->x		= Pos->x;
	StartPos->y		= Pos->y;
	StartPos->z		= Pos->z;
	StartRot->x		= Rot->x;
	StartRot->y		= Rot->y;
	StartRot->z		= Rot->z;
	StartScale->x	= Scale->x;
	StartScale->y	= Scale->y;
	StartScale->z	= Scale->z;

	// �F�̐ݒ�.
	json ColorData = m_SpriteStateData[m_SpriteRenderState.SceneName][m_SpriteRenderState.No]["Color"];
	Color->x		= ColorData["R"];
	Color->y		= ColorData["G"];
	Color->z		= ColorData["B"];
	Color->w		= ColorData["A"];
	StartColor->x	= ColorData["R"];
	StartColor->y	= ColorData["G"];
	StartColor->z	= ColorData["B"];
	StartColor->w	= ColorData["A"];

	// UI�A�j���[�V�����̐ݒ�.
	json			AnimData	= m_SpriteStateData[m_SpriteRenderState.SceneName][m_SpriteRenderState.No]["UIAnimation"];
	std::string*	UIAnimWhen	= &m_SpriteRenderState.UIAnimState.UIAnimWhen;
	std::string*	UIAnimWhat	= &m_SpriteRenderState.UIAnimState.UIAnimWhat;
	*UIAnimWhen	= AnimData["When"];
	*UIAnimWhat	= AnimData["What"];

	// ���[�v�����邩�̐ݒ�.
	if ( *UIAnimWhen != "None" && *UIAnimWhat != "None" ) {
		m_SpriteRenderState.UIAnimState.IsLoop		= AnimData["IsLoop"];
		m_SpriteRenderState.UIAnimState.PlayMax		= AnimData["PlayMax"];
		m_SpriteRenderState.UIAnimState.EndMessage	= AnimData["EndMessage"];
	}
	// �g�kUI�A�j���[�V����.
	if ( *UIAnimWhat == "Scale" ) {
		// �g�k�̎n�߂�����̐ݒ�.
		m_SpriteRenderState.UIAnimState.Flag_x = AnimData["Scale"]["Start"]["x"] == "Shrink";
		m_SpriteRenderState.UIAnimState.Flag_y = AnimData["Scale"]["Start"]["y"] == "Shrink";
	}
	// ����/�s����UI�A�j���[�V����.
	else if ( *UIAnimWhat == "Alpha" ) {
		// ����/�s�����̎n�߂�����̐ݒ�.
		m_SpriteRenderState.UIAnimState.Flag_x = AnimData["Alpha"]["Start"] == "Opacity";

		// �J�n���̓����l��ۑ�.
		m_SpriteRenderState.UIAnimState.StartColor.w = Color->w;
	}
	// �U��qUI�A�j���[�V����.
	else if ( *UIAnimWhat == "Pendulum" ) {
		// ���߂ɗh�������̐ݒ�.
		m_SpriteRenderState.UIAnimState.Flag_x = AnimData["Pendulum"]["Start"] == "Left";
	}
	// �A�j���[�V����UI�A�j���[�V����.
	else if ( *UIAnimWhat == "Animation" ) {
		// �A�j���[�V�������x�̐ݒ�.
		m_SpriteRenderState.AnimState.FrameTime = AnimData["Anim"]["Time"];
	}
	// UV�X�N���[��UI�A�j���[�V����.
	else if ( *UIAnimWhat == "Scroll" ) {
		// UV�X�N���[�����x�̐ݒ�.
		m_SpriteRenderState.AnimState.ScrollSpeed.x = AnimData["Scroll"]["Speed"]["x"];
		m_SpriteRenderState.AnimState.ScrollSpeed.y = AnimData["Scroll"]["Speed"]["y"];
	}
}

//----------------------------.
// UI�Ń����_�����O.
//----------------------------.
void CSprite::RenderUI( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;

	// ��\���̏ꍇ�͏������s��Ȃ�.
	if ( m_IsAllDisp			== false	) return;
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	if ( m_IsAnimPlay ) {
		UIAnimUpdate( RenderState );	// UI�A�j���[�V�����̍X�V.
		AnimState->SetAnimNumber();		// �A�j���[�V�����ԍ���ݒ�.
		AnimState->SetPatternNo();		// �p�^�[���ԍ���ݒ�.
		AnimState->AnimUpdate();		// �A�j���[�V�����̍X�V.
		AnimState->UVScrollUpdate();	// �X�N���[���̍X�V.
	}

	// �␳�l�p���s�ړ��s��.
	D3DXMATRIX mOffSet;
	D3DXMatrixTranslation( &mOffSet, m_SpriteState.OffSet.x, m_SpriteState.OffSet.y, 0.0f );

	// ���[���h�s����擾.
	D3DXMATRIX mWorld = RenderState->Transform.GetWorldMatrix() * mOffSet;

	// �g�p����V�F�[�_�̓o�^.
	const int PSShaderNo = m_pRuleTexture == nullptr ? m_pMaskTexture == nullptr ? 0 : 1 : 2;
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShaders[PSShaderNo],  nullptr, 0 );

	// �V�F�[�_�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE		pData;
	SSpriteShaderConstantBuffer		cb;	// �R���X�^���g�o�b�t�@.
	// �o�b�t�@���̃f�[�^�̏��������J�n����map.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ���[���h�s���n��(�r���[,�v���W�F�N�V�����͎g��Ȃ�).
		D3DXMATRIX m = mWorld;
		D3DXMatrixTranspose( &m, &m );// �s���]�u����.
		cb.mWVP = m;

		// �J���[.
		cb.vColor = RenderState->Color;

		// �e�N�X�`�����W(UV���W).
		// 1�}�X������̊����Ƀp�^�[���ԍ�(�}�X��)�������č��W��ݒ肷��.
		cb.vUV.x = AnimState->UV.x + AnimState->Scroll.x / 2;
		cb.vUV.y = AnimState->UV.y + AnimState->Scroll.y / 2;

		// �`�悷��G���A.
		const float WndW = DirectX11::GetWndWidth();
		const float WndH = DirectX11::GetWndHeight();
		cb.vRenderArea	 = RenderState->RenderArea;
		if ( cb.vRenderArea.z <= -1.0f ) cb.vRenderArea.z = WndW;
		if ( cb.vRenderArea.w <= -1.0f ) cb.vRenderArea.w = WndH;

		// �r���[�|�[�g�̕��A������n��.
		cb.fViewPortWidth	= WndW;
		cb.fViewPortHeight	= WndH;

		// �f�B�U�������g�p���邩.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// �A���t�@�u���b�N���g�p���邩��n��.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�����H.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// ���_�o�b�t�@���Z�b�g.
	UINT stride = sizeof( SVertex );// �f�[�^�̊Ԋu.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBufferUI, &stride, &offset );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// �e�N�X�`�����V�F�[�_�ɓn��.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( RenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &m_pTexture );
	m_pContext->PSSetShaderResources( 1, 1, &m_pMaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &m_pRuleTexture );

	// �A���t�@�u�����h�L���ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// �v���~�e�B�u�������_�����O.
	m_pContext->Draw( 4, 0 );// �|��(���_4��).

	// �A���t�@�u�����h�����ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}

//----------------------------.
// 3D�Ń����_�����O.
//----------------------------.
void CSprite::Render3D( SSpriteRenderState* pRenderState, const bool IsBillBoard )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;
#ifdef _DEBUG
	PositionRenderer::Render( RenderState->Transform );
#endif
	// ��\���̏ꍇ�͏������s��Ȃ�.
	if ( m_IsAllDisp			== false	) return;
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	if ( m_IsAnimPlay ) {
		UIAnimUpdate( RenderState );	// UI�A�j���[�V�����̍X�V.
		AnimState->SetAnimNumber();		// �A�j���[�V�����ԍ���ݒ�.
		AnimState->SetPatternNo();		// �p�^�[���ԍ���ݒ�.
		AnimState->AnimUpdate();		// �A�j���[�V�����̍X�V.
		AnimState->UVScrollUpdate();	// �X�N���[���̍X�V.
	}

	// �␳�l�p���s�ړ��s��.
	D3DXMATRIX mOffSet;
	D3DXMatrixTranslation( &mOffSet, m_SpriteState.OffSet.x, m_SpriteState.OffSet.y, 0.0f );

	// ���[���h�s����擾.
	D3DXMATRIX mWorld = RenderState->Transform.GetWorldMatrix() * mOffSet;

	// �r���{�[�h�p.
	if ( IsBillBoard == true ) {
		D3DXMATRIX CancelRotation = CameraManager::GetViewMatrix();	// �r���[�s��.
		CancelRotation._41
			= CancelRotation._42 = CancelRotation._43 = 0.0f;			// xyz��0�ɂ���.
		// CancelRotation�̋t�s������߂܂�.
		D3DXMatrixInverse( &CancelRotation, nullptr, &CancelRotation );
		mWorld = CancelRotation * mWorld;
	}

	// �g�p����V�F�[�_�̓o�^.
	const int PSShaderNo = m_pRuleTexture == nullptr ? m_pMaskTexture == nullptr ? 0 : 1 : 2;
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShaders[PSShaderNo], nullptr, 0 );

	// �V�F�[�_�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE		pData;
	SSpriteShaderConstantBuffer		cb;	// �R���X�^���g�o�b�t�@.
	// �o�b�t�@���̃f�[�^�̏��������J�n����map.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ���[���h,�r���[,�v���W�F�N�V�����s���n��.
		D3DXMATRIX m = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose( &m, &m );// �s���]�u����.
		cb.mWVP = m;

		// �J���[.
		cb.vColor = RenderState->Color;

		// �e�N�X�`�����W(UV���W).
		cb.vUV.x = AnimState->UV.x + AnimState->Scroll.x / 2;
		cb.vUV.y = AnimState->UV.y + AnimState->Scroll.y / 2;

		// �`�悷��G���A.
		const float WndW = DirectX11::GetWndWidth();
		const float WndH = DirectX11::GetWndHeight();
		cb.vRenderArea = RenderState->RenderArea;
		if ( cb.vRenderArea.z <= -1.0f ) cb.vRenderArea.z = WndW;
		if ( cb.vRenderArea.w <= -1.0f ) cb.vRenderArea.w = WndH;

		// �r���[�|�[�g�̕��A�����͎g�p���Ȃ����ߏ�����.
		cb.fViewPortWidth	= 0.0f;
		cb.fViewPortHeight	= 0.0f;

		// �f�B�U�������g�p���邩.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// �A���t�@�u���b�N���g�p���邩��n��.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�����H.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// ���_�o�b�t�@���Z�b�g.
	UINT stride = sizeof( SVertex );// �f�[�^�̊Ԋu.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBuffer3D, &stride, &offset );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// �e�N�X�`�����V�F�[�_�ɓn��.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( RenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &m_pTexture );
	m_pContext->PSSetShaderResources( 1, 1, &m_pMaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &m_pRuleTexture );

	// �A���t�@�u�����h�L���ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// �v���~�e�B�u�������_�����O.
	m_pContext->Draw( 4, 0 );// �|��(���_4��).

	// �A���t�@�u�����h�����ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}

//----------------------------.
// UI�G�f�B�^��ImGui�̕`��.
//----------------------------.
void CSprite::UIEdtorImGuiRender( const bool IsAutoSave, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState		= &RenderState->UIAnimState;
	std::string*		UIAnimWhen		= &UIAnimState->UIAnimWhen;
	std::string*		UIAnimWhat		= &UIAnimState->UIAnimWhat;
	const json			OldStateData	= m_SpriteStateData;

	// �g�����X�t�H�[���̐ݒ�.
	STransform*			pTrance		= &UIAnimState->StartTransform;
	D3DXROTATION3		pDegRot		= Math::ToDegree( pTrance->Rotation );
	const STransform	OldTrance	= *pTrance;
	ImGuiManager::InputVec3( "Position",	&pTrance->Position );
	ImGuiManager::InputVec3( "Rotation",	&pDegRot );
	ImGuiManager::InputVec3( "Scale",		&pTrance->Scale, { 1.0f, 1.0f, 1.0f } );
	pTrance->Rotation = Math::ToRadian( pDegRot );
	if ( *pTrance != OldTrance ) {
		RenderState->Transform = *pTrance;

		// �ۑ����čēǂݍ���.
		if ( IsAutoSave ) {
			if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return;
		}
	}

	// �F�̐ݒ�.
	D3DXCOLOR4*			pColor		= &UIAnimState->StartColor;
	const D3DXCOLOR4	OldColor	= *pColor;
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "Color" ) ) {
		ImGui::ColorEdit4( "##Color", *pColor );
		ImGui::TreePop();
	}
	if ( *pColor != OldColor ) {
		RenderState->Color = *pColor;

		// �ۑ����čēǂݍ���.
		if ( IsAutoSave ) {
			if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return;
		}
	}

	// ���s�������X�g.
	const std::vector<std::string> WhenList = {
		u8"����",
		u8"�펞",
		u8"�}�E�X�J�[�\�����d�Ȃ�����",
		u8"�N���b�N���ꂽ��",
		u8"����̃L�[��������Ă��鎞",
		u8"���b�Z�[�W���󂯎������",
		u8"�����_��( �m�� )"
	};
	// �����s�������X�g.
	const std::vector<std::string> WhatList = {
		u8"����",
		u8"�g�k",
		u8"���� / �s�����̓_��",
		u8"��]",
		u8"�ړ�",
		u8"�U��q",
		u8"�A�j���[�V����",
		u8"UV�X�N���[��",
		u8"���b�Z�[�W"
	};

	// ��R�}�̑傫���̐ݒ�̕\��.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "Stride" ) ) {
		const D3DXVECTOR2 OldStride = { m_SpriteState.Stride.w, m_SpriteState.Stride.h };
		D3DXVECTOR2 NewStride = OldStride;
		ImGui::InputFloat2( "##In2_Stride", NewStride );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"�ۑ���A�J���Ȃ������ƂŔ��f����܂��B\n���ύX����ƁA�\���T�C�Y�͈�R�}�̃T�C�Y�ɕύX����܂��B" );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) NewStride = { m_SpriteState.Base.w, m_SpriteState.Base.h };
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"������Ԃɖ߂��܂�" );

		// ��R�}�̑傫�����X�V.
		if ( OldStride != NewStride ) {
			m_SpriteState.Stride				= { NewStride.x, NewStride.y };
			m_SpriteState.Disp					= { NewStride.x, NewStride.y };
			m_SpriteStateData["Stride"]["w"]	= NewStride.x;
			m_SpriteStateData["Stride"]["h"]	= NewStride.y;
			m_SpriteStateData["Disp"]["w"]		= NewStride.x;
			m_SpriteStateData["Disp"]["h"]		= NewStride.y;
		}
		ImGui::TreePop();
	}

	// ���[�J�����W�̐ݒ�̕\��.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "LocalPosition" ) ) {
		// ���[�J�����W�̃R���{�{�b�N�X�̍쐬.
		ELocalPosition SpriteLocalPos		= static_cast<ELocalPosition>( m_SpriteStateData["LocalPosition"] );
		ImGuiManager::Combo( "##NoName", &SpriteLocalPos, { ELocalPosition::Max } );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"�ۑ���A�J���Ȃ������ƂŔ��f����܂��B" );

		// ���[�J�����W�̍X�V.
		m_SpriteStateData["LocalPosition"]	= static_cast<int>( SpriteLocalPos );
		ImGui::TreePop();
	}
	ImGui::Separator();

	// UI�A�j���[�V�����̐ݒ�c���[���쐬.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "UIAnimation" ) ) {
		// ���݂̂��s��������{��ɕϊ����ăR���{�{�b�N�X�̍쐬.
		std::string	NowWhen = ConvertUIAnimationString( *UIAnimWhen );
		ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( u8"���s���H" ) ) {
			ImGuiManager::Combo( "##NoName", &NowWhen, WhenList );
			ImGui::TreePop();
		}
		// �O�̏�Ԃƍ��̏�Ԃ����ĕύX����Ă��邩.
		const std::string OldWhen = *UIAnimWhen;
		*UIAnimWhen = ConvertUIAnimationString( NowWhen );
		m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"]["When"] = *UIAnimWhen;
		// ���f�[�^���K�v�ȏꍇ�͍쐬����.
		if ( OldWhen != *UIAnimWhen ) CreateWhenTempData( RenderState );

		// ���s�����̏ڍאݒ�.
		WhenAdvancedSetting( RenderState );
		ImGui::Separator();
		
		// ���݂̉����s��������{��ɕϊ����ăR���{�{�b�N�X�̍쐬.
		std::string NowWhat = ConvertUIAnimationString( *UIAnimWhat );
		ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( u8"�����s���H" ) ) {
			ImGuiManager::Combo( "##NoName", &NowWhat, WhatList );
			ImGui::TreePop();
		}

		// �O�̏�Ԃƍ��̏�Ԃ����ĕύX����Ă��邩.
		const std::string OldWhat = *UIAnimWhat;
		*UIAnimWhat = ConvertUIAnimationString( NowWhat );
		m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"]["What"] = *UIAnimWhat;
		// ���f�[�^���K�v�ȏꍇ�͍쐬����.
		if ( OldWhat != *UIAnimWhat ) CreateWhatTempData( RenderState );

		// �����s�����̏ڍאݒ�.
		WhatAdvancedSetting( RenderState );
		ImGui::Separator();
		ImGui::TreePop();
	}
	if ( ImGuiManager::Button( "Save" ) ) {
		SpriteStateDataSave( RenderState );
		MessageWindow::PushMessage( u8"�ۑ����܂����B", Color4::Magenta );
	}
	ImGui::Separator();

	// �X�v���C�g�̏�񂪕ύX����Ă���ꍇ�A�����ŕۑ���������.
	if ( IsAutoSave		== false				) return;
	if ( OldStateData	!= m_SpriteStateData	) {
		// �ۑ����čēǂݍ���.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return;
	}
}

//----------------------------.
// �}�E�X�̂��ݔ���̕`��.
//----------------------------.
void CSprite::HitBoxRender( const D3DXVECTOR4& Color, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	if ( m_IsAllDisp			== false ) return;
	if ( RenderState->IsDisp	== false ) return;
	if ( RenderState->Color.w	== 0.0f	 ) return;

	// �����蔻��̕`��.
	SSpriteRenderState HitBoxState;
	HitBoxState.Transform.Position	= GetSpriteUpperLeftPos( RenderState );
	HitBoxState.Transform.Scale.x	= m_SpriteState.Disp.w * RenderState->Transform.Scale.x;
	HitBoxState.Transform.Scale.y	= m_SpriteState.Disp.h * RenderState->Transform.Scale.y;
	HitBoxState.Color				= Color;
	SpriteResource::GetSprite( "HitBox" )->RenderUI( &HitBoxState );
}

//----------------------------.
// �}�E�X�̃h���b�O�̍X�V.
//----------------------------.
bool CSprite::DragUpdate( const bool IsDrag, const bool IsAutoSave, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	if ( IsDrag					== false ) return false;
	if ( m_IsAllDisp			== false ) return false;
	if ( RenderState->IsDisp	== false ) return false;
	if ( RenderState->Color.w	== 0.0f	 ) return false;

	const D3DXPOSITION2& MousePos		= Input::GetMousePosition();
	const D3DXPOSITION2& MouseOldPos	= Input::GetMouseOldPosition();

	// ���܂�Ă��邩.
	if ( RenderState->IsGrab ) {
		// �}�E�X�̈ړ��������X�v���C�g�𓮂���.
		const D3DXVECTOR2& MoveValue = MousePos - MouseOldPos;
		RenderState->Transform.Position.x += MoveValue.x;
		RenderState->Transform.Position.y += MoveValue.y;

		// ���N���b�N��������Ă��邩.
		if ( !KeyInput::IsKeyPress( VK_LBUTTON ) ) {
			// ������.
			Input::SetIsMousrGrab( false );
			RenderState->IsGrab = false;

			// ���W�̍X�V.
			RenderState->UIAnimState.StartTransform.Position = RenderState->Transform.Position;

			// �ۑ����čēǂݍ���.
			if ( IsAutoSave ) {
				if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return false;
			}
		}
		return true;
	}
	// ���܂�Ă��Ȃ�.
	else {
		// �}�E�X���ʂ̂�����ł���.
		if ( Input::IsMousrGrab() ) return false;
		static bool f = false;
		// �}�E�X�J�[�\���ɏd�Ȃ��Ă��邩.
		if ( GetIsOverlappingTheMouse( pRenderState ) == false ) return false;

		// ���N���b�N�������ꂽ�u�Ԃ�.
		if ( !Input::GetIsLeftClickDown() ) return false;

		// ����.
		Input::SetIsMousrGrab( true );
		RenderState->IsGrab = true;
		return true;
	}
}

//----------------------------.
// �}�E�X�ɏd�Ȃ��Ă��邩�擾����.
//----------------------------.
bool CSprite::GetIsOverlappingTheMouse( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const D3DXPOSITION2&	MousePos		= Input::GetMousePosition();
	const D3DXPOSITION3&	UpperLeftPos	= GetSpriteUpperLeftPos( RenderState );
	const D3DXSCALE3&		Scale			= RenderState->Transform.Scale;

	return	( ( UpperLeftPos.x <= MousePos.x ) && ( MousePos.x <= UpperLeftPos.x + m_SpriteState.Disp.w * Scale.x ) ) &&
			( ( UpperLeftPos.y <= MousePos.y ) && ( MousePos.y <= UpperLeftPos.y + m_SpriteState.Disp.h * Scale.y ) );
}

//---------------------------.
// �X�v���C�g����ۑ�����.
//---------------------------.
HRESULT CSprite::SpriteStateDataSave( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	const std::string&	UIAnimWhen	= UIAnimState->UIAnimWhen;
	const std::string&	UIAnimWhat	= UIAnimState->UIAnimWhat;

	// �s�v�ȃf�[�^���폜����.
	json AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	if ( UIAnimWhen != "" && UIAnimWhen != "None" ) {
		if ( UIAnimWhen != "Key"		) AnimData.erase( "Key"			);
		if ( UIAnimWhen != "Click"		) AnimData.erase( "Click"		);
		if ( UIAnimWhen != "GetMessage"	) AnimData.erase( "GetMessage"	);
		if ( UIAnimWhen != "Random"		) AnimData.erase( "Random"		);
	}
	if ( UIAnimWhat != "" && UIAnimWhat != "None" ) {
		if ( UIAnimWhat != "Scale"		) AnimData.erase( "Scale"		);
		if ( UIAnimWhat != "Alpha"		) AnimData.erase( "Alpha"		);
		if ( UIAnimWhat != "Rotate"		) AnimData.erase( "Rotate"		);
		if ( UIAnimWhat != "Move"		) AnimData.erase( "Move"		);
		if ( UIAnimWhat != "Pendulum"	) AnimData.erase( "Pendulum"	);
		if ( UIAnimWhat != "Animation"	) AnimData.erase( "Animation"	);
		if ( UIAnimWhat != "Scroll"		) AnimData.erase( "Scroll"		);
		if ( UIAnimWhat != "Message"	) AnimData.erase( "Message"		);
	}
	if ( UIAnimWhen != "" && UIAnimWhen != "None" && UIAnimWhat != "" && UIAnimWhat != "None" ) {
		// ���[�v�Đ����邩�̏㏑��.
		AnimData["IsLoop"]		= UIAnimState->IsLoop;
		AnimData["PlayMax"]		= UIAnimState->PlayMax;
		AnimData["EndMessage"]	= UIAnimState->EndMessage;
	}
	else if ( AnimData.empty() == false ) {
		// �s�v�ȃf�[�^�̍폜.
		AnimData.erase( "IsLoop" );
		AnimData.erase( "PlayMax" );
		AnimData.erase( "EndMessage" );
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;

	// �g�����X�t�H�[���̏㏑��
	json TransformData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["Transform"];
	TransformData["Position"]["x"]	= UIAnimState->StartTransform.Position.x;
	TransformData["Position"]["y"]	= UIAnimState->StartTransform.Position.y;
	TransformData["Position"]["z"]	= UIAnimState->StartTransform.Position.z;
	TransformData["Rotation"]["x"]	= UIAnimState->StartTransform.Rotation.x;
	TransformData["Rotation"]["y"]	= UIAnimState->StartTransform.Rotation.y;
	TransformData["Rotation"]["z"]	= UIAnimState->StartTransform.Rotation.z;
	TransformData["Scale"]["x"]		= UIAnimState->StartTransform.Scale.x;
	TransformData["Scale"]["y"]		= UIAnimState->StartTransform.Scale.y;
	TransformData["Scale"]["z"]		= UIAnimState->StartTransform.Scale.z;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["Transform"] = TransformData;
	// �F�̏㏑��.
	json ColorData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["Color"];
	ColorData["R"] = UIAnimState->StartColor.x;
	ColorData["G"] = UIAnimState->StartColor.y;
	ColorData["B"] = UIAnimState->StartColor.z;
	ColorData["A"] = UIAnimState->StartColor.w;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["Color"] = ColorData;

	// ���߂Ȃ����邩�̏㏑��.
	m_SpriteStateData["IsLock"] = RenderState->IsLock;

	// �ۑ����čēǂݍ���.
	SpriteStateSaveReload();
	return S_OK;
}

//---------------------------.
// �X�v���C�g���̎擾.
//---------------------------.
HRESULT CSprite::SpriteStateDataLoad()
{
	// �␳�l�e�L�X�g�̓ǂݍ���.
	std::string TextPath	= m_SpriteState.FilePath;
	TextPath.erase( TextPath.rfind( "\\" ), TextPath.size() ) += "\\OffSet.json";
	json OffSetData			= FileManager::JsonLoad( TextPath );
	
	// �������O�̃e�L�X�g�̓ǂݍ���.
	TextPath = m_SpriteState.FilePath;
	TextPath.erase( TextPath.rfind( "." ), TextPath.size() ) += ".json";
#ifndef _DEBUG
	// �Í�������Ă��邽�ߌ��̃p�X�ɖ߂�
	auto dp = TextPath.rfind( "." );
	auto fp = TextPath.rfind( "\\" ) + 1;
	if ( dp != std::string::npos && fp != std::string::npos ) {
		std::string n = TextPath.substr( fp, dp - fp );
		n.erase( 0, 1 );
		n.erase( n.length() - 2 );
		TextPath.erase( fp, dp - fp );
		TextPath.insert( fp, n );
	}
#endif
	m_SpriteStateData = FileManager::JsonLoad( TextPath );

	// �t�@�C�����������߃t�@�C�����쐬����.
	if ( m_SpriteStateData.empty() ) {
#ifdef _DEBUG
		Log::PushLog( TextPath + " ���������ߍ쐬���܂��B" );
		if ( FAILED( CreateSpriteState() ) ) return E_FAIL;

		// �쐬�ł������߂�����x�ǂݍ��ݒ���.
		Log::PushLog( m_SpriteState.FilePath + " ��������x�ǂݍ��ݒ����܂��B" );
		SpriteStateDataLoad();
		return S_OK;
#else
		return E_FAIL;
#endif
	}

	// �X�v���C�g���̎擾.
	m_SpriteState.LocalPosNo	= static_cast<ELocalPosition>( m_SpriteStateData["LocalPosition"] );
	m_SpriteState.Disp.w		= m_SpriteStateData["Disp"]["w"];
	m_SpriteState.Disp.h		= m_SpriteStateData["Disp"]["h"];
	m_SpriteState.Base.w		= m_SpriteStateData["Base"]["w"];
	m_SpriteState.Base.h		= m_SpriteStateData["Base"]["h"];
	m_SpriteState.Stride.w		= m_SpriteStateData["Stride"]["w"];
	m_SpriteState.Stride.h		= m_SpriteStateData["Stride"]["h"];
	m_SpriteState.AddCenterPos	= GetAddCenterPosition();

	// ���߂Ȃ����邩�̎擾.
	m_SpriteRenderState.IsLock	= m_SpriteStateData["IsLock"];

	// �\������V�[���̓ǂݍ���.
	const int& SceneSize = static_cast<int>( m_SpriteStateData["Scene"].size() );
	m_SpriteState.DispSceneList.resize( SceneSize );
	for ( int i = 0; i < SceneSize; ++i ) {
		const std::string& SceneName	= m_SpriteStateData["Scene"][i];
		m_SpriteState.DispSceneList[i]	= SceneName;

		// �\�����̎擾.
		if ( SceneName == "None" )	m_SpriteState.SceneDispNum[SceneName] = 0;
		else						m_SpriteState.SceneDispNum[SceneName] = m_SpriteStateData[SceneName]["Num"];
	}

	// �␳�l�̐ݒ�.
	if ( OffSetData.empty() ) m_SpriteState.OffSet = { 0.0f, 0.0f };
	else {
		m_SpriteState.OffSet.x	= OffSetData["x"];
		m_SpriteState.OffSet.y	= OffSetData["y"];
	}

#if _DEBUG
	// �t�@�C���p�X���X�V����.
	m_SpriteStateData["FilePath"]	= TextPath;
	if ( FAILED( FileManager::JsonSave( TextPath, m_SpriteStateData ) ) ) return E_FAIL;
#endif	// #if _DEBUG.
	return S_OK;
}

//----------------------------.
// �X�v���C�g���̕ۑ����čēǂݍ���.
//----------------------------.
HRESULT CSprite::SpriteStateSaveReload()
{
	// ���݂̎��Ԃ��擾.
	auto now	= std::chrono::system_clock::now();
	auto now_c	= std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

	// �t�@�C�������擾.
	std::string FileName = "";
	std::string::size_type Spos = m_SpriteState.FilePath.rfind( "\\" );
	if ( Spos != std::string::npos ) {
		std::string::size_type Epos = m_SpriteState.FilePath.find( ".", Spos + 1 );
		if ( Epos != std::string::npos ) {
			FileName = m_SpriteState.FilePath.substr( Spos + 1, Epos - Spos - 1 );
		}
	}

	// �t�@�C���p�X�̍쐬.
	std::string TextPath	= m_SpriteState.FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

	// ���O�t�@�C���̍쐬.
	if ( *m_pIsCreaterLog ) {
		// �t�@�C���p�X�̍쐬.
		std::string LogTextPath = UNDO_LOG_FILE_PATH + Time.str() + "_" + FileName + ".json";

		// �t�@�C���̍쐬.
		json j = FileManager::JsonLoad( TextPath );
		if ( j != m_SpriteStateData ) {
			FileManager::CreateFileDirectory( UNDO_LOG_FILE_PATH );
			if ( std::rename( TextPath.c_str(), LogTextPath.c_str() ) ) return E_FAIL;
			m_pLogList->emplace_back( LogTextPath );
			Log::PushLog( TextPath + " �t�@�C���̃��O�̍쐬 : ����" );
		}
	}

	// �X�v���C�g���̕ۑ�.
	if ( FAILED( FileManager::JsonSave( TextPath, m_SpriteStateData ) ) ) return E_FAIL;
	Log::PushLog( TextPath + " �t�@�C���̍쐬 : ����" );

	// �쐬�ł������߂�����x�ǂݍ��ݒ���.
	Log::PushLog( m_SpriteState.FilePath + " ��������x�ǂݍ��ݒ����܂��B" );
	if ( FAILED( SpriteStateDataLoad() ) ) return E_FAIL;
	return S_OK;
}

//----------------------------.
// �\������X�v���C�g�̍폜.
//----------------------------.
HRESULT CSprite::DispSpriteDeleteScene( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	// �\������V�[������폜����.
	std::vector<std::string> SceneList = m_SpriteStateData["Scene"];
	m_SpriteStateData["Scene"].clear();
	for ( auto& s : SceneList ) {
		if ( s != DispScene ) m_SpriteStateData["Scene"].emplace_back( s );
	}
	m_SpriteStateData.erase( DispScene );

	// �ύX���ۑ�����.
	if ( IsAutoSave ) {
		// �ۑ����čēǂݍ���.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// �\�����Ă���X�v���C�g�̐������炷.
//----------------------------.
HRESULT CSprite::DispSpriteDeleteNum( const bool IsAutoSave, const std::string& DispScene, const int No, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	// ���݂̕\�����Ă��鐔���擾.
	const int NowNum = m_SpriteStateData[DispScene]["Num"];
	if ( NowNum <= No ) return E_FAIL;

	// �\�����Ă��鐔��1�̂��߃V�[�����ƍ폜����.
	if ( NowNum == 1 ) return DispSpriteDeleteScene( IsAutoSave, DispScene, RenderState );

	// �Ō�̗v�f�ł͖����ꍇ.
	if ( NowNum - 1 != No ) {
		for ( int i = No + 1; i < NowNum; ++i ) {
			// �폜���鏊�����ɂ��炵�Ă���.
			m_SpriteStateData[DispScene][std::to_string( i - 1 )] = m_SpriteStateData[DispScene][std::to_string( i )];
		}
	}

	// �\������������炷.
	m_SpriteStateData[DispScene]["Num"] = NowNum - 1;
	m_SpriteStateData[DispScene].erase( std::to_string( NowNum - 1 ) );

	// �ύX���ۑ�����.
	if ( IsAutoSave ) {
		// �ۑ����čēǂݍ���.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// �\������X�v���C�g�̒ǉ�.
//----------------------------.
HRESULT CSprite::AddDispSprite( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	bool IsNewScene = true;
	for ( auto& s : m_SpriteState.DispSceneList ) {
		// �ǉ�����V�[���ɂ��łɕ`�悵�Ă��邩.
		if ( s == DispScene ) {
			IsNewScene = false;
			break;
		}
	}
	
	// �V�����\������V�[����ǉ�����.
	if ( IsNewScene ) {
		// �\������V�[���̒ǉ�.
		m_SpriteStateData["Scene"].emplace_back( DispScene );
		const std::vector<std::string> SceneList = m_SpriteStateData["Scene"];
		m_SpriteStateData["Scene"].clear();
		m_SpriteState.DispSceneList.clear();
		for ( auto& s : SceneList ) {
			if ( s != "None" ) {
				m_SpriteStateData["Scene"].emplace_back( s );
				m_SpriteState.DispSceneList.emplace_back( s );
			}
		}
		m_SpriteStateData[DispScene]["Num"] = 1;

		// ���f�[�^���쐬.
		NewDispSpriteTmpData( RenderState, DispScene, 0 );
	}
	// �\��������𑝂₷.
	else {
		// ���݂̕\�����Ă��鐔���擾.
		const int NowNum = m_SpriteStateData[DispScene]["Num"];

		// �\��������𑝂₷.
		m_SpriteStateData[DispScene]["Num"] = NowNum + 1;

		// ���f�[�^���쐬.
		NewDispSpriteTmpData( RenderState, DispScene, NowNum );
	}

	// �ύX���ۑ�����.
	if ( IsAutoSave ) {
		// �ۑ����čēǂݍ���.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return E_FAIL;
	}
	return S_OK;
}

//---------------------------.
// �X�v���C�g�̍���̍��W���擾����.
//---------------------------.
D3DXVECTOR3 CSprite::GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const D3DXPOSITION3&	Pos			= RenderState->Transform.Position;
	const SSize&			Size		= m_SpriteState.Disp;
	const D3DXSCALE3		Scale		= RenderState->Transform.Scale;

	// ����̍��W���v�Z���ĕԂ�.
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::Left:		return D3DXPOSITION3( Pos.x,							Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	case ELocalPosition::LeftDown:	return D3DXPOSITION3( Pos.x,							Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::Down:		return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::RightDown:	return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::Right:		return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	case ELocalPosition::RightUp:	return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y,								Pos.z );
	case ELocalPosition::Up:		return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y,								Pos.z );
	case ELocalPosition::Center:	return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	default:						return Pos;
	}
}

//----------------------------.
// �V�F�[�_�쐬.
//----------------------------.
HRESULT CSprite::CreateShader()
{
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSL����o�[�e�b�N�X�V�F�[�_�̃u���u���쐬.
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME,		// �V�F�[�_�t�@�C����(HLSL�t�@�C��).
			nullptr,			// �}�N����`�̔z��ւ̃|�C���^(���g�p).
			nullptr,			// �C���N���[�h�t�@�C���������C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
			"VS_Main",			// �V�F�[�_�G���g���[�|�C���g�֐��̖��O.
			"vs_5_0",			// �V�F�[�_�̃��f�����w�肷�镶����(�v���t�@�C��).
			uCompileFlag,		// �V�F�[�_�R���p�C���t���O.
			0,					// �G�t�F�N�g�R���p�C���t���O(���g�p).
			nullptr,			// �X���b�h �|���v �C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
			&pCompiledShader,	// �u���u���i�[���郁�����ւ̃|�C���^.
			&pErrors,			// �G���[�ƌx���ꗗ���i�[���郁�����ւ̃|�C���^.
			nullptr)))			// �߂�l�ւ̃|�C���^(���g�p).
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// ��L�ō쐬�����u���u����u�o�[�e�b�N�X�V�F�[�_�v���쐬.
	if (FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr,
			&m_pVertexShader)))	// (out)�o�[�e�b�N�X�V�F�[�_.
	{
		ErrorMessage( "�o�[�e�b�N�X�V�F�[�_�쐬���s" );
		return E_FAIL;
	}

	// ���_�C���v�b�g���C�A�E�g���`.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{
			"POSITION",						// �ʒu.
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,	// DXGI�̃t�H�[�}�b�g(32bit float�^*3).
			0,
			0,								// �f�[�^�̊J�n�ʒu.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD",						// �e�N�X�`���ʒu.
			0,
			DXGI_FORMAT_R32G32_FLOAT,		// DXGI�̃t�H�[�}�b�g(32bit float�^*2).
			0,
			12,								// �f�[�^�̊J�n�ʒu.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	// ���_�C���v�b�g���C�A�E�g�̔z��v�f�����Z�o.
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// ���_�C���v�b�g���C�A�E�g���쐬.
	if (FAILED(
		DirectX11::MutexDX11CreateInputLayout(
			layout,
			numElements,
			pCompiledShader,
			&m_pVertexLayout)))	//(out)���_�C���v�b�g���C�A�E�g.
	{
		ErrorMessage( "���_�C���v�b�g���C�A�E�g�쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	const int Size = static_cast<int>( m_pPixelShaders.size() );
	for( int i = 0; i < Size; ++i ){
		// HLSL����s�N�Z���V�F�[�_�̃u���u���쐬.
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME,				// �V�F�[�_�t�@�C����(HLSL�t�@�C��).
				nullptr,					// �}�N����`�̔z��ւ̃|�C���^(���g�p).
				nullptr,					// �C���N���[�h�t�@�C���������C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
				PS_SHADER_ENTRY_NAMES[i],	// �V�F�[�_�G���g���[�|�C���g�֐��̖��O.
				"ps_5_0",					// �V�F�[�_�̃��f�����w�肷�镶����(�v���t�@�C��).
				uCompileFlag,				// �V�F�[�_�R���p�C���t���O.
				0,							// �G�t�F�N�g�R���p�C���t���O(���g�p).
				nullptr,					// �X���b�h �|���v �C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
				&pCompiledShader,			// �u���u���i�[���郁�����ւ̃|�C���^.
				&pErrors,					// �G���[�ƌx���ꗗ���i�[���郁�����ւ̃|�C���^.
				nullptr)))					// �߂�l�ւ̃|�C���^(���g�p).
		{
			ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
			return E_FAIL;
		}
		SAFE_RELEASE(pErrors);

		// ��L�ō쐬�����u���u����u�s�N�Z���V�F�[�_�v���쐬.
		if (FAILED(
			DirectX11::MutexDX11CreatePixelShader(
				pCompiledShader,
				nullptr,
				&m_pPixelShaders[i] )))	// (out)�s�N�Z���V�F�[�_.
		{
			ErrorMessage( "�s�N�Z���V�F�[�_�u" + std::string( PS_SHADER_ENTRY_NAMES[i] ) + "�v�쐬���s" );
			return E_FAIL;
		}
		SAFE_RELEASE(pCompiledShader);
	}

	// �R���X�^���g(�萔)�o�b�t�@�쐬.
	//	�V�F�[�_�ɓ���̐��l�𑗂�o�b�t�@.
	//	�����ł͕ϊ��s��n���p.
	//	�V�F�[�_�� World, View, Projection �s���n��.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;				// �R���X�^���g�o�b�t�@���w��.
	cb.ByteWidth			= sizeof( SSpriteShaderConstantBuffer );	// �R���X�^���g�o�b�t�@�̃T�C�Y.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;					// �������݂ŃA�N�Z�X.
	cb.MiscFlags			= 0;										// ���̑��̃t���O(���g�p).
	cb.StructureByteStride	= 0;										// �\���̂̃T�C�Y(���g�p).
	cb.Usage				= D3D11_USAGE_DYNAMIC;						// �g�p���@:���ڏ�������.

	// �R���X�^���g�o�b�t�@�̍쐬.
	if (FAILED(
		m_pDevice->CreateBuffer(
			&cb,
			nullptr,
			&m_pConstantBuffer)))
	{
		ErrorMessage( "�R���X�^���g�o�b�t�@�쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// ���f���쐬(UI).
//----------------------------.
HRESULT CSprite::CreateModelUI()
{
	SSpriteAnimState* AnimState = &m_SpriteRenderState.AnimState;

	float w = m_SpriteState.Disp.w;
	float h = m_SpriteState.Disp.h;
	float u = m_SpriteState.Stride.w / m_SpriteState.Base.w;// 1�}�X������̕�.
	float v = m_SpriteState.Stride.h / m_SpriteState.Base.h;// 1�}�X������̍���.

	// x,y���ꂼ��̍ő�}�X��.
	AnimState->PatternMax.x		= static_cast<SHORT>( m_SpriteState.Base.w / m_SpriteState.Stride.w );
	AnimState->PatternMax.y		= static_cast<SHORT>( m_SpriteState.Base.h / m_SpriteState.Stride.h );
	AnimState->AnimNumberMax	= AnimState->PatternMax.x * AnimState->PatternMax.y;

	// �|��(�l�p�`)�̒��_���쐬.
	//	���_���W(x,y,z), UV���W(u,v).
	CreateVertex( w, h, u, v );
	// �ő�v�f�����Z�o����.
	UINT uVerMax = sizeof(m_Vertices) / sizeof(m_Vertices[0]);

	// �o�b�t�@�\����.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// �g�p���@(�f�t�H���g).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// ���_�̃T�C�Y.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// ���_�o�b�t�@�Ƃ��Ĉ���.
	bd.CPUAccessFlags		= 0;							// CPU����̓A�N�Z�X���Ȃ�.
	bd.MiscFlags			= 0;							// ���̑��̃t���O(���g�p).
	bd.StructureByteStride	= 0;							// �\���̂̃T�C�Y(���g�p).

	// �T�u���\�[�X�f�[�^�\����.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;	// �|���̒��_���Z�b�g.

	// ���_�o�b�t�@�̍쐬.
	if (FAILED( m_pDevice->CreateBuffer(
		&bd, &InitData, &m_pVertexBufferUI)))
	{
		ErrorMessage( "���_�o�b�t�@UI�쐬���s" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// ���f���쐬(3D).
//----------------------------.
HRESULT CSprite::CreateModel3D()
{
	// �摜�̔䗦�����߂�.
	int		as = myGcd( static_cast<int>( m_SpriteState.Disp.w ), static_cast<int>( m_SpriteState.Disp.h ) );
	float	w = ( m_SpriteState.Disp.w / as ) * m_SpriteState.Disp.w * 0.1f;
	float	h = ( m_SpriteState.Disp.h / as ) * m_SpriteState.Disp.h * 0.1f;
	float	u = m_SpriteState.Stride.w / m_SpriteState.Base.w;	// 1�R�}������̕�,
	float	v = m_SpriteState.Stride.h / m_SpriteState.Base.h;	// 1�R�}������̍���.

	// �|��(�l�p�`)�̒��_���쐬.
	//	���_���W(x,y,z), UV���W(u,v).
	CreateVertex( w, -h, u, v );
	// �ő�v�f�����Z�o����.
	UINT uVerMax = sizeof( m_Vertices ) / sizeof( m_Vertices[0] );

	// �o�b�t�@�\����.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// �g�p���@(�f�t�H���g).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// ���_�̃T�C�Y.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// ���_�o�b�t�@�Ƃ��Ĉ���.
	bd.CPUAccessFlags		= 0;							// CPU����̓A�N�Z�X���Ȃ�.
	bd.MiscFlags			= 0;							// ���̑��̃t���O(���g�p).
	bd.StructureByteStride	= 0;							// �\���̂̃T�C�Y(���g�p).

	// �T�u���\�[�X�\����.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;

	// ���_�o�b�t�@�̍쐬.
	if (FAILED( m_pDevice->CreateBuffer(
		&bd, &InitData, &m_pVertexBuffer3D)))
	{
		ErrorMessage( "���_�o�b�t�@3D�쐬���s" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// �e�N�X�`���쐬.
//----------------------------.
HRESULT CSprite::CreateTexture()
{
	// �t�@�C���p�X�� w_char �ɕϊ�����.
	const std::wstring wFilePath = StringConversion::to_wString( m_SpriteState.FilePath );

	//�e�N�X�`���쐬.
	if( FAILED( DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
		wFilePath.c_str(),	// �t�@�C����.
		nullptr, nullptr,
		&m_pTexture,		// (out)�e�N�X�`��.
		nullptr ) ) )
	{
		ErrorMessage( "�e�N�X�`���쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// �T���v���쐬.
//----------------------------.
HRESULT CSprite::CreateSampler()
{
	// �e�N�X�`���p�̃T���v���\����.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof(samDesc) );
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;

	// �T���v���쐬.
	for( int i = 0; i < static_cast<int>( ESamplerState::Max ); i++ ){
		samDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		if( FAILED( m_pDevice->CreateSamplerState( &samDesc, &m_pSampleLinears[i] ))){
			ErrorMessage( "�T���v���쐬 : ���s" );
			return E_FAIL;
		}
	}
	return S_OK;
}

//---------------------------.
// ���_���̍쐬.
//---------------------------.
HRESULT CSprite::CreateVertex( const float w, const float h, const float u, const float v )
{
	// �|��(�l�p�`)�̒��_���쐬.
	//	���_���W(x,y,z), UV���W(u,v).
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(    w,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Left:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(    w,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(    w, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::LeftDown:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(    w,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Down:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::RightDown:
		m_Vertices[0] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3(   -w,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Right:
		m_Vertices[0] = { D3DXPOSITION3(   -w,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3(   -w, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::RightUp:
		m_Vertices[0] = { D3DXPOSITION3(   -w,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Up:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Center:
	default: 
		m_Vertices[0] = { D3DXVECTOR3( -w / 2,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXVECTOR3( -w / 2, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXVECTOR3(  w / 2,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXVECTOR3(  w / 2, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	}
	return S_OK;
}

//---------------------------.
// �X�v���C�g���̍쐬.
//---------------------------.
HRESULT CSprite::CreateSpriteState()
{
	// �쐬����t�@�C���p�X.
	std::string TextPath = m_SpriteState.FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

	// �摜�̕��A�����̎擾.
	const D3DXVECTOR2& BaseSize = ImageSize::GetImageSize( m_SpriteState.FilePath );

	// ����ǉ����Ă���.
	json SpriteState;
	SpriteState["LocalPosition"]		= 0;
	SpriteState["Disp"]["w"]			= BaseSize.x;
	SpriteState["Disp"]["h"]			= BaseSize.y;
	SpriteState["Base"]["w"]			= BaseSize.x;
	SpriteState["Base"]["h"]			= BaseSize.y;
	SpriteState["Stride"]["w"]			= BaseSize.x;
	SpriteState["Stride"]["h"]			= BaseSize.y;
	SpriteState["Scene"]				= { "None" };
	SpriteState["IsLock"]				= false;

	// �X�v���C�g���̍쐬.
	if( FAILED( FileManager::JsonSave( TextPath, SpriteState ) ) ) return E_FAIL;
	Log::PushLog( TextPath + " �t�@�C���̍쐬 : ����" );

	// �쐬����.
	return S_OK;
}

//---------------------------.
// ���S���W�ɕϊ����邽�߂̒ǉ����W�̎擾.
//---------------------------.
D3DXPOSITION3 CSprite::GetAddCenterPosition()
{
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f,  m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Left:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f,  0.0f,							0.0f );
	case ELocalPosition::LeftDown:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f, -m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Down:
		return D3DXPOSITION3(  0.0f,						-m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::RightDown:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f, -m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Right:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f,  0.0f,							0.0f );
	case ELocalPosition::RightUp:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f,  m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Up:
		return D3DXPOSITION3(  0.0f,						 m_SpriteState.Disp.h / 2.0f,	0.0f );
	default:
		return INIT_FLOAT3;
	}
}

//---------------------------.
// UI�A�j���[�V�����̍X�V.
//	UI�A�j���[�V�������s��Ȃ��ꍇ�͏����͍s��Ȃ�.
//---------------------------.
void CSprite::UIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhen	= RenderState->UIAnimState.UIAnimWhen;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;

	// UI�A�j���[�V�������s�������ׂ�.
	if ( UIAnimWhen							== "None"	) return;
	if ( UIAnimPlayCheck( pRenderState )	== false	) return;

	// UI�A�j���[�V�����̏�����.
	UIAnimInit( pRenderState );

	// UI�̃A�j���[�V�������s��.
	if (	  UIAnimWhat == "Scale"		) ScaleUIAnimUpdate(	 pRenderState );	// �g�k.
	else if ( UIAnimWhat == "Alpha"		) AlphaUIAnimUpdate(	 pRenderState );	// ����/�s�����̓_��.
	else if ( UIAnimWhat == "Rotate"	) RotateUIAnimUpdate(	 pRenderState );	// ��].
	else if ( UIAnimWhat == "Move"		) MoveUIAnimUpdate(		 pRenderState );	// �ړ�.
	else if ( UIAnimWhat == "Pendulum"	) PendulumUIAnimUpdate(	 pRenderState );	// �U��q.
	else if ( UIAnimWhat == "Animation"	) AnimationUIAnimUpdate( pRenderState );	// �A�j���[�V����.
	else if ( UIAnimWhat == "Scroll"	) ScrollUIAnimUpdate(	 pRenderState );	// UV�X�N���[��.
	else if ( UIAnimWhat == "Message"	) MessageUIAnimUpdate(	 pRenderState );	// ���b�Z�[�W.
}

//----------------------------.
// UI�A�j���[�V�����̏�����.
//----------------------------.
void CSprite::UIAnimInit( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;
	std::string*		UIAnimWhat	= &RenderState->UIAnimState.UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �A�j���[�V����.
	if ( *UIAnimWhat == "Animation" ) {
		AnimState->IsAnimation = false;
	}
	// UV�X�N���[��.
	else if ( *UIAnimWhat == "Scroll" ) {
		if ( AnimData["Scroll"]["IsPlay"]["x"] ) {
			AnimState->IsUVScrollX = false;
		}
		else if ( AnimData["Scroll"]["IsPlay"]["y"] ) {
			AnimState->IsUVScrollY = false;
		}
	}
}

//----------------------------.
// UI�A�j���[�V�������s�������ׂ�.
//----------------------------.
bool CSprite::UIAnimPlayCheck( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;

	// UI�Ƃ��ĕ\������V�[�����ݒ肳��Ă��Ȃ����ߍs��Ȃ�.
	if ( RenderState->SceneName == "" ) return false;

	const std::string&	UIAnimWhen	= UIAnimState->UIAnimWhen;
	const std::string&	UIAnimWhat	= UIAnimState->UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �A�j���[�V�������Đ�������.
	if ( UIAnimState->IsAnimPlay ) {
		return true;
	}
	// �펞.
	else if ( UIAnimWhen == "Always" ) {
		return true;
	}
	// �}�E�X�J�[�\�����d�Ȃ�����.
	else if ( UIAnimWhen == "CursorOverlap" ) {
		return GetIsOverlappingTheMouse( pRenderState );
	}
	// �N���b�N���ꂽ��.
	else if ( UIAnimWhen == "Click" ) {
		bool IsClick = false;
		if ( AnimData["Click"]["Left"]		) IsClick = KeyInput::IsKeyDown( VK_LBUTTON );
		if ( AnimData["Click"]["Center"]	) IsClick = KeyInput::IsKeyDown( VK_MBUTTON );
		if ( AnimData["Click"]["Right"]		) IsClick = KeyInput::IsKeyDown( VK_RBUTTON );
		return GetIsOverlappingTheMouse( pRenderState ) && IsClick;
	}
	// ����̃L�[��������Ă��鎞.
	else if ( UIAnimWhen == "Key" ) {
		if ( AnimData["Key"]["Press"] ) {
			if ( AnimData["Key"]["KeyName"]		!= "None" ) return KeyInput::IsKeyPress(	InputList::StringToKeyData( AnimData["Key"]["KeyName"] ) );
			if ( AnimData["Key"]["ButName"]		!= "None" ) return XInput::IsKeyPress(		InputList::StringToButData( AnimData["Key"]["ButName"] ) );
			if ( AnimData["Key"]["InputName"]	!= "None" ) return Input::IsKeyPress(		AnimData["Key"]["InputName"] );
		}
		if ( AnimData["Key"]["Down"] ) {
			if ( AnimData["Key"]["KeyName"]		!= "None" ) return KeyInput::IsKeyDown(	InputList::StringToKeyData( AnimData["Key"]["KeyName"] ) );
			if ( AnimData["Key"]["ButName"]		!= "None" ) return XInput::IsKeyDown(		InputList::StringToButData( AnimData["Key"]["ButName"] ) );
			if ( AnimData["Key"]["InputName"]	!= "None" ) return Input::IsKeyDown(		AnimData["Key"]["InputName"] );
		}
		if ( AnimData["Key"]["Up"] ) {
			if ( AnimData["Key"]["KeyName"]		!= "None" ) return KeyInput::IsKeyUp(		InputList::StringToKeyData( AnimData["Key"]["KeyName"] ) );
			if ( AnimData["Key"]["ButName"]		!= "None" ) return XInput::IsKeyUp(		InputList::StringToButData( AnimData["Key"]["ButName"] ) );
			if ( AnimData["Key"]["InputName"]	!= "None" ) return Input::IsKeyUp(			AnimData["Key"]["InputName"] );
		}
	}
	// ���b�Z�[�W���󂯎������.
	else if ( UIAnimWhen == "GetMessage" ) {
		return Message::Check( AnimData["GetMessage"]["Name"] );
	}
	// �����_��( �m�� ).
	else if ( UIAnimWhen == "Random" ) {
		return Random::GetRand( 1, 100 ) <= AnimData["Random"]["Probability"];
	}
	return false;
}

//----------------------------.
// �g�kUI�A�j���[�V����.
//----------------------------.
void CSprite::ScaleUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	D3DXSCALE3*			Scale		= &RenderState->Transform.Scale;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	const D3DXVECTOR2&	ShrinkSpeed	= {
		AnimData["Scale"]["ShrinkSpeed"]["x"],
		AnimData["Scale"]["ShrinkSpeed"]["y"]
	};
	const D3DXVECTOR2& ExpansionSpeed = {
		AnimData["Scale"]["ExpansionSpeed"]["x"],
		AnimData["Scale"]["ExpansionSpeed"]["y"]
	};
	const D3DXVECTOR2& MaxSize = {
		AnimData["Scale"]["MaxSize"]["x"],
		AnimData["Scale"]["MaxSize"]["y"]
	};
	const D3DXVECTOR2& MinSize = {
		AnimData["Scale"]["MinSize"]["x"],
		AnimData["Scale"]["MinSize"]["y"]
	};

	// ����̉񐔕��A�j���[�V�������Đ�������.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// �g�k������.
	bool IsCnt = false;
	if ( UIAnimState->Flag_x )	Scale->x -= ShrinkSpeed.x;
	else						Scale->x += ExpansionSpeed.x;
	if ( UIAnimState->Flag_y )	Scale->y -= ShrinkSpeed.y;
	else						Scale->y += ExpansionSpeed.y;

	// �g�k�𔽓]������.
	if ( Scale->x >= MaxSize.x ) {
		UIAnimState->Flag_x = true;
		IsCnt = true;
	}
	else if ( Scale->x <= MinSize.x ) {
		UIAnimState->Flag_x = false;
		IsCnt = true;
	}
	if ( Scale->y >= MaxSize.y ) {
		UIAnimState->Flag_y = true;
		IsCnt				= true;
	}
	else if ( Scale->y <= MinSize.y ) {
		UIAnimState->Flag_y = false;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// ����/�s����UI�A�j���[�V����.
//----------------------------.
void CSprite::AlphaUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState		= &RenderState->UIAnimState;
	json				AnimData		= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const float&		OpacitySpeed	= AnimData["Alpha"]["Speed"]["Opacity"];
	const float&		ClearSpeed		= AnimData["Alpha"]["Speed"]["Clear"];
	const float&		MaxAlpha		= AnimData["Alpha"]["MaxAlpha"];
	const float&		MinAlpha		= AnimData["Alpha"]["MinAlpha"];

	// ����̉񐔕��A�j���[�V�������Đ�������.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// ����/�s�����ɂ��Ă���.
	if ( UIAnimState->Flag_x )	RenderState->Color.w -= OpacitySpeed;
	else						RenderState->Color.w += ClearSpeed;

	// ���]������.
	bool IsCnt = false;
	if ( RenderState->Color.w >= MaxAlpha ) {
		UIAnimState->Flag_x = true;
		IsCnt				= true;
	}
	else if ( RenderState->Color.w <= MinAlpha ) {
		UIAnimState->Flag_x = false;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// ��]UI�A�j���[�V����.
//----------------------------.
void CSprite::RotateUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	D3DXROTATION3*		Rot			= &RenderState->Transform.Rotation;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const std::string&	Direction	= AnimData["Rotate"]["Direction"];
	const float&		Speed		= AnimData["Rotate"]["Speed"];

	// ����̉񐔕��A�j���[�V�������Đ�������.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// ��]�����ɉ����ĉ�]������.
	if ( Direction == "Right" ) Rot->z += Math::ToRadian( Speed );
	else						Rot->z -= Math::ToRadian( Speed );

	// �p�x��0 ~ 360�ȓ��Ɏ��߂�.
	bool IsCnt = false;
	if ( Rot->z >= Math::RADIAN_MAX ) {
		Rot->z -= Math::RADIAN_MAX;
		IsCnt = true;
	}
	else if ( Rot->z <= Math::RADIAN_MIN ) {
		Rot->z += Math::RADIAN_MIN;
		IsCnt = true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//---------------------------.
// �ړ�UI�A�j���[�V����.
//---------------------------.
void CSprite::MoveUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*			UIAnimState = &RenderState->UIAnimState;
	D3DXPOSITION3*			Pos			= &RenderState->Transform.Position;
	json					AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const std::string&		XEasingName	= AnimData["Move"]["EasingName"]["x"];
	const std::string&		YEasingName	= AnimData["Move"]["EasingName"]["y"];
	const D3DXPOSITION3&	StartPos	= RenderState->UIAnimState.StartTransform.Position;
	const D3DXPOSITION2&	EndPos		= { AnimData["Move"]["End"]["x"],	AnimData["Move"]["End"]["y"] };
	const D3DXVECTOR2&		Speed		= { AnimData["Move"]["Speed"]["x"], AnimData["Move"]["Speed"]["y"] };
	const D3DXVECTOR2&		Back		= { AnimData["Move"]["Back"]["x"],	AnimData["Move"]["Back"]["y"] };

	// ����̉񐔕��A�j���[�V�������Đ�������.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// ������.
	UIAnimState->Cnt_x += Speed.x;
	UIAnimState->Cnt_y += Speed.y;
	if ( UIAnimState->Flag_x  )	Pos->x = Easing::Easing( XEasingName, UIAnimState->Cnt_x, 100.0f, Back.x, StartPos.x,				StartPos.x + EndPos.x );
	else						Pos->x = Easing::Easing( XEasingName, UIAnimState->Cnt_x, 100.0f, Back.x, StartPos.x + EndPos.x,	StartPos.x );
	if ( UIAnimState->Flag_y  )	Pos->y = Easing::Easing( YEasingName, UIAnimState->Cnt_y, 100.0f, Back.y, StartPos.y,				StartPos.y + EndPos.y );
	else						Pos->y = Easing::Easing( YEasingName, UIAnimState->Cnt_y, 100.0f, Back.y, StartPos.y + EndPos.y,	StartPos.y );
	
	// �ړ������𔽓]������.
	bool IsCnt = false;
	if ( UIAnimState->Cnt_x >= 100.0f ) {
		UIAnimState->Cnt_x	= 0.0f;
		UIAnimState->Flag_x = !UIAnimState->Flag_x;
		IsCnt				= true;
	}
	if ( UIAnimState->Cnt_y >= 100.0f ) {
		UIAnimState->Cnt_y	= 0.0f;
		UIAnimState->Flag_y = !UIAnimState->Flag_y;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// �U��qUI�A�j���[�V����.
//----------------------------.
void CSprite::PendulumUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	D3DXROTATION3*		Rot			= &RenderState->Transform.Rotation;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const float&		Speed		= AnimData["Pendulum"]["Speed"];
	const float&		EndAng		= AnimData["Pendulum"]["EndAng"];

	// ����̉񐔕��A�j���[�V�������Đ�������.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// ��]������.
	if ( UIAnimState->Flag_x )	Rot->z -= Math::ToRadian( Speed );
	else						Rot->z += Math::ToRadian( Speed );

	// ��]�����𔽓]������.
	bool IsCnt = false;
	if ( Rot->z >= Math::ToRadian( EndAng ) ) {
		UIAnimState->Flag_x = true;
		IsCnt				= true;
	}
	else if ( Rot->z <= -Math::ToRadian( EndAng ) ) {
		UIAnimState->Flag_x = false;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// �A�j���[�V����UI�A�j���[�V����.
//----------------------------.
void CSprite::AnimationUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;

	// �A�j���[�V�������g�p����.
	AnimState->IsAnimation = true;
}

//----------------------------.
// UV�X�N���[��UI�A�j���[�V����.
//----------------------------.
void CSprite::ScrollUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const bool&			IsXScroll	= AnimData["Scroll"]["IsPlay"]["x"];
	const bool&			IsYScroll	= AnimData["Scroll"]["IsPlay"]["y"];

	// �X�N���[�����g�p����.
	if ( IsXScroll ) AnimState->IsUVScrollX = true;
	if ( IsYScroll ) AnimState->IsUVScrollY = true;
}

//---------------------------.
// ���b�Z�[�WUI�A�j���[�V����.
//---------------------------.
void CSprite::MessageUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const std::string&	MsgName		= AnimData["Message"];

	// ���b�Z�[�W�𑗂�.
	if ( MsgName.empty() ) return;
	Message::Send( AnimData["Message"], Msg::EMsgType::CheckMsg );
}

//---------------------------.
// UI�A�j���[�V�����̕������ϊ�����.
//---------------------------.
std::string CSprite::ConvertUIAnimationString( const std::string& string )
{
	if(		 string == "None"								) return u8"����";
	else if( string == "Always"								) return u8"�펞";
	else if( string == "CursorOverlap"						) return u8"�}�E�X�J�[�\�����d�Ȃ�����";
	else if( string == "Click"								) return u8"�N���b�N���ꂽ��";
	else if( string == "Key"								) return u8"����̃L�[��������Ă��鎞";
	else if( string == "GetMessage"							) return u8"���b�Z�[�W���󂯎������";
	else if( string == "Random"								) return u8"�����_��( �m�� )";
	else if( string == "Scale"								) return u8"�g�k";
	else if( string == "Alpha"								) return u8"���� / �s�����̓_��";
	else if( string == "Rotate"								) return u8"��]";
	else if( string == "Move"								) return u8"�ړ�";
	else if( string == "Pendulum"							) return u8"�U��q";
	else if( string == "Animation"							) return u8"�A�j���[�V����"	;
	else if( string == "Scroll"								) return u8"UV�X�N���[��";
	else if( string == "Message"							) return u8"���b�Z�[�W";
	else if( string == u8"����"								) return "None";
	else if( string == u8"�펞"								) return "Always";
	else if( string == u8"�}�E�X�J�[�\�����d�Ȃ�����"			) return "CursorOverlap";
	else if( string == u8"�N���b�N���ꂽ��"					) return "Click";
	else if( string == u8"����̃L�[��������Ă��鎞"			) return "Key";
	else if( string == u8"���b�Z�[�W���󂯎������"			) return "GetMessage";
	else if( string == u8"�����_��( �m�� )"					) return "Random";
	else if( string == u8"�g�k"								) return "Scale";
	else if( string == u8"���� / �s�����̓_��"				) return "Alpha";
	else if( string == u8"��]"								) return "Rotate";
	else if( string == u8"�ړ�"								) return "Move";
	else if( string == u8"�U��q"							) return "Pendulum";
	else if( string == u8"�A�j���[�V����"						) return "Animation";
	else if( string == u8"UV�X�N���[��"						) return "Scroll";
	else if( string == u8"���b�Z�[�W"						) return "Message";
	return "None";
}

//---------------------------.
// ���s�����̉��f�[�^���쐬����.
//---------------------------.
void CSprite::CreateWhenTempData( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhen	= RenderState->UIAnimState.UIAnimWhen;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	if ( UIAnimWhen == "Key" && AnimData["Key"].empty() ) {
		AnimData["Key"]["Press"]		= true;
		AnimData["Key"]["Down"]			= false;
		AnimData["Key"]["Up"]			= false;
		AnimData["Key"]["KeyName"]		= "None";
		AnimData["Key"]["ButName"]		= "None";
		AnimData["Key"]["InputName"]	= "None";
	}
	else if ( UIAnimWhen == "Click" && AnimData["Click"].empty() ) {
		AnimData["Click"]["Left"]		= true;
		AnimData["Click"]["Center"]		= false;
		AnimData["Click"]["Right"]		= false;
	}
	else if ( UIAnimWhen == "GetMessage" && AnimData["GetMessage"].empty() ) {
		AnimData["GetMessage"]["Name"]	= "";
	}
	else if ( UIAnimWhen == "Random" && AnimData["Random"].empty() ) {
		AnimData["Random"]["Probability"] = 20;
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// ���s�����̏ڍאݒ�.
//---------------------------.
void CSprite::WhenAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhen	= RenderState->UIAnimState.UIAnimWhen;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;

	if (		UIAnimWhen == "Key"			) KeyAdvancedSetting(			RenderState );	// ����̃L�[��������Ă��鎞�̏ڍאݒ�.
	if (		UIAnimWhen == "Click"		) ClickAdvancedSetting(			RenderState );	// �N���b�N��������Ă��鎞�̏ڍאݒ�.
	else if (	UIAnimWhen == "GetMessage"	) GetMessageAdvancedSetting(	RenderState );	// ���b�Z�[�W���󂯎�������̏ڍאݒ�.
	else if (	UIAnimWhen == "Random"		) RandomAdvancedSetting(		RenderState );	// �����_���̏ڍאݒ�.
}

//---------------------------.
// ����̃L�[��������Ă��鎞�̏ڍאݒ�.
//---------------------------.
void CSprite::KeyAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �{�^�����X�g.
	const std::vector<std::string> ButtonList = {
		u8"�����Ă��鎞",
		u8"�������u��",
		u8"�������u��"
	};

	// ���݂̏�Ԃ��擾.
	std::string NowButton;
	if (		AnimData["Key"]["Press"]	) NowButton = ButtonList[0];
	else if (	AnimData["Key"]["Down"]		) NowButton = ButtonList[1];
	else if (	AnimData["Key"]["Up"]		) NowButton = ButtonList[2];
	
	// �ǂ̃^�C�~���O�ōs�����̃��W�I�{�^�����쐬.
	std::string OldButton = NowButton;
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�ǂ̃^�C�~���O�ōs���܂����H" ) ) {
		ImGuiManager::RadioButton( "KeyTypeRadio##NoName", &NowButton, ButtonList );
		ImGui::TreePop();
	}
	// �O�̏�Ԃƍ��̏�Ԃ����ĕύX����Ă��邩.
	if ( OldButton != NowButton ) {
		// �t���O�̍X�V.
		if ( NowButton == ButtonList[0] ) {
			AnimData["Key"]["Press"]	= true;
			AnimData["Key"]["Down"]		= false;
			AnimData["Key"]["Up"]		= false;
		}
		if ( NowButton == ButtonList[1] ) {
			AnimData["Key"]["Press"]	= false;
			AnimData["Key"]["Down"]		= true;
			AnimData["Key"]["Up"]		= false;
		}
		if ( NowButton == ButtonList[2] ) {
			AnimData["Key"]["Press"]	= false;
			AnimData["Key"]["Down"]		= false;
			AnimData["Key"]["Up"]		= true;
		}
	}

	// �����ꂽ���Ƃ𒲂ׂ�L�[�̖��O�̐ݒ�.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�����ꂽ���Ƃ𒲂ׂ�L�[�̖��O" ) ) {
		std::string KeyName = AnimData["Key"]["KeyName"];
		AnimData["Key"]["KeyName"] = ImGuiManager::Combo("KeyCombo##NoName", KeyName, InputList::GetKeyList());
		ImGui::TreePop();
	}
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�����ꂽ���Ƃ𒲂ׂ�{�^���̖��O" ) ) {
		std::string ButName = AnimData["Key"]["ButName"];
		AnimData["Key"]["ButName"] = ImGuiManager::Combo( "ButCombo##NoName", ButName, InputList::GetButList() );
		ImGui::TreePop();
	}
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�����ꂽ���Ƃ𒲂ׂ鑀��̖��O" ) ) {
		std::string InputName = AnimData["Key"]["InputName"];
		ImGui::InputText( "##KeyName", &InputName );
		if ( InputName == "" ) InputName = "None";
		AnimData["Key"]["InputName"] = InputName;
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// �N���b�N���ꂽ���̏ڍאݒ�.
//---------------------------.
void CSprite::ClickAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �N���b�N���X�g.
	const std::vector<std::string> ClickList = {
		u8"���N���b�N",
		u8"�^�񒆃N���b�N",
		u8"�E�N���b�N"
	};

	// ���݂̏�Ԃ��擾.
	std::string NowButton;
	if (		AnimData["Click"]["Left"]	) NowButton = ClickList[0];
	else if (	AnimData["Click"]["Center"]	) NowButton = ClickList[1];
	else if (	AnimData["Click"]["Right"]	) NowButton = ClickList[2];
	
	// �ǂ̃^�C�~���O�ōs�����̃��W�I�{�^�����쐬.
	std::string OldButton = NowButton;
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�ǂ̃N���b�N�ōs���܂����H" ) ) {
		ImGuiManager::RadioButton( "ClickTypeRadio##NoName", &NowButton, ClickList );
		ImGui::TreePop();
	}
	// �O�̏�Ԃƍ��̏�Ԃ����ĕύX����Ă��邩.
	if ( OldButton != NowButton ) {
		// �t���O�̍X�V.
		if ( NowButton == ClickList[0] ) {
			AnimData["Click"]["Left"]	= true;
			AnimData["Click"]["Center"] = false;
			AnimData["Click"]["Right"]	= false;
		}
		if ( NowButton == ClickList[1] ) {
			AnimData["Click"]["Left"]	= false;
			AnimData["Click"]["Center"] = true;
			AnimData["Click"]["Right"]	= false;
		}
		if ( NowButton == ClickList[2] ) {
			AnimData["Click"]["Left"]	= false;
			AnimData["Click"]["Center"] = false;
			AnimData["Click"]["Right"]	= true;
		}
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// ���b�Z�[�W���󂯎������.
//---------------------------.
void CSprite::GetMessageAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�󂯎�郁�b�Z�[�W��" ) ) {
		std::string MsgName = AnimData["GetMessage"]["Name"];
		ImGui::InputText( "##MsgName", &MsgName );
		AnimData["GetMessage"]["Name"] = MsgName;

		// �e�X�g�p�̃��b�Z�[�W�𑗂�{�^��.
		if( ImGuiManager::Button( u8"���b�Z�[�W�𑗂�", !MsgName.empty() ) ) {
			Message::Send( MsgName, Msg::EMsgType::CheckMsg );
		}
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// �����_���̏ڍאݒ�.
//---------------------------.
void CSprite::RandomAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �m���̐ݒ�.
	int Prob = AnimData["Random"]["Probability"];
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�m���̐ݒ�" ) ) {
		ImGui::SliderInt( "##SliderInt", &Prob, 0, 100 );
		ImGui::InputInt( "##InputInt", &Prob );
		ImGui::TreePop();
	}
	// �m����͈͓��Ɏ��߂�.
	Prob = std::clamp( Prob, 0, 100 );
	AnimData["Random"]["Probability"] = Prob;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// �����s�����̉��f�[�^���쐬����.
//---------------------------.
void CSprite::CreateWhatTempData( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	if ( UIAnimWhat == "Scale" && AnimData["Scale"].empty() ) {
		AnimData["Scale"]["ShrinkSpeed"]["x"]		= 0.01f;
		AnimData["Scale"]["ShrinkSpeed"]["y"]		= 0.01f;
		AnimData["Scale"]["ExpansionSpeed"]["x"]	= 0.01f;
		AnimData["Scale"]["ExpansionSpeed"]["y"]	= 0.01f;
		AnimData["Scale"]["MaxSize"]["x"]			= 1.0f;
		AnimData["Scale"]["MaxSize"]["y"]			= 1.0f;
		AnimData["Scale"]["MinSize"]["x"]			= 0.0f;
		AnimData["Scale"]["MinSize"]["y"]			= 0.0f;
		AnimData["Scale"]["Start"]["x"]				= "Shrink";
		AnimData["Scale"]["Start"]["y"]				= "Shrink";
	}
	else if ( UIAnimWhat == "Alpha" && AnimData["Alpha"].empty() ) {
		AnimData["Alpha"]["Speed"]["Opacity"]		= 0.01f;
		AnimData["Alpha"]["Speed"]["Clear"]			= 0.01f;
		AnimData["Alpha"]["MaxAlpha"]				= 1.0f;
		AnimData["Alpha"]["MinAlpha"]				= 0.0f;
		AnimData["Alpha"]["Start"]					= "Opacity";
	}
	else if ( UIAnimWhat == "Rotate" && AnimData["Rotate"].empty() ) {
		AnimData["Rotate"]["Direction"]				= "Right";
		AnimData["Rotate"]["Speed"]					= 0.1f;
	}
	else if ( UIAnimWhat == "Move" && AnimData["Move"].empty() ) {
		AnimData["Move"]["EasingName"]["x"]			= "Linear";
		AnimData["Move"]["EasingName"]["y"]			= "Linear";
		AnimData["Move"]["End"]["x"]				= 50.0f;
		AnimData["Move"]["End"]["y"]				= 0.0f;
		AnimData["Move"]["Speed"]["x"]				= 1.0f;
		AnimData["Move"]["Speed"]["y"]				= 0.0f;
		AnimData["Move"]["Back"]["x"]				= 0.0f;
		AnimData["Move"]["Back"]["y"]				= 0.0f;
	}
	else if ( UIAnimWhat == "Pendulum" && AnimData["Pendulum"].empty() ) {
		AnimData["Pendulum"]["Speed"]				= 0.1f;
		AnimData["Pendulum"]["EndAng"]				= 30.0f;
		AnimData["Pendulum"]["Start"]				= "Left";
	}
	else if ( UIAnimWhat == "Animation" && AnimData["Animation"].empty() ) {
		AnimData["Anim"]["Time"]					= 0.01f;
	}
	else if ( UIAnimWhat == "Scroll" && AnimData["Scroll"].empty() ) {
		AnimData["Scroll"]["IsPlay"]["x"]			= true;
		AnimData["Scroll"]["IsPlay"]["y"]			= false;
		AnimData["Scroll"]["Speed"]["x"]			= 0.01f;
		AnimData["Scroll"]["Speed"]["y"]			= 0.00f;
	}
	else if ( UIAnimWhat == "Message" && AnimData["Message"].empty() ) {
		AnimData["Message"]							= "";
	}
	AnimData["IsLoop"]		= true;
	AnimData["PlayMax"]		= 0;
	AnimData["EndMessage"]	= "";

	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// �����s�����̏ڍאݒ�.
//----------------------------.
void CSprite::WhatAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	const std::string&	UIAnimWhat	= UIAnimState->UIAnimWhat;
	const std::string&	UIAnimWhen	= UIAnimState->UIAnimWhen;

	if ( UIAnimWhat != "None" ) {
		ImGui::Separator();
		if (		UIAnimWhat == "Scale"		) ScaleAdvancedSetting(		RenderState );	// �g�k�̏ڍאݒ�.
		else if (	UIAnimWhat == "Alpha"		) AlphaAdvancedSetting(		RenderState );	// ����/�s�����̓_�ł̏ڍאݒ�.
		else if (	UIAnimWhat == "Rotate"		) RotateAdvancedSetting(	RenderState );	// ��]�̏ڍאݒ�.
		else if (	UIAnimWhat == "Move"		) MoveAdvancedSetting(		RenderState );	// �ړ��̏ڍאݒ�.
		else if (	UIAnimWhat == "Pendulum"	) PendulumAdvancedSetting(	RenderState );	// �U��q�̏ڍאݒ�.
		else if (	UIAnimWhat == "Animation"	) AnimationAdvancedSetting( RenderState );	// �A�j���[�V�����̏ڍאݒ�.
		else if (	UIAnimWhat == "Scroll"		) ScrollAdvancedSetting(	RenderState );	// UV�X�N���[���̏ڍאݒ�.
		else if (	UIAnimWhat == "Message"		) MessageAdvancedSetting(	RenderState );	// ���b�Z�[�W�̏ڍאݒ�.

		// ���[�v�Đ����邩�̏ڍאݒ�.
		if ( UIAnimWhat == "Animation" || UIAnimWhat == "Scroll" || UIAnimWhen == "Always" ) return;
		LoopAdvancedSetting( RenderState );
	}
}

//----------------------------.
// ���[�v�Đ��̏ڍאݒ�.
//----------------------------.
void CSprite::LoopAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;

	// ���[�v�Đ����s�����̐ݒ�.
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"���[�v�Đ����邩" ) ) {
		ImGuiManager::CheckBox( "##IsLoopCheckBox", &UIAnimState->IsLoop );
		ImGui::TreePop();
	}
	if ( UIAnimState->IsLoop ) return;

	// ���[�v�Đ����s���ꍇ�A����Đ����邩�̐ݒ�.
	ImGui::Separator();
	ImGuiManager::InputInt( u8"�Đ���", &UIAnimState->PlayMax );

	// �Đ��I�����ɑ��郁�b�Z�[�W���̐ݒ�.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�Đ��I�����ɑ��郁�b�Z�[�W��" ) ) {
		ImGui::InputText( "##EndMessage", &UIAnimState->EndMessage );
		ImGui::TreePop();
	}
}

//----------------------------.
// �g�k�̏ڍאݒ�.
//----------------------------.
void CSprite::ScaleAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �ő�T�C�Y�̐ݒ�.
	D3DXVECTOR2 MaxSize = {
		AnimData["Scale"]["MaxSize"]["x"],
		AnimData["Scale"]["MaxSize"]["y"]
	};
	ImGuiManager::InputVec2( u8"�ő�T�C�Y�̐ݒ�", &MaxSize, { 1.0f, 1.0f } );
	AnimData["Scale"]["MaxSize"]["x"] = MaxSize.x;
	AnimData["Scale"]["MaxSize"]["y"] = MaxSize.y;
	// �ŏ��T�C�Y�̐ݒ�.
	D3DXVECTOR2 MinSize = {
		AnimData["Scale"]["MinSize"]["x"],
		AnimData["Scale"]["MinSize"]["y"]
	};
	ImGuiManager::InputVec2( u8"�ŏ��T�C�Y�̐ݒ�", &MinSize );
	AnimData["Scale"]["MinSize"]["x"] = MinSize.x;
	AnimData["Scale"]["MinSize"]["y"] = MinSize.y;

	// �g�呬�x�̐ݒ�.
	D3DXVECTOR2 ExpansionSpeed = {
		AnimData["Scale"]["ExpansionSpeed"]["x"],
		AnimData["Scale"]["ExpansionSpeed"]["y"]
	};
	ImGuiManager::InputVec2( u8"�g�呬�x�̐ݒ�", &ExpansionSpeed, { 0.01f, 0.01f } );
	AnimData["Scale"]["ExpansionSpeed"]["x"] = ExpansionSpeed.x;
	AnimData["Scale"]["ExpansionSpeed"]["y"] = ExpansionSpeed.y;
	// �k�����x�̐ݒ�.
	D3DXVECTOR2 ShrinkSpeed = {
		AnimData["Scale"]["ShrinkSpeed"]["x"],
		AnimData["Scale"]["ShrinkSpeed"]["y"]
	};
	ImGuiManager::InputVec2( u8"�k�����x�̐ݒ�", &ShrinkSpeed, { 0.01f, 0.01f } );
	AnimData["Scale"]["ShrinkSpeed"]["x"] = ShrinkSpeed.x;
	AnimData["Scale"]["ShrinkSpeed"]["y"] = ShrinkSpeed.y;

	// �J�n���̏�ԃ��X�g.
	const std::vector<std::string> StartList = {
		u8"�g�債�Ă���",
		u8"�k�����Ă���"
	};
	// x�����̊J�n���̏�Ԃ̐ݒ�.
	std::string NowStart = AnimData["Scale"]["Start"]["x"] == "Expansion" ?
		u8"�g�債�Ă���" : u8"�k�����Ă���";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"x�����̊J�n���̏�Ԃ̐ݒ�" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowStart, StartList );
		ImGui::TreePop();
	}
	AnimData["Scale"]["Start"]["x"] = NowStart == u8"�g�債�Ă���" ?
		"Expansion" : "Shrink";

	// y�����̊J�n���̏�Ԃ̐ݒ�.
	NowStart = AnimData["Scale"]["Start"]["y"] == "Expansion" ?
		u8"�g�債�Ă���" : u8"�k�����Ă���";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"y�����̊J�n���̏�Ԃ̐ݒ�" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowStart, StartList );
		ImGui::TreePop();
	}
	AnimData["Scale"]["Start"]["y"] = NowStart == u8"�g�債�Ă���" ?
		"Expansion" : "Shrink";
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// ����/�s�����̓_�ł̏ڍאݒ�.
//----------------------------.
void CSprite::AlphaAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �J�n���̓����x�̐ݒ�.
	ImGuiManager::SliderInputFloat( u8"�J�n���̓����x", &RenderState->UIAnimState.StartColor.w, 0.0f, 1.0f );

	// �s�����ɂȂ鑬�x�̐ݒ�.
	float OpacitySpeed = AnimData["Alpha"]["Speed"]["Opacity"];
	ImGuiManager::InputFloat( u8"�s�����ɂȂ鑬�x�̐ݒ�", &OpacitySpeed, 0.01f );
	AnimData["Alpha"]["Speed"]["Opacity"] = OpacitySpeed;

	// �����ɂȂ鑬�x�̐ݒ�.
	float ClearSpeed = AnimData["Alpha"]["Speed"]["Clear"];
	ImGuiManager::InputFloat( u8"�����ɂȂ鑬�x�̐ݒ�", &ClearSpeed, 0.01f );
	AnimData["Alpha"]["Speed"]["Clear"] = ClearSpeed;

	// �ő�̓����x�̐ݒ�.
	float MaxAlpha = AnimData["Alpha"]["MaxAlpha"];
	ImGuiManager::SliderInputFloat( u8"�ő�̓����x�̐ݒ�", &MaxAlpha, 0.0f, 1.0f, 1.0f );
	AnimData["Alpha"]["MaxAlpha"] = MaxAlpha;

	// �ŏ��̓����x�̐ݒ�.
	float MinAlpha = AnimData["Alpha"]["MinAlpha"];
	ImGuiManager::SliderInputFloat( u8"�ŏ��̓����x�̐ݒ�", &MinAlpha, 0.0f, 1.0f, 0.0f );
	AnimData["Alpha"]["MinAlpha"] = MinAlpha;

	// �J�n���̏�ԃ��X�g.
	const std::vector<std::string> StartList = {
		u8"�s�����ɂȂ��Ă���",
		u8"�����ɂȂ��Ă���"
	};
	// �J�n���̏�Ԃ̐ݒ�.
	std::string NowStart = AnimData["Alpha"]["Start"] == "Opacity" ?
		u8"�s�����ɂȂ��Ă���" : u8"�����ɂȂ��Ă���";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�J�n���̏�Ԃ̐ݒ�" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowStart, StartList );
		ImGui::TreePop();
	}
	AnimData["Alpha"]["Start"] = NowStart == u8"�s�����ɂȂ��Ă���" ?
		"Opacity" : "Clear";
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// ��]�̏ڍאݒ�.
//----------------------------.
void CSprite::RotateAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// ��]�������X�g.
	const std::vector<std::string> DireList = {
		u8"���v���( �E��� )",
		u8"�����v���( ����� )"
	};
	// ��]�����̐ݒ�.
	std::string NowDire = AnimData["Rotate"]["Direction"] == "Right" ?
		u8"���v���( �E��� )" : u8"�����v���( ����� )";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"��]�����̐ݒ�" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowDire, DireList );
		ImGui::TreePop();
	}
	AnimData["Rotate"]["Direction"] = NowDire == u8"���v���( �E��� )" ?
		"Right" : "Left";

	// ��]���x�̐ݒ�.
	float Speed = AnimData["Rotate"]["Speed"];
	ImGuiManager::InputFloat( u8"��]���x�̐ݒ�", &Speed, 0.1f );
	AnimData["Rotate"]["Speed"] = Speed;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// �ړ��̏ڍאݒ�.
//----------------------------.
void CSprite::MoveAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// �ړ��I�����W�̐ݒ�.
	D3DXPOSITION2 MoveEndPos;
	MoveEndPos.x = AnimData["Move"]["End"]["x"];
	MoveEndPos.y = AnimData["Move"]["End"]["y"];
	ImGuiManager::InputVec2( u8"�ړ�����", &MoveEndPos );
	AnimData["Move"]["End"]["x"] = MoveEndPos.x;
	AnimData["Move"]["End"]["y"] = MoveEndPos.y;

	// �ړ����x�̐ݒ�.
	D3DXVECTOR2 Speed;
	Speed.x = AnimData["Move"]["Speed"]["x"];
	Speed.y = AnimData["Move"]["Speed"]["y"];
	ImGuiManager::InputVec2( u8"�ړ����x", &Speed );
	AnimData["Move"]["Speed"]["x"] = Speed.x;
	AnimData["Move"]["Speed"]["y"] = Speed.y;

	// �߂鋗���̐ݒ�.
	D3DXVECTOR2 Back;
	Back.x = AnimData["Move"]["Back"]["x"];
	Back.y = AnimData["Move"]["Back"]["y"];
	ImGuiManager::InputVec2( u8"�߂鋗��", &Back );
	AnimData["Move"]["Back"]["x"] = Back.x;
	AnimData["Move"]["Back"]["y"] = Back.y;

	// �g�p����C�[�W���O�̐ݒ�.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�g�p����C�[�W���O" ) ) {
		std::string XEasing = AnimData["Move"]["EasingName"]["x"];
		std::string YEasing = AnimData["Move"]["EasingName"]["y"];
		ImGuiManager::Combo( u8"X����", &XEasing, Easing::GetEasingList() );
		ImGuiManager::Combo( u8"Y����", &YEasing, Easing::GetEasingList() );
		AnimData["Move"]["EasingName"]["x"] = XEasing;
		AnimData["Move"]["EasingName"]["y"] = YEasing;
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// �U��q�̏ڍאݒ�.
//----------------------------.
void CSprite::PendulumAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// ���x�̐ݒ�.
	float Speed = AnimData["Pendulum"]["Speed"];
	ImGuiManager::InputFloat( u8"���x�̐ݒ�", &Speed, 0.1f );
	AnimData["Pendulum"]["Speed"] = Speed;

	// �ŏI�p�x�̐ݒ�.
	float EndAng = AnimData["Pendulum"]["EndAng"];
	ImGuiManager::InputFloat( u8"�ŏI�p�x�̐ݒ�", &EndAng, 30.0f );
	AnimData["Pendulum"]["EndAng"] = EndAng;

	// �J�n���̕������X�g.
	const std::vector<std::string> DireList = {
		u8"���v���( �E��� )",
		u8"�����v���( ����� )"
	};
	// �J�n���̕����̐ݒ�.
	std::string NowDire = AnimData["Pendulum"]["Start"] == "Right" ?
		u8"���v���( �E��� )" : u8"�����v���( ����� )";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"�J�n���̕����̐ݒ�" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowDire, DireList );
		ImGui::TreePop();
	}
	AnimData["Pendulum"]["Start"] = NowDire == u8"���v���( �E��� )" ?
		"Right" : "Left";
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// �A�j���[�V�����̏ڍאݒ�.
//----------------------------.
void CSprite::AnimationAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// ���x�̐ݒ�.
	float Speed = AnimData["Anim"]["Time"];
	ImGuiManager::InputFloat( u8"���x�̐ݒ�", &Speed, 0.01f );
	AnimData["Anim"]["Time"] = Speed;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// UV�X�N���[���̏ڍאݒ�.
//----------------------------.
void CSprite::ScrollAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"UV�X�N���[�����s����" ) ) {
		// x������UV�X�N���[�����s����.
		RenderState->AnimState.IsUVScrollX	= AnimData["Scroll"]["IsPlay"]["x"];
		ImGuiManager::CheckBox( u8"x������UV�X�N���[�����s����", &RenderState->AnimState.IsUVScrollX );
		AnimData["Scroll"]["IsPlay"]["x"]	= RenderState->AnimState.IsUVScrollX;

		// y������UV�X�N���[�����s����.
		RenderState->AnimState.IsUVScrollY	= AnimData["Scroll"]["IsPlay"]["y"];
		ImGuiManager::CheckBox( u8"y������UV�X�N���[�����s����", &RenderState->AnimState.IsUVScrollY );
		AnimData["Scroll"]["IsPlay"]["y"]	= RenderState->AnimState.IsUVScrollY;
		ImGui::TreePop();
	}
	// �X�N���[�����x�̐ݒ�.
	D3DXVECTOR2 ScrollSpeed = {
		AnimData["Scroll"]["Speed"]["x"],
		AnimData["Scroll"]["Speed"]["y"]
	};
	ImGuiManager::InputVec2( u8"�X�N���[�����x�̐ݒ�", &ScrollSpeed, { 0.01f, 0.01f } );
	AnimData["Scroll"]["Speed"]["x"]		= ScrollSpeed.x;
	AnimData["Scroll"]["Speed"]["y"]		= ScrollSpeed.y;
	RenderState->AnimState.ScrollSpeed		= ScrollSpeed;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// ���b�Z�[�W�̏ڍאݒ�.
//---------------------------.
void CSprite::MessageAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// ���郁�b�Z�[�W���̐ݒ�.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"���郁�b�Z�[�W��" ) ) {
		std::string MsgName = AnimData["Message"];
		ImGui::InputText( "##Message", &MsgName );
		AnimData["Message"] = MsgName;
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// �V�����ǉ�����X�v���C�g�̉��f�[�^���쐬����.
//---------------------------.
void CSprite::NewDispSpriteTmpData( SSpriteRenderState* pRenderState, const std::string& SceneName, const int No )
{
	SSpriteRenderState* RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData		= m_SpriteStateData[SceneName][std::to_string( No )];

	// �X�v���C�g�̏�Ԃ�������.
	const SSpriteAnimState AnimState	= RenderState->AnimState;
	*RenderState						= SSpriteRenderState();
	RenderState->AnimState				= AnimState;
	RenderState->SceneName				= SceneName;
	RenderState->No						= std::to_string( No );

	// �K�v�ȉ��f�[�^���쐬����.
	AnimData["Transform"]["Position"]["x"]	= 0.0f;
	AnimData["Transform"]["Position"]["y"]	= 0.0f;
	AnimData["Transform"]["Position"]["z"]	= 0.0f;
	AnimData["Transform"]["Rotation"]["x"]	= 0.0f;
	AnimData["Transform"]["Rotation"]["y"]	= 0.0f;
	AnimData["Transform"]["Rotation"]["z"]	= 0.0f;
	AnimData["Transform"]["Scale"]["x"]		= 1.0f;
	AnimData["Transform"]["Scale"]["y"]		= 1.0f;
	AnimData["Transform"]["Scale"]["z"]		= 1.0f;
	AnimData["Color"]["R"]					= 1.0f;
	AnimData["Color"]["G"]					= 1.0f;
	AnimData["Color"]["B"]					= 1.0f;
	AnimData["Color"]["A"]					= 1.0f;
	AnimData["UIAnimation"]["When"]			= "None";
	AnimData["UIAnimation"]["What"]			= "None";
	m_SpriteStateData[SceneName][std::to_string( No )] = AnimData;
}
