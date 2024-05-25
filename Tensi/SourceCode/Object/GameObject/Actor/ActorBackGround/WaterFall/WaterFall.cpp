#include "WaterFall.h"
#include "WaterFallTile/WaterFallTile.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"
#include "..\..\..\..\..\Utility\Const\Const.h"

CWaterFall::CWaterFall()
	: m_WaterFallTileList	()
	, m_ImageSize			( INIT_FLOAT )
{
}

CWaterFall::~CWaterFall()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWaterFall::Init()
{
	// �摜�̎擾.
	const auto* pWaterFall = SpriteResource::GetSprite( "WaterFall" );
	m_ImageSize	= pWaterFall->GetSpriteState().Disp.w;

	SaveDataManager::SetSaveData()->WaterFallTransform = &m_Transform;
	SaveDataManager::PushLoadFunction( [this]() { CreateWaterFall(); } );
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWaterFall::Update( const float& DeltaTime )
{
	for ( auto& w : m_WaterFallTileList ) w->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CWaterFall::SubRender()
{
	for ( auto& w : m_WaterFallTileList ) w->SubRender();
}

//---------------------------.
// ��̐ݒ�.
//---------------------------.
void CWaterFall::Setting()
{
	// �\������ʒu�̐ݒ�.
	const RECT& Size		= WindowManager::GetMyWndSize();
	const float Wnd_W		= static_cast< float >( Size.right - Size.left );
	m_Transform.Position.x  = Random::GetRand( 0.0f , Wnd_W - m_ImageSize * Const::WaterFall.WATER_FALL_W );
	m_Transform.Position.x += m_ImageSize / 2.0f;

	// �^�X�N�o�[�̃T�C�Y���擾.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// �ݒu�ꏊ�̐ݒ�
	m_Transform.Position.y = static_cast<float>( Rect.top ) + 3.0f;

	// ������.
	CreateWaterFall();
}

//---------------------------.
// ������.
//---------------------------.
void CWaterFall::CreateWaterFall()
{
	// ��̃^�C���̐ݒ�.
	D3DXPOSITION3	SetPos  = m_Transform.Position;
	const float		startX	= SetPos.x;
	bool			isFirst = true;
	while ( SetPos.y > 0.0f )
	{
		for ( int i = 0; i < Const::WaterFall.WATER_FALL_W; ++i ) {
			m_WaterFallTileList.emplace_back( std::make_unique<CWaterFallTile>() );
			m_WaterFallTileList.back()->Init();
			m_WaterFallTileList.back()->Setting( SetPos, isFirst );

			// �z�u����x���W���炷
			SetPos.x += m_ImageSize;
		}

		// ��ԍŏ��̗��\�����I�������.
		if ( isFirst ) isFirst = false;

		// �z�u����x���W�����ɖ߂�
		SetPos.x = startX;

		// �z�u����y���W����ɂ�����
		SetPos.y -= m_ImageSize;
	}
}
