#include "SleepEffectManager.h"
#include "SleepEffect\SleepEffect.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"

namespace {
	constexpr float COOL_TIME = 0.5f;	// �N�[���^�C��.
}

CSleepEffectManager::CSleepEffectManager()
	: m_pSurprisedEffect		( nullptr )
	, m_SurprisedEffectState	()
	, m_pSleepEffectList		()
	, m_CoolTime				( INIT_FLOAT )
	, m_EffectAnimAng			( INIT_FLOAT )
	, m_IsAnimation				( false )
{
}

CSleepEffectManager::~CSleepEffectManager()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CSleepEffectManager::Init()
{
	// �摜�̎擾.
	m_pSurprisedEffect = SpriteResource::GetSprite( "SurprisedEffect", &m_SurprisedEffectState );
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CSleepEffectManager::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;

	SurprisedEffect();			// �т�����G�t�F�N�g�̍Đ�.
	SleepEffectPlay();			// ����G�t�F�N�g�̍Đ�.
	SleepEffectListDelete();	// ����G�t�F�N�g���X�g�̍폜.

	for ( auto& e : m_pSleepEffectList ) e->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CSleepEffectManager::Render()
{
	for ( auto& e : m_pSleepEffectList ) e->Render();
	m_pSurprisedEffect->RenderUI( &m_SurprisedEffectState );
}

//---------------------------.
// �A�j���[�V�����̊J�n.
//---------------------------.
void CSleepEffectManager::Play()
{
	// ����G�t�F�N�g�A�j���[�V�����̊J�n.
	m_IsAnimation = true;
}

//---------------------------.
// �ꎞ��~.
//---------------------------.
void CSleepEffectManager::Pause( const bool Flag )
{
	m_IsPause = Flag;
}

//---------------------------.
// �A�j���[�V�����̒�~.
//---------------------------.
void CSleepEffectManager::Stop()
{
	// ����G�t�F�N�g�A�j���[�V�����̒�~.
	m_IsAnimation = false;

	// �т�����G�t�F�N�g�̏�����.
	if ( m_IsPause ) return;
	m_EffectAnimAng								= Math::ToRadian( 45.0f );
	m_SurprisedEffectState.Color.w				= Color::ALPHA_MAX;
	m_SurprisedEffectState.IsDisp				= true;
	m_SurprisedEffectState.Transform.Position	= m_Transform.Position;
	m_SurprisedEffectState.Transform.Position.y -= 128.0f;
}

//---------------------------.
// �т�����G�t�F�N�g�̍Đ�.
//---------------------------.
void CSleepEffectManager::SurprisedEffect()
{
	if ( m_SurprisedEffectState.IsDisp == false ) return;

	// �A�j���[�V�����p�̊p�x�̉��Z.
	if ( m_EffectAnimAng < Math::RADIAN_MID ) {
		m_EffectAnimAng += Math::RADIAN_MAX * m_DeltaTime;
		if ( m_EffectAnimAng >= Math::RADIAN_MID ) {
			m_EffectAnimAng = Math::RADIAN_MID;
		}

		// �g�k�̐ݒ�.
		m_SurprisedEffectState.Transform.Scale.y = 1.0f - sinf( m_EffectAnimAng );
	}

	// �A���t�@�l�̐ݒ�.
	m_SurprisedEffectState.Color.w -= Color::ALPHA_MAX * m_DeltaTime;

	// �A�j���[�V�������I��������.
	if ( m_SurprisedEffectState.Color.w < Color::ALPHA_MIN ) {
		m_SurprisedEffectState.IsDisp = false;
	}
}

//---------------------------.
// ����G�t�F�N�g�̍Đ�.
//---------------------------.
void CSleepEffectManager::SleepEffectPlay()
{
	if ( m_IsPause				) return;
	if ( m_IsAnimation == false ) return;

	// �N�[���^�C�������炷.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// �N�[���^�C���̐ݒ�.
	m_CoolTime = COOL_TIME;

	// �g�p���Ă��Ȃ��̂�����Ύg�p����.
	for ( auto& s : m_pSleepEffectList ) {
		if ( s->GetIsAnimPlay() ) continue;

		// �A�j���[�V�������J�n������.
		s->Play( m_Transform.Position );
		return;
	}

	// �V�����쐬.
	m_pSleepEffectList.emplace_back( std::make_unique<CSleepEffect>() );

	// �A�j���[�V�������J�n������.
	m_pSleepEffectList.back()->Init();
	m_pSleepEffectList.back()->Play( m_Transform.Position );
}

//---------------------------.
// ����G�t�F�N�g���X�g�̍폜.
//---------------------------.
void CSleepEffectManager::SleepEffectListDelete()
{
	for ( int i = static_cast<int>( m_pSleepEffectList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pSleepEffectList[i]->GetIsAnimPlay() ) break;
		m_pSleepEffectList.pop_back();
	}
}
