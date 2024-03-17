#include "Flower.h"
#include "WateringAnimManager/WateringAnimManager.h"
#include "..\..\WindowObject\WindowObject.h"
#include "..\..\WindowObjectManager\WindowObjectManager.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\Input\Input.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\Utility\TimeManager\TimeManager.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\Const\Const.h"

namespace {
	constexpr int	SPRITE_FLOWER_NO		= 0;	// �摜�̉Ԃ̈ʒu.
	constexpr int	SPRITE_LEAF_NO			= 1;	// �摜�̗t���ς̈ʒu.
	constexpr int	SPRITE_WITHER_FLOWER_NO	= 2;	// �摜�̌͂ꂽ�Ԃ̈ʒu.
	constexpr int	SPRITE_WITHER_LEAF_NO	= 3;	// �摜�̌͂ꂽ�t���ς̈ʒu.
}

CFlower::CFlower()
	: m_pFlower				( nullptr )
	, m_LeafState			()
	, m_FlowerState			()
	, m_FlowerSaveData		()
	, m_pWateringAnim		( nullptr )
	, m_AddCenterPosition	( INIT_FLOAT3 )
	, m_FlowerSize			( INIT_FLOAT )
	, m_FlowerAnimCnt		( INIT_FLOAT )
	, m_ReactionAnimCnt		( INIT_FLOAT )
	, m_WateringAnimCnt		( INIT_FLOAT )
	, m_IsReactionAnim		( false )
	, m_WateringCnt			( INIT_INT )
{
}

CFlower::~CFlower()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CFlower::Init()
{
	// �摜�̎擾.
	m_pFlower			= SpriteResource::GetSprite( "Flower", &m_FlowerState );
	m_LeafState			= m_FlowerState;
	m_AddCenterPosition = m_pFlower->GetSpriteState().AddCenterPos;
	m_FlowerSize		= m_pFlower->GetSpriteState().Disp.w;

	// �I�u�W�F�N�g�̏�����.
	m_IsDisp								= false;
	m_IsReactionAnim						= false;
	m_ObjectTag								= EObjectTag::Flower;
	m_FlowerState.AnimState.IsSetPatternNo	= true;
	m_LeafState.AnimState.IsSetPatternNo	= true;
	m_FlowerState.AnimState.PatternNo.x		= SPRITE_FLOWER_NO;
	m_LeafState.AnimState.PatternNo.x		= SPRITE_LEAF_NO;

	m_pWateringAnim = std::make_unique<CWateringAnimManager>();
	m_pWateringAnim->Init();

	const RECT& Rect = WindowManager::GetTaskBarRect();
	m_FlowerState.RenderArea.w	= static_cast<float>( Rect.top );
	m_LeafState.RenderArea.w	= m_FlowerState.RenderArea.w;

	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CFlower::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	FlowerClickUpdate();			// �Ԃ��N���b�N�������̍X�V.
	FlowerAnimUpdate();				// �Ԃ̃A�j���[�V�����̍X�V
	MouseTouchReactionUpdate();		// �}�E�X�ɂ��Ԃ̃A�j���[�V�����̍X�V.
	ReactionAnimUpdate();			// �ԂɐG�ꂽ���̃A�j���[�V�����̍X�V.
	WateringAnimCoolTimeUpdate();	// ����芮���A�j���[�V�����̑ҋ@���Ԃ̍X�V.
	WateringAnimUpdate();			// ����芮���A�j���[�V�����̍X�V
	m_pWateringAnim->Update( DeltaTime );

	TransformUpdate( m_LeafState.Transform );
	UpdateCollision();
	if ( m_FlowerState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::BeAHit );
}

//---------------------------.
// �`��.
//---------------------------.
void CFlower::SubRender()
{
	if ( m_IsDisp == false ) return;

	m_pFlower->RenderUI( &m_LeafState );
	m_pFlower->RenderUI( &m_FlowerState );
	m_pWateringAnim->Render();
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CFlower::Collision( CActor* pActor )
{
	const EObjectTag Tag = pActor->GetObjectTag();
	if ( Tag == EObjectTag::Player && Tag == EObjectTag::Ball ) {
		PlayerBallCollision( pActor, Tag );
	}
	else if ( Tag == EObjectTag::Water ) {
		WaterCollision( pActor, Tag );
	}
}

//---------------------------.
// �Ԃ̃f�[�^���擾.
//---------------------------.
SFlowerData CFlower::GetFlowerData()
{
	// �ꕔ�̃f�[�^�̍X�V
	m_FlowerSaveData.Transform	= m_LeafState.Transform;
	m_FlowerSaveData.Color		= m_FlowerState.Color;
	m_FlowerSaveData.FlowerType	= m_FlowerState.AnimState.PatternNo.y;
	return m_FlowerSaveData;
}

//---------------------------.
// �Ԃ̃f�[�^��ݒ�.
//---------------------------.
void CFlower::SetFlowerData( const SFlowerData& Data )
{
	// �Z�[�u�f�[�^��ݒ�.
	m_IsDisp							= true;
	m_FlowerSaveData					= Data;
	m_Transform							= Data.Transform;
	m_FlowerState.Transform				= Data.Transform;
	m_LeafState.Transform				= Data.Transform;
	m_FlowerState.Color					= Data.Color;
	m_FlowerState.AnimState.PatternNo.y = Data.FlowerType;
	m_LeafState.AnimState.PatternNo.y	= Data.FlowerType;

	// �͂�Ă���ꍇ�����ڂ̕ύX
	if ( m_FlowerSaveData.IsWither ) {
		m_FlowerState.AnimState.PatternNo.x	= SPRITE_WITHER_FLOWER_NO;
		m_LeafState.AnimState.PatternNo.x	= SPRITE_WITHER_LEAF_NO;
	}

	// ��������肵�����̍X�V.
	const std::tm& Now = TimeManager::GetTime();
	m_FlowerSaveData.IsWatering = TimeManager::GetIsSameDay( Now, Data.WateringDay );

	// ����肵�Ă��Ȃ����Ԃ������Ԍo�߂����ꍇ�͂��.
	if ( TimeManager::GetTimediff( Now, Data.WateringDay ).tm_mday >= Const::Flower.WITHER_DAY ) {
		Wither();
	}
	
	// �͂�Ă��Ĉ����Ԍo�߂����ꍇ���ł���.
	if ( TimeManager::GetTimediff( Now, Data.WitherDay ).tm_mday >= Const::Flower.DELETE_DAY ) {
		m_IsDisp = false;
	}
}

//---------------------------.
// �c�؂𖄂߂�.
//---------------------------.
void CFlower::Fill( const int Type, const D3DXCOLOR3& Color, const D3DXPOSITION3& Pos )
{
	if ( m_IsDisp ) return;

	// �^�X�N�o�[�̃T�C�Y���擾.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// ������ۑ�
	m_FlowerSaveData.FillDay		= TimeManager::GetTime();
	m_FlowerSaveData.WateringDay	= TimeManager::Yesterday();

	// �Ԃ̈ʒu�̐ݒ�.
	m_IsDisp					= true;
	m_FlowerSaveData.IsWither	= false;
	m_Transform.Position		= Pos;
	m_Transform.Position.y		= static_cast<float>( Rect.top ) + 5.0f;
	m_FlowerState.Transform		= m_Transform;
	m_LeafState.Transform		= m_Transform;

	// �Ԃ̎�ނ̐ݒ�.
	m_FlowerState.AnimState.PatternNo.y = Type;
	m_LeafState.AnimState.PatternNo.y	= Type;
	m_FlowerState.Color					= Color4::RGBA( Color );
}

//---------------------------.
// �����蔻��̏�����.
//---------------------------.
void CFlower::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pFlower, &m_LeafState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 20.0f );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CFlower::UpdateCollision()
{
}

//---------------------------.
// �}�E�X�ɐG��Ă��邩.
//---------------------------.
bool CFlower::GetIsTouchMouse()
{
	// �ʒu�̎擾.
	D3DXPOSITION3*		 Pos	  = &m_LeafState.Transform.Position;
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// �}�E�X���摜�̏�ɂ��邩�ǂ���.
	const D3DXPOSITION3& CPos = *Pos + m_AddCenterPosition;
	if ( CPos.x - m_FlowerSize / 2.0f <= MousePos.x && MousePos.x <= CPos.x + m_FlowerSize / 2.0f &&
		 CPos.y - m_FlowerSize / 2.0f <= MousePos.y && MousePos.y <= CPos.y + m_FlowerSize / 2.0f )
	{
		return true;
	}
	return false;
}

//---------------------------.
// �Ԃɉ����������̃��A�N�V�����̐ݒ�.
//---------------------------.
void CFlower::Reaction()
{
	if ( m_IsReactionAnim ) return;

	m_IsReactionAnim	= true;
	m_ReactionAnimCnt	= Math::DEGREE_MIN;
}

//---------------------------.
// �͂ꂳ����.
//---------------------------.
void CFlower::Wither()
{
	if ( m_FlowerSaveData.IsWither ) return;

	m_FlowerSaveData.IsWither			= true;
	m_FlowerSaveData.WitherDay			= TimeManager::GetTime();
	m_FlowerState.AnimState.PatternNo.x = SPRITE_WITHER_FLOWER_NO;
	m_LeafState.AnimState.PatternNo.x	= SPRITE_WITHER_LEAF_NO;
}

//---------------------------.
// �Ԃ��N���b�N�������̍X�V
//---------------------------.
void CFlower::FlowerClickUpdate()
{
	if ( GetIsTouchMouse() == false || Input::GetIsLeftClickDown() == false ) return;

	Reaction();
}

//---------------------------.
// �Ԃ̃A�j���[�V�����̍X�V
//---------------------------.
void CFlower::FlowerAnimUpdate()
{
	if ( m_FlowerSaveData.IsWither ) return;

	m_FlowerState.Transform.Position.x = m_LeafState.Transform.Position.x +
		std::sinf( Math::ToRadian( m_FlowerAnimCnt ) );
	m_FlowerState.Transform.Position.y = m_LeafState.Transform.Position.y +
		std::cosf( Math::ToRadian( m_FlowerAnimCnt * 2.0f ) );
}

//---------------------------.
// �}�E�X�ɂ��Ԃ̃A�j���[�V�����̍X�V.
//---------------------------.
void CFlower::MouseTouchReactionUpdate()
{
	if ( m_IsReactionAnim ) return;

	// �}�E�X���摜�̏�ɂ��邩�ǂ���.
	if ( GetIsTouchMouse() == false ) return;

	// �A�j���[�V�������s��Ȃ���.
	const D3DXPOSITION3& MousePos	 = Input::GetMousePosition3();
	const D3DXPOSITION3& OldMousePos = Input::GetMouseOldPosition3();
	if ( std::abs( MousePos.x - OldMousePos.x ) <= Const::Flower.SKIP_SHAKE_SPEED ) return;

	Reaction();
}

//---------------------------.
// �ԂɐG�ꂽ���̃A�j���[�V�����̍X�V.
//---------------------------.
void CFlower::ReactionAnimUpdate()
{
	if ( m_IsReactionAnim == false ) return;

	m_ReactionAnimCnt += Math::DEGREE_MAX * Const::Flower.REACTION_ANIM_SPEED * m_DeltaTime;
	if ( m_ReactionAnimCnt >= Math::DEGREE_MAX ) {
		m_ReactionAnimCnt	= Math::DEGREE_MIN;
		m_IsReactionAnim	= false;
	}

	m_FlowerState.Transform.Scale.x = STransform::NORMAL_SCALE +
		std::sinf( Math::ToRadian( m_ReactionAnimCnt ) ) * Const::Flower.REACTION_ANIM_SCALE;
	m_FlowerState.Transform.Scale.y = STransform::NORMAL_SCALE +
		std::sinf( Math::ToRadian( m_ReactionAnimCnt ) ) * Const::Flower.REACTION_ANIM_SCALE;
}

//---------------------------.
// ����芮���A�j���[�V�����̑ҋ@���Ԃ̍X�V.
//---------------------------.
void CFlower::WateringAnimCoolTimeUpdate()
{
	if ( m_WateringAnimCnt <= 0.0f ) return;

	m_WateringAnimCnt -= m_DeltaTime;
	if ( m_WateringAnimCnt <= 0.0f ) {
		m_WateringAnimCnt = 0.0f;
	}
}

//---------------------------.
// ����芮���A�j���[�V�����̍X�V.
//---------------------------.
void CFlower::WateringAnimUpdate()
{
	if ( m_FlowerSaveData.IsWatering == false ) return;
	if ( m_WateringAnimCnt > 0.0f			  ) return;

	auto pos  = m_LeafState.Transform.Position + m_AddCenterPosition;
	auto fsize = m_FlowerSize / 2.0f;
	pos.x += Random::GetRand( -fsize, fsize );
	pos.y += Random::GetRand( -fsize, fsize );

	m_pWateringAnim->Play( pos );

	m_WateringAnimCnt = Random::GetRand(
		Const::Flower.WATERING_ANIM_COOL_TIME_MIN,
		Const::Flower.WATERING_ANIM_COOL_TIME_MAX
	);
}

//---------------------------.
// �v���C���[�ƃ{�[���̓����蔻��
//---------------------------.
void CFlower::PlayerBallCollision( CActor* pActor, const EObjectTag Tag )
{
	if ( Coll2D::IsSphere2DToSphere2D(
		m_pCollisions->GetCollision<CSphere2D>(),
		pActor->GetCollision<CSphere2D>() ) == false ) return;

	const D3DXPOSITION3& Pos	= pActor->GetPosition();
	const D3DXPOSITION3& OldPos = pActor->GetTransform().OldPosition;
	if ( std::abs( Pos.x - OldPos.x ) <= Const::Flower.SKIP_SHAKE_SPEED ) return;

	Reaction();
}

//---------------------------.
// ���̓����蔻��
//---------------------------.
void CFlower::WaterCollision( CActor* pActor, const EObjectTag Tag )
{
	if ( m_FlowerSaveData.IsWatering ) return;

	auto* pWinObj = dynamic_cast<CWindowObject*>( pActor );
	if ( pWinObj->GetIsInWindow() ) return;
	if ( Coll2D::IsBox2DToSphere2D(
		pWinObj->GetCollision<CBox2D>(),
		m_pCollisions->GetCollision<CSphere2D>() ) == false ) return;

	m_WateringCnt++;
	if ( m_WateringCnt >= Const::Flower.WATER_OK_NUM ) {
		m_FlowerSaveData.IsWatering  = true;
		m_FlowerSaveData.WateringDay = TimeManager::GetTime();
	}
}
