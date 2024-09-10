#include "WeedManager.h"
#include "Weed/Weed.h"
#include "..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"

CWeedManager::CWeedManager()
	: m_pWeedList	()
	, m_WeedSize	( INIT_FLOAT )
{
}

CWeedManager::~CWeedManager()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWeedManager::Init()
{
	const auto* pWeed = SpriteResource::GetSprite( "Weed" );
	m_WeedSize = pWeed->GetSpriteState().Disp.w;

	SaveDataManager::SetWeedSaveData( [this]() { return Save(); }, [this]( const auto& Data ) { Load( Data ); } );

	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWeedManager::Update( const float& DeltaTime )
{
	WeedListDelete();	// �G�����X�g�̍폜.

	for ( auto& w : m_pWeedList ) w->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CWeedManager::SubRender()
{
	for ( auto& w : m_pWeedList ) w->SubRender();
}

//---------------------------.
// �G���𖄂߂�.
//---------------------------.
void CWeedManager::Fill( const int Num )
{
	const RECT& Size  = WindowManager::GetMyWndSize();
	const float Wnd_W = static_cast< float >( Size.right - Size.left );

	for ( int i = 0; i < Num; ++i ) {
		// ���߂�ꏊ�������_���Ō��߂�.
		D3DXPOSITION3 Pos = INIT_FLOAT3;
		Pos.x = Random::GetRand( 0.0f, Wnd_W );

		// �g�p���Ă��Ȃ��̂�����Ύg�p����.
		for ( auto& s : m_pWeedList ) {
			if ( s->GetIsDisp() ) continue;

			s->Init();
			s->Fill( Pos );
			return;
		}

		// �V�������
		m_pWeedList.emplace_back( std::make_unique<CWeed>() );
		m_pWeedList.back()->Init();
		m_pWeedList.back()->Fill( Pos );
	}
}

//---------------------------.
// �\�����Ă���{���̎擾
//---------------------------.
int CWeedManager::GetNum()
{
	return static_cast<int>( std::count_if(
		m_pWeedList.begin(),
		m_pWeedList.end(),
		[]( const auto& w ) { return w->GetIsDisp(); }
	) );
}

//---------------------------.
// �ۑ�.
//---------------------------.
std::vector<SWeedData> CWeedManager::Save()
{
	const int size = static_cast<int>( m_pWeedList.size() );

	// �G���̃Z�[�u�f�[�^�̎擾.
	std::vector<SWeedData> Data;
	Data.resize( size );
	for ( int i = 0; i < size; ++i ) {
		Data[i] = m_pWeedList[i]->GetWeedData();
	}

	return Data;
}

//---------------------------.
// �ǂݍ���.
//---------------------------.
void CWeedManager::Load( std::vector<SWeedData> Data )
{
	const int size = static_cast<int>( Data.size() );

	m_pWeedList.resize( size );
	for ( int i = 0; i < size; ++i ) {
		m_pWeedList[i] = std::make_unique<CWeed>();
		m_pWeedList[i]->Init();
		m_pWeedList[i]->SetWeedData( Data[i] );
	}

	// �z����̎g�p���Ă��Ȃ��v�f���폜����
	m_pWeedList.erase( 
		std::remove_if(
			std::begin( m_pWeedList ),
			std::end( m_pWeedList ),
			[]( const auto& w ) { return w->GetIsDisp() == false; } ),
		std::cend( m_pWeedList )
	);
}

//---------------------------.
// �G�����X�g�̍폜.
//---------------------------.
void CWeedManager::WeedListDelete()
{
	for ( int i = static_cast<int>( m_pWeedList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWeedList[i]->GetIsDisp() ) break;
		m_pWeedList.pop_back();
	}
}
