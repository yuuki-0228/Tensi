#include "WateringAnimManager.h"
#include "WateringAnim/WateringAnim.h"

CWateringAnimManager::CWateringAnimManager()
	: m_pWateringList	()
{
}

CWateringAnimManager::~CWateringAnimManager()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWateringAnimManager::Init()
{
	return false;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWateringAnimManager::Update( const float& DeltaTime )
{
	for ( auto& w : m_pWateringList ) w->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CWateringAnimManager::Render()
{
	for ( auto& w : m_pWateringList ) w->Render();
}

//---------------------------.
// �Đ�.
//---------------------------.
void CWateringAnimManager::Play( const D3DXPOSITION3& Pos )
{
	// �g�p���Ă��Ȃ��̂�����Ύg�p����.
	for ( auto& w : m_pWateringList ) {
		if ( w->GetIsDisp() ) continue;

		w->Init();
		w->Play( Pos );
		return;
	}

	// �V�������
	m_pWateringList.emplace_back( std::make_unique<CWateringAnim>() );
	m_pWateringList.back()->Init();
	m_pWateringList.back()->Play( Pos );
}

//---------------------------.
// �����X�g�̍폜.
//---------------------------.
void CWateringAnimManager::WateringListDelete()
{
	for ( int i = static_cast<int>( m_pWateringList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWateringList[i]->GetIsDisp() ) break;
		m_pWateringList.pop_back();
	}
}
