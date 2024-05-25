#include "WaterManager.h"
#include "Water/Water.h"
#include "..\..\..\..\..\..\Utility\Random\Random.h"

CWaterManager::CWaterManager()
	: m_pWaterList	()
{
}

CWaterManager::~CWaterManager()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWaterManager::Init()
{
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWaterManager::Update( const float& DeltaTime )
{
	WaterListDelete();	// �����X�g�̍폜.

	for ( auto& w : m_pWaterList ) w->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CWaterManager::Render()
{
	for ( auto& w : m_pWaterList ) w->Render();
}

//---------------------------.
// ����\������
//---------------------------.
void CWaterManager::Spill( const SWaterData& data, const HWND hWnd )
{
	// �x�N�g���������_���ŉ�]������
	auto moveVec = data.MoveVec;
	auto rot = D3DXROTATION3( 0.0f, 0.0f, Random::GetRand( data.VecRotMin, data.VecRotMax ) );
	Math::VectorRotation( &moveVec, rot );

	// ���x�������_���Ō��߂�
	moveVec *= Random::GetRand( data.PowerMin, data.PowerMax );

	// �g�p���Ă��Ȃ��̂�����Ύg�p����.
	for ( auto& s : m_pWaterList ) {
		if ( s->GetIsDisp() ) continue;

		s->Init();
		s->SetWater( data.Pos, moveVec, hWnd );
		return;
	}

	// �V�������
	m_pWaterList.emplace_back( std::make_unique<CWater>() );
	m_pWaterList.back()->Init();
	m_pWaterList.back()->SetWater( data.Pos, moveVec, hWnd );
}

//---------------------------.
// �����X�g�̍폜.
//---------------------------.
void CWaterManager::WaterListDelete()
{
	for ( int i = static_cast<int>( m_pWaterList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWaterList[i]->GetIsDisp() ) break;
		m_pWaterList.pop_back();
	}
}
