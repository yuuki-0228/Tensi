#include "NoteEffectManager.h"
#include "NoteEffect\NoteEffect.h"

namespace {
	constexpr float COOL_TIME = 0.3f;	// �N�[���^�C��.
}

CNoteEffectManager::CNoteEffectManager()
	: m_pNoteEffectList	()
	, m_CoolTime		( INIT_FLOAT )
	, m_EffectSize		( INIT_FLOAT )
	, m_IsAnimation		( false )
	, m_IsPause			( false )
{
}

CNoteEffectManager::~CNoteEffectManager()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CNoteEffectManager::Init()
{
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CNoteEffectManager::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;

	NoteEffectPlay();		// �����G�t�F�N�g�̍Đ�.
	NoteEffectListDelete();	// �����G�t�F�N�g���X�g�̍폜.

	for ( auto& n : m_pNoteEffectList ) n->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CNoteEffectManager::Render()
{
	for ( auto& n : m_pNoteEffectList ) n->Render();
}

//---------------------------.
// �A�j���[�V�����̊J�n.
//---------------------------.
void CNoteEffectManager::Play( const float Size )
{
	// ����G�t�F�N�g�A�j���[�V�����̊J�n.
	m_IsAnimation	= true;
	m_EffectSize	= Size;
}

//---------------------------.
// �ꎞ��~.
//---------------------------.
void CNoteEffectManager::Pause( const bool Flag )
{
	// �ꎞ��~�̐ݒ�.
	m_IsPause = Flag;
}

//---------------------------.
// �A�j���[�V�����̒�~.
//---------------------------.
void CNoteEffectManager::Stop()
{
	// ����G�t�F�N�g�A�j���[�V�����̒�~.
	m_IsAnimation = false;
}

//---------------------------.
// �����G�t�F�N�g�̍Đ�.
//---------------------------.
void CNoteEffectManager::NoteEffectPlay()
{
	if ( m_IsPause				) return;
	if ( m_IsAnimation == false ) return;

	// �N�[���^�C�������炷.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// �N�[���^�C���̐ݒ�.
	m_CoolTime = COOL_TIME;

	// �g�p���Ă��Ȃ��̂�����Ύg�p����.
	for ( auto& n : m_pNoteEffectList ) {
		if ( n->GetIsAnimPlay() ) continue;

		// �A�j���[�V�������J�n������.
		n->Play( m_Transform.Position, m_EffectSize );
		return;
	}

	// �V�����쐬.
	m_pNoteEffectList.emplace_back( std::make_unique<CNoteEffect>() );

	// �A�j���[�V�������J�n������.
	m_pNoteEffectList.back()->Init();
	m_pNoteEffectList.back()->Play( m_Transform.Position, m_EffectSize );
}

//---------------------------.
// �����G�t�F�N�g���X�g�̍폜.
//---------------------------.
void CNoteEffectManager::NoteEffectListDelete()
{
	for ( int i = static_cast< int >( m_pNoteEffectList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pNoteEffectList[i]->GetIsAnimPlay() ) break;
		m_pNoteEffectList.pop_back();
	}
}
