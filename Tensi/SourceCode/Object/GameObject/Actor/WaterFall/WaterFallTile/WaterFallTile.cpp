#include "WaterFallTile.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"

namespace {
	constexpr int WATERFALL_PATTERN_NO	= 0;	// ��̃p�^�[��No
	constexpr int SPLASH_PATTERN_NO		= 1;	// �����Ԃ��̃p�^�[��No
}

CWaterFallTile::CWaterFallTile()
	: m_pWaterFall		( nullptr )
	, m_WaterFallState	()
	, m_SplashState		()
	, m_ImageSize		( INIT_FLOAT )
{
}

CWaterFallTile::~CWaterFallTile()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWaterFallTile::Init()
{
	// �摜�̎擾.
	m_pWaterFall	= SpriteResource::GetSprite( "WaterFall", &m_WaterFallState );
	m_SplashState	= m_WaterFallState;	
	m_ImageSize		= m_pWaterFall->GetSpriteState().Disp.w;

	// �I�u�W�F�N�g�̏�����.
	m_IsDisp								= false;
	m_SplashState.IsDisp					= false;
	m_WaterFallState.AnimState.IsAnimationX	= true;
	m_SplashState.AnimState.IsAnimationX	= true;
	m_ObjectTag								= EObjectTag::WaterFall;
	m_WaterFallState.AnimState.PatternNo.y	= WATERFALL_PATTERN_NO;
	m_SplashState.AnimState.PatternNo.y		= SPLASH_PATTERN_NO;

	const RECT& Rect = WindowManager::GetTaskBarRect();
	m_WaterFallState.RenderArea.w	= static_cast<float>( Rect.top );
	m_SplashState.RenderArea.w		= m_WaterFallState.RenderArea.w;

	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWaterFallTile::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	UpdateCollision();
	if ( m_WaterFallState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::Hit );
}

//---------------------------.
// �`��.
//---------------------------.
void CWaterFallTile::SubRender()
{
	if ( m_IsDisp == false ) return;
	m_pWaterFall->RenderUI( &m_SplashState		);
	m_pWaterFall->RenderUI( &m_WaterFallState	);
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CWaterFallTile::Collision( CActor* pActor )
{
}

//---------------------------.
// �^�C���̐ݒ�
//---------------------------.
void CWaterFallTile::Setting( const D3DXPOSITION3& Pos, const bool IsWater )
{
	m_IsDisp							= true;
	m_WaterFallState.Transform.Position = Pos;
	m_SplashState.Transform.Position	= Pos;
	m_SplashState.IsDisp				= IsWater;
}

//---------------------------.
// �����蔻��̏�����.
//---------------------------.
void CWaterFallTile::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Box2D );
	m_pCollisions->GetCollision<CBox2D>()->SetSprite( m_pWaterFall, &m_WaterFallState );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CWaterFallTile::UpdateCollision()
{
}
