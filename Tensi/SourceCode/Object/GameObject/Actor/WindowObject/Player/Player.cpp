#include "Player.h"
#include "..\..\SleepEffectManager\SleepEffectManager.h"
#include "..\..\NoteEffectManager\NoteEffectManager.h"
#include "..\..\..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\..\..\Common\SoundManeger\SoundManeger.h"
#include "..\..\..\..\..\Scene\SceneManager\SceneManager.h"
#include "..\..\..\..\..\Utility\Math\Math.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\Input\Input.h"
#include "..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"

namespace {
	const D3DXVECTOR3	LEFT_MOVE_VECTOR	= { -10.0f, -20.0f, 0.0f };	// ���Ɉړ�����x�N�g��.
	const D3DXVECTOR3	RIGHT_MOVE_VECTOR	= { 10.0f,  -20.0f, 0.0f };	// �E�Ɉړ�����x�N�g��.
	const D3DXVECTOR3	JUMP_VECTOR			= { 0.0f,   -20.0f, 0.0f };	// �W�����v�x�N�g��.
	const float			PLAYER_SIZE			= 80.0f;					// �摜�̃T�C�Y.
	const float			CEILING_POS			= -PLAYER_SIZE * 5.0f;		// �V��̈ʒu.

	// ������No.
	enum enImageNo : unsigned char {
		Image_Normal,		// �ʏ펞.
		Image_Right,		// �E����.
		Image_Left			// ������.
	} typedef EImageNo;

	// �s��No.
	enum enActionNo : unsigned char {
		ActionWait,						// �ҋ@.
		ActionMove,						// �����_���ȏꏊ�Ɉړ�.
		ActionJump,						// �W�����v.
		ActionPlayWithBall,				// �{�[���ŗV��.

		Random_Action_Max,
		ActionSleep = Random_Action_Max,// �Q��.
		ActionMusic,					// �������Ă�Ƃ��̍s��.

		Action_Max
	} typedef EActionNo;
}

CPlayer::CPlayer()
	: m_pCollapsed		( nullptr )
	, m_CollapsedState	()
	, m_pSleepEffect	( nullptr )
	, m_pNoteEffect		( nullptr )
	, m_ActionTime		( INIT_FLOAT )
	, m_MoveTimeCnt		( INIT_FLOAT )
	, m_CoolTime		( INIT_FLOAT )
	, m_Action			( INIT_INT )
	, m_IsLandingWait	( false )
	, m_IsOldMoveLeft	( false )
{
}

CPlayer::~CPlayer()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CPlayer::Init()
{
	// �摜�̎擾.
	m_pSprite		= SpriteResource::GetSprite( "Slime", &m_SpriteState );
	m_pCollapsed	= SpriteResource::GetSprite( "CollapsedSlime", &m_CollapsedState );

	// ����G�t�F�N�g�̏�����.
	m_pSleepEffect	= std::make_unique<CSleepEffectManager>();
	m_pSleepEffect->Init();

	// �����G�t�F�N�g�̏�����.
	m_pNoteEffect	= std::make_unique<CNoteEffectManager>();
	m_pNoteEffect->Init();

	// �^�X�N�o�[�̏����擾.
	const RECT& TaskBar = WindowManager::GetTaskBarRect();

	// �摜�̐ݒ�.
	m_SpriteState.Transform.Position.x		= 256.0f;
	m_SpriteState.Transform.Position.y		= static_cast<float>( TaskBar.top );
	m_SpriteState.Color.w					= 0.85f;
	m_CollapsedState.Color.w				= 0.85f;
	m_SpriteState.AnimState.FrameTime		= 50;
	m_SpriteState.AnimState.IsAnimationY	= true;
	m_SpriteState.AnimState.PatternNo.x 	= Image_Normal;

	// �I�u�W�F�N�g�̏�����.
	m_CollSize			= PLAYER_SIZE;
	m_SpeedRate			= 0.95f;
	m_CeilingPosY		= CEILING_POS;
	m_ObjectTag			= EObjectTag::Player;

	SaveDataManager::SetSaveData()->PlayerTransform = &m_SpriteState.Transform;

	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CPlayer::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false	) return;

	ActionUpdate();		// �s���̍X�V.
	EffectUpdate();		// �G�t�F�N�g�̍X�V.

	// �E�B���h�E�I�u�W�F�N�g�̍X�V.
	WindowObjectUpdate();

	int a_b = 0;
	a_b = 10;

	// �`��G���A�����낦��.
	m_CollapsedState.RenderArea		= m_SpriteState.RenderArea;
	m_CollapsedState.RenderArea.w	= -1.0f;

	TransformUpdate( m_SpriteState.Transform );
	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::Both );
}

