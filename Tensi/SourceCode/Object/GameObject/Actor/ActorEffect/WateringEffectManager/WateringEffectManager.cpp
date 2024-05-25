#include "WateringEffectManager.h"
#include "WateringEffect/WateringEffect.h"

CWateringEffectManager::CWateringEffectManager()
	: m_pWateringList	()
{
}

CWateringEffectManager::~CWateringEffectManager()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWateringEffectManager::Init()
{
	return false;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWateringEffectManager::Update( const float& DeltaTime )
{
	for ( auto& w : m_pWateringList ) w->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CWateringEffectManager::Render()
{
	for ( auto& w : m_pWateringList ) w->Render();
}

//---------------------------.
// �Đ�.
//---------------------------.
void CWateringEffectManager::Play( const D3DXPOSITION3& Pos )
{
	// �g�p���Ă��Ȃ��̂�����Ύg�p����.
	for ( auto& w : m_pWateringList ) {
		if ( w->GetIsAnimPlay() ) continue;

		w->Init();
		w->Play( Pos );
		return;
	}

	// �V�������
	m_pWateringList.emplace_back( std::make_unique<CWateringEffect>() );
	m_pWateringList.back()->Init();
	m_pWateringList.back()->Play( Pos );
}

//---------------------------.
// �����X�g�̍폜.
//---------------------------.
void CWateringEffectManager::WateringListDelete()
{
	for ( int i = static_cast<int>( m_pWateringList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWateringList[i]->GetIsAnimPlay() ) break;
		m_pWateringList.pop_back();
	}
}
