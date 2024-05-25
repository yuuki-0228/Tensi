#include "Tree.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\Utility\Input\Input.h"

namespace {
	enum enTreeNo : unsigned char {
		Sapling,	// �c��.
		SmallTree,	// ��������.
		Tree,		// ��.
	} typedef ETreeNo;
}

CTree::CTree()
	: m_pSapling			( nullptr )
	, m_pSmallTree			( nullptr )
	, m_pSmallTreeLeaf		( nullptr )
	, m_pTree				( nullptr )
	, m_SaplingState		()
	, m_SmallTreeState		()
	, m_SmallTreeLeafState	()
	, m_TreeState			()
	, m_LeafColor			( INIT_FLOAT4 )
	, m_TreeNo				( INIT_INT )
{
}

CTree::~CTree()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CTree::Init()
{
	// �摜�̎擾.
	m_pSapling			= SpriteResource::GetSprite( "Sapling",		&m_SaplingState			);
	m_pSmallTree		= SpriteResource::GetSprite( "SmallTree",		&m_SmallTreeState		);
	m_pSmallTreeLeaf	= SpriteResource::GetSprite( "SmallTreeLeaf",	&m_SmallTreeLeafState	);
	m_pTree				= SpriteResource::GetSprite( "Tree",			&m_TreeState			);
	m_pTreeLeaf			= SpriteResource::GetSprite( "TreeLeaf",		&m_TreeLeafState		);
	m_LeafColor.w		= Color::ALPHA_MAX;
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CTree::Update( const float& DeltaTime )
{
	if ( DebugKeyInput::IsKeyDown( 'Q' ) ) {
		m_TreeNo						= Sapling;
		m_SaplingState.Transform		= m_Transform;
	}

	if ( DebugKeyInput::IsKeyDown( 'W' ) ) {
		m_TreeNo						= SmallTree;
		m_SmallTreeState.Transform		= m_Transform;
		m_SmallTreeLeafState.Transform	= m_Transform;
		m_SmallTreeLeafState.Color		= m_LeafColor;
	}

	if ( DebugKeyInput::IsKeyDown( 'E' ) ) {
		m_TreeNo						= Tree;
		m_TreeState.Transform			= m_Transform;
		m_TreeLeafState.Transform		= m_Transform;
		m_TreeLeafState.Color			= m_LeafColor;
	}

	if ( DebugKeyInput::IsKeyDown( '1' ) ) m_LeafColor = Color4::White;
	if ( DebugKeyInput::IsKeyDown( '2' ) ) m_LeafColor = Color4::Yellow;
	if ( DebugKeyInput::IsKeyDown( '3' ) ) m_LeafColor = Color4::Pink;
	if ( DebugKeyInput::IsKeyDown( '4' ) ) m_LeafColor = Color4::Darkgreen;
	if ( DebugKeyInput::IsKeyDown( '5' ) ) m_LeafColor = Color4::Red;

	m_TreeLeafState.Color		= m_LeafColor;
	m_SmallTreeLeafState.Color	= m_LeafColor;
}

//---------------------------.
// �`��.
//---------------------------.
void CTree::SubRender()
{
	if ( m_IsDisp == false ) return;

	switch ( m_TreeNo ) {
	case Sapling:
		m_pSapling->RenderUI( &m_SaplingState );
		break;
	case SmallTree:
		m_pSmallTree->RenderUI( &m_SmallTreeState );
		m_pSmallTreeLeaf->RenderUI( &m_SmallTreeLeafState );
		break;
	case Tree:
		m_pTree->RenderUI( &m_TreeState );
		m_pTreeLeaf->RenderUI( &m_TreeLeafState );
		break;
	default:
		break;
	}
}

//---------------------------.
// �c�؂𖄂߂�.
//---------------------------.
void CTree::Fill( const D3DXPOSITION3& Pos )
{
	if ( m_IsDisp ) return;

	// �c�؂ɐݒ�.
	m_TreeNo = Sapling;
	m_IsDisp = true;

	// �^�X�N�o�[�̃T�C�Y���擾.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// �c�؂̈ʒu�̐ݒ�.
	m_Transform.Position	 = Pos;
	m_Transform.Position.y	 = static_cast<float>( Rect.top );
	m_SaplingState.Transform = m_Transform;
}