//---------------------------.
// �����蔻��I����Ăяo�����X�V.
//---------------------------.
void CPlayer::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// �`��.
//---------------------------.
void CPlayer::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSleepEffect->Render();
	m_pNoteEffect->Render();
	if ( m_IsLandingWait )	m_pCollapsed->RenderUI( &m_CollapsedState );
	else					m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CPlayer::Collision( CActor* pActor )
{
}

//---------------------------.
// �X���C�����Ԃ�.
//---------------------------.
void CPlayer::Collapsed()
{
	// ���n�d���̐ݒ�.
	m_IsGrab		= false;
	m_IsLandingWait = true;
	m_CoolTime		= 4.0f;

	// �Ԃꂽ�摜�̈ʒu�̐ݒ�.
	m_CollapsedState.Transform.Position = m_SpriteState.Transform.Position;

	// �s���̏I��.
	ActionEnd();
}

//---------------------------.
// �����蔻��̏�����.
//---------------------------.
void CPlayer::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 40.0f );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CPlayer::UpdateCollision()
{
}

//---------------------------.
// �͂߂邩�̊m�F.
//---------------------------.
bool CPlayer::GrabCheck()
{
	if ( m_Action == ActionSleep	) return false;
	if ( m_IsLandingWait			) return false;
	return true;
}

//---------------------------.
// �͂񂾎��̏�����.
//---------------------------.
void CPlayer::GrabInit()
{
	// �s����ҋ@�ɕύX����.
	ActionEnd();
	m_Action		= ActionWait;
	m_ActionTime	= 2.0f;
}

//---------------------------.
// �ړ����̍X�V.
//---------------------------.
void CPlayer::MoveUpdate()
{
	// �����ڂ̕ύX.
	if ( m_IsGrab ) {
		m_SpriteState.AnimState.PatternNo.x = Image_Normal;
	}
	else {
		if (		m_MoveVector.x > 0.0f ) m_SpriteState.AnimState.PatternNo.x = Image_Right;
		else if (	m_MoveVector.x < 0.0f ) m_SpriteState.AnimState.PatternNo.x = Image_Left;
		else								m_SpriteState.AnimState.PatternNo.x = Image_Normal;
	}
}

//---------------------------.
// ��ɓ����������̍X�V.
//---------------------------.
void CPlayer::HitUpUpdate()
{
	// �ړ��̗͂𖳂���.
	if ( m_MoveVector.y < 0.0f ) m_MoveVector.y = 0.0f;
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CPlayer::HitDownUpdate()
{
	// �ړ��̗͂𖳂���.
	if ( m_MoveVector.y > 0.0f ) m_MoveVector.y = 0.0f;

	// �������̗͂����炷.
	m_MoveVector.x *= 0.8f;
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CPlayer::HitLeftUpdate()
{
	// �ړ��̗͂𖳂���.
	if ( m_MoveVector.x > 0.0f ) m_MoveVector.x = 0.0f;

	// ������̗͂����炷.
	if ( m_MoveVector.y < 0.0f ) m_MoveVector.y *= 0.8f;
}

//---------------------------.
// �E�ɓ����������̍X�V.
//---------------------------.
void CPlayer::HitRightUpdate()
{
	// �ړ��̗͂𖳂���.
	if ( m_MoveVector.x > 0.0f ) m_MoveVector.x = 0.0f;

	// ������̗͂����炷.
	if ( m_MoveVector.y < 0.0f ) m_MoveVector.y *= 0.8f;
}

//---------------------------.
// ���n���̏�����.
//---------------------------.
void CPlayer::LandingInit()
{
	// ���n��ɍd�������邩.
	if ( m_IsLandingWait ) return;
	const float Fall	= m_SpriteState.Transform.Position.y - m_StartPosition.y;
	m_StartPosition		= INIT_FLOAT3;
	if ( Fall <= 400.0f ) return;

	// ���n�d���̐ݒ�.
	float Power = Fall / ( m_Wnd_h / 2.0f );
	m_IsGrab		= false;
	m_IsLandingWait = true;
	m_CoolTime		= 4.0f * Power;

	// �Ԃꂽ�摜�̈ʒu�̐ݒ�.
	m_CollapsedState.Transform.Position = m_SpriteState.Transform.Position;

	// �s���̏I��.
	ActionEnd();
}

//---------------------------.
// ���n�̍X�V.
//---------------------------.
void CPlayer::LandingUpdate()
{
	if ( m_IsLandingWait == false ) return;

	// �Ԃꂽ�摜�̈ʒu�̐ݒ�.
	m_CollapsedState.Transform.Position = m_SpriteState.Transform.Position;

	// �N���b�N���ꂽ��.
	if ( m_pCollapsed->GetIsOverlappingTheMouse( &m_CollapsedState ) &&
		 KeyInput::IsKeyDown( VK_LBUTTON ) )
	{
		// �҂����Ԃ��������炷.
		m_CoolTime -= 0.5f;
	}

	// �҂����Ԃ����炷.
	m_CoolTime -= m_DeltaTime;

	// ���n�̍d������������.
	if ( m_CoolTime < 0.0f ) {
		m_IsLandingWait = false;
		m_CoolTime		= INIT_FLOAT;

		// �W�����v�A�N�V�����ɕύX.
		m_Action		= ActionJump;
		m_ActionTime	= 3.0f;
	}
}

//---------------------------.
// �G�t�F�N�g�̍X�V.
//---------------------------.
void CPlayer::EffectUpdate()
{
	// ���W�̍X�V.
	m_pSleepEffect->SetPosition( m_SpriteState.Transform.Position );
	m_pNoteEffect->SetPosition( m_SpriteState.Transform.Position );

	// �G�t�F�N�g�̍X�V.
	m_pSleepEffect->Update( m_DeltaTime );
	m_pNoteEffect->Update( m_DeltaTime );
}

//---------------------------.
// �s���̏I�����̏�����.
//---------------------------.
void CPlayer::ActionEnd()
{
	switch ( m_Action ) {
	case ActionWait:
		// ������.
		m_EndPosition = INIT_FLOAT3;
		m_MoveTimeCnt = INIT_FLOAT;
		break;
	case ActionMove:
		break;
	case ActionJump:
		break;
	case ActionSleep:
		// ������.
		m_SpriteState.AnimState.FrameTime = 50;

		// �G�t�F�N�g�̒�~.
		m_pSleepEffect->Stop();
		break;
	case ActionPlayWithBall:
		// ������.
		m_EndPosition = INIT_FLOAT3;
		m_MoveTimeCnt = INIT_FLOAT;
		break;
	case ActionMusic:
		// ������.
		m_EndPosition = INIT_FLOAT3;
		m_MoveTimeCnt = INIT_FLOAT;

		// �G�t�F�N�g�̒�~.
		m_pNoteEffect->Stop();
		break;
	default:
		break;
	}
}

//---------------------------.
// �s���̊J�n�̏�����.
//---------------------------.
void CPlayer::ActionStart()
{
	switch ( m_Action ) {
	case ActionWait:
		break;
	case ActionMove:
		break;
	case ActionJump:
		break;
	case ActionSleep:
		// �G�t�F�N�g�̊J�n.
		m_pSleepEffect->Play();

		// �s���̍s�����Ԃ𒷂�����.
		m_ActionTime *= Random::GetRand( 10.0f, 30.0f );
		m_SpriteState.AnimState.FrameTime = 100;
		break;
	case ActionPlayWithBall:
		// �s���̍s�����Ԃ𒷂�����.
		m_ActionTime *= Random::GetRand( 3.0f, 20.0f );
		break;
	case ActionMusic:
		// �G�t�F�N�g�̊J�n.
		m_pNoteEffect->Play( 128.0f );
		m_EndPosition = m_SpriteState.Transform.Position;

		// �s���̍s�����Ԃ𒷂�����.
		m_ActionTime *= Random::GetRand( 10.0f, 60.0f );
		break;
	default:
		break;
	}
}

//---------------------------.
// �s���̍X�V.
//---------------------------.
void CPlayer::ActionUpdate()
{
	if ( m_IsInWndSmall ) return;

	// ���n�̍d���̍X�V.
	LandingUpdate();
	if ( m_IsLandingWait ) return;

	// �s���̍X�V.
	switch ( m_Action ) {
	case ActionWait:			ActionWaitUpdate();			break;
	case ActionMove:			ActionMoveUpdate();			break;
	case ActionJump:			ActionJumpUpdate();			break;
	case ActionSleep:			ActionSleepUpdate();		break;
	case ActionPlayWithBall:	ActionPlayWithBallUpdate();	break;
	case ActionMusic:			ActionMusicUpdate();		break;
	default:												break;
	}

	// �s���̎��Ԃ����炷.
	if ( m_IsGrab ) return;
	m_ActionTime -= m_DeltaTime;
	if ( m_ActionTime <= 0.0f ) {
		// �s���̕ύX.
		ActionEnd();
		m_Action		= Random::GetRand( 0, Random_Action_Max - 1 );
		m_ActionTime	= Random::GetRand( 3.0f, 10.0f );
		if ( Random::Probability( 1, 20 )			) m_Action = ActionSleep;
		if ( SoundManager::GetPeakValue() > 0.0f	) {
			if ( Random::Probability( 1, 5 )		) m_Action = ActionMusic;
		}
		ActionStart();
	}
}

//---------------------------.
// �ҋ@.
//---------------------------.
void CPlayer::ActionWaitUpdate()
{
}

//---------------------------.
// �ړ��̍s���̍X�V.
//---------------------------.
void CPlayer::ActionMoveUpdate()
{
	// �ړ����Ԃ����Z.
	m_MoveTimeCnt += m_DeltaTime;

	// �����Ă���ꍇ�͏����͍s��Ȃ�.
	if ( m_IsLanding == false ) return;

	// �N�[���^�C���̌���.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// �ړI�n��ݒ�.
	if ( m_EndPosition == D3DXPOSITION3( INIT_FLOAT3 ) || m_EndPosition.x < m_LandingWnd.left || m_EndPosition.x > m_LandingWnd.right ) {
		m_EndPosition.x = Random::GetRand( m_LandingWnd.left + PLAYER_SIZE * 2.0f, m_LandingWnd.right - PLAYER_SIZE * 2.0f );
		m_IsOldMoveLeft	= m_EndPosition.x < m_SpriteState.Transform.Position.x;
	}
	
	// �ړ��x�N�g���̒ǉ�.
	const bool	IsLeftMove = m_EndPosition.x < m_SpriteState.Transform.Position.x;
	D3DXVECTOR3 MoveVecotr = IsLeftMove ? LEFT_MOVE_VECTOR : RIGHT_MOVE_VECTOR;
	m_VectorList.emplace_back( MoveVecotr );

	// �ړI�n�ɓ���������.
	if ( m_IsOldMoveLeft != IsLeftMove || m_MoveTimeCnt > 10.0f ) {
		m_EndPosition	= INIT_FLOAT3;
		m_MoveTimeCnt	= INIT_FLOAT;
	}
	m_IsOldMoveLeft = IsLeftMove;

	// �N�[���^�C���̐ݒ�.
	m_CoolTime = Random::GetRand( 0.1f, 1.0f );
}

//---------------------------.
// �{�[���ŗV�ԍs���̍X�V.
//---------------------------.
void CPlayer::ActionPlayWithBallUpdate()
{
	// �����Ă���ꍇ�͏����͍s��Ȃ�.
	if ( m_IsLanding == false ) return;

	// �N�[���^�C���̌���.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// �ړI�n�̍X�V.
	m_EndPosition = WindowObjectManager::GetTransform( EObjectTag::Ball, 0 )->Position;
	
	// �ړ��x�N�g���̒ǉ�.
	const bool	IsLeftMove = m_EndPosition.x < m_SpriteState.Transform.Position.x;
	D3DXVECTOR3 MoveVecotr = IsLeftMove ? LEFT_MOVE_VECTOR : RIGHT_MOVE_VECTOR;
	m_VectorList.emplace_back( MoveVecotr );

	// �N�[���^�C���̐ݒ�.
	m_CoolTime = Random::GetRand( 0.05f, 0.25f );
}

//---------------------------.
// �W�����v�̍s���̍X�V.
//---------------------------.
void CPlayer::ActionJumpUpdate()
{
	// �����Ă���ꍇ�͏����͍s��Ȃ�.
	if ( m_IsLanding == false ) return;

	// �N�[���^�C���̌���.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// �x�N�g���̒ǉ�.
	m_VectorList.emplace_back( JUMP_VECTOR );

	// �N�[���^�C���̐ݒ�.
	m_CoolTime = Random::GetRand( 0.1f, 1.0f );
}

//---------------------------.
// �Q��̍s���̍X�V.
//---------------------------.
void CPlayer::ActionSleepUpdate()
{
	// ����G�t�F�N�g�̃|�[�Y�̐ݒ�.
	m_pSleepEffect->Pause( !m_SpriteState.IsDisp );

	// �N���b�N���ꂽ��.
	if ( m_pSprite->GetIsOverlappingTheMouse( &m_SpriteState ) &&
		 KeyInput::IsKeyDown( VK_LBUTTON ) )
	{
		// �s���̎��Ԃ����炷.
		m_ActionTime *= 0.5f;
	}
}

//---------------------------.
// �������Ă�Ƃ��̍s���̍X�V.
//---------------------------.
void CPlayer::ActionMusicUpdate()
{
	// �����Ă���ꍇ�͏����͍s��Ȃ�.
	if ( m_IsLanding == false ) return;

	// ����G�t�F�N�g�̃|�[�Y�̐ݒ�.
	m_pNoteEffect->Pause( !m_SpriteState.IsDisp );

	// �����~�܂����ꍇ�s�����I������.
	if ( SoundManager::GetPeakValue() == 0.0f ) {
		m_ActionTime = 0.0f;
	}

	// �N�[���^�C���̌���.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;
	
	// �ړ��x�N�g���̒ǉ�.
	const bool	IsLeftMove = m_EndPosition.x < m_SpriteState.Transform.Position.x;
	D3DXVECTOR3 MoveVecotr = IsLeftMove ? LEFT_MOVE_VECTOR : RIGHT_MOVE_VECTOR;
	MoveVecotr.y /= 2.0f;
	m_VectorList.emplace_back( MoveVecotr );

	// �N�[���^�C���̐ݒ�.
	m_CoolTime = Random::GetRand( 0.05f, 0.25f );
}
