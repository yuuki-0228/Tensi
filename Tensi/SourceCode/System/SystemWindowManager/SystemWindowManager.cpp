#include "SystemWindowManager.h"
#include "..\InputSetting\InputSetting.h"
#include "..\WindowSetting\WindowSetting.h"
#include "..\BugReport\BugReport.h"
#include "..\..\Common\Sprite\Sprite.h"
#include "..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include "..\..\Utility\Message\Message.h"

SystemWindowManager::SystemWindowManager()
	: m_pInputSetting	( nullptr )
	, m_pWindowSetting	( nullptr )
	, m_pBugReport		( nullptr )
	, m_pButton			( nullptr )
{
	m_pInputSetting		= std::make_unique<InputSetting>();
	m_pWindowSetting	= std::make_unique<CWindowSetting>();
	m_pBugReport		= std::make_unique<CBugReport>();

	m_pButton = SpriteResource::GetSprite( "BugButton" );
	m_pButton->SetIsAllDisp( false );
}

SystemWindowManager::~SystemWindowManager()
{
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
SystemWindowManager* SystemWindowManager::GetInstance()
{
	static std::unique_ptr<SystemWindowManager> pInstance = std::make_unique<SystemWindowManager>();
	return pInstance.get();
}

//---------------------------.
// �X�V.
//---------------------------.
void SystemWindowManager::Update( const float& DeltaTime )
{
	SystemWindowManager* pI = GetInstance();
#if _DEBUG

	// �o�O�{�^���̕\��.
	pI->m_pButton->SetIsAllDisp( !pI->m_pBugReport->GetBugListEmpty() );

	// �o�O�{�^���������ꂽ�ꍇ.
	if ( Message::Check( "BugButtonDown" ) ) pI->m_pBugReport->Open();
#endif
}

//---------------------------.
// �`��.
//---------------------------.
void SystemWindowManager::Render()
{
#if _DEBUG
	SystemWindowManager* pI = GetInstance();

	pI->m_pInputSetting->Render();
	pI->m_pWindowSetting->Render();
	pI->m_pBugReport->Render();
#endif
}
