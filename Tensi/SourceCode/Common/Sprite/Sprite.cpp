#include "Sprite.h"
#include "..\DirectX\DirectX11.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\Utility\Input\Input.h"
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
	, m_Vertices				()
	, m_SpriteState				()
	, m_SpriteRenderState		()
	, m_SpriteStateData			()
	, m_pLogList				( nullptr )
	, m_IsAnimPlay				( true )
	, m_IsAllDisp				( true )
	, m_pIsCreaterLog			( nullptr )
{
}

CSprite::~CSprite()
{
	for ( auto& s : m_pSampleLinears )	SAFE_RELEASE( s );
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
	const int PSShaderNo = RenderState->RuleTexture == nullptr ? RenderState->MaskTexture == nullptr || RenderState->IsColorMask ? 0 : 1 : 2;
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
		cb.vFlag.x = RenderState->IsDither == true ? 1.0f : 0.0f;

		// �A���t�@�u���b�N���g�p���邩��n��.
		cb.vFlag.y = RenderState->IsAlphaBlock == true ? 1.0f : 0.0f;

		// �J���[�}�X�N���g�p���邩��n��.
		cb.vFlag.z = RenderState->IsColorMask == true ? 1.0f : 0.0f;

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
	m_pContext->PSSetShaderResources( 1, 1, &RenderState->MaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &RenderState->RuleTexture );

	// �A���t�@�u�����h�L���ɂ���.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( true );

	// �v���~�e�B�u�������_�����O.
	m_pContext->Draw( 4, 0 );// �|��(���_4��).

	// �A���t�@�u�����h�����ɂ���.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( false );
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
	const int PSShaderNo = RenderState->RuleTexture == nullptr ? RenderState->MaskTexture == nullptr || RenderState->IsColorMask ? 0 : 1 : 2;
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
		cb.vFlag.x = RenderState->IsDither == true ? 1.0f : 0.0f;

		// �A���t�@�u���b�N���g�p���邩��n��.
		cb.vFlag.y = RenderState->IsAlphaBlock == true ? 1.0f : 0.0f;

		// �J���[�}�X�N���g�p���邩��n��.
		cb.vFlag.z = RenderState->IsColorMask == true ? 1.0f : 0.0f;

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
	m_pContext->PSSetShaderResources( 1, 1, &RenderState->MaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &RenderState->RuleTexture );

	// �A���t�@�u�����h�L���ɂ���.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( true );

	// �v���~�e�B�u�������_�����O.
	m_pContext->Draw( 4, 0 );// �|��(���_4��).

	// �A���t�@�u�����h�����ɂ���.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( false );
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

	// �␳�l�̐ݒ�.
	if ( OffSetData.empty() ) m_SpriteState.OffSet = { 0.0f, 0.0f };
	else {
		m_SpriteState.OffSet.x	= OffSetData["x"];
		m_SpriteState.OffSet.y	= OffSetData["y"];
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