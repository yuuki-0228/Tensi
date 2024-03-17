#include "SystemBase.h"
#include "..\Scene\SceneManager\SceneManager.h"

CSystemBase::CSystemBase()
	: m_RenderFunc	()
	, m_IsOpen		( false, "", "System" )
	, m_WndName		( "" )
	, m_IsOldOpen	( false )
{
}

CSystemBase::~CSystemBase()
{
}

//---------------------------.
// �`��.
//	�`�悵�Ȃ��ꍇ�͏����͍s��Ȃ�.
//---------------------------.
void CSystemBase::Render()
{
	// �V�X�e���E�B���h�E���J���Ă���ԃV�[�����~������.
	bool Flag	= m_IsOpen.get();
	if ( m_IsOldOpen != Flag ) SceneManager::SetIsSceneStop( Flag );
	m_IsOldOpen = Flag;

	// �E�B���h�E���J���Ă��Ȃ��ꍇ�����͍s��Ȃ�.
	if ( m_IsOpen == false ) return;

	// �t���O�������Ă���ꍇ�A�E�B���h�E���J��.
	ImGui::Begin( m_WndName.c_str(), &Flag );

	// �`�揈��.
	m_RenderFunc();

	// �E�B���h�E�̏����I��.
	ImGui::End();
	m_IsOpen = Flag;
}

//---------------------------.
// �E�B���h�E���̏�����.
//---------------------------.
void CSystemBase::InitWndName( const std::string& Name )
{
	m_WndName = Name;
	m_IsOpen.SetName( Name + u8"�̕\��/��\��" );
}