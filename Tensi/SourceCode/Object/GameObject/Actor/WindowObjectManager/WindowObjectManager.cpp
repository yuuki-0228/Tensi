#include "WindowObjectManager.h"
#include "..\WindowObject\WindowObject.h"
#include "..\..\..\..\Utility\Input\Input.h"

WindowObjectManager::WindowObjectManager()
	: m_pObjectList				()
	, m_pObjectTransformList	()
{
}

WindowObjectManager::~WindowObjectManager()
{
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
WindowObjectManager* WindowObjectManager::GetInstance()
{
	static std::unique_ptr<WindowObjectManager> pInstance = std::make_unique<WindowObjectManager>();
	return pInstance.get();
}

//---------------------------.
// �I�u�W�F�N�g�̎擾.
//---------------------------.
std::vector<CWindowObject*> WindowObjectManager::GetObjectPtr( const EObjectTag Tag )
{
	return GetInstance()->m_pObjectList[Tag];
}
CWindowObject* WindowObjectManager::GetObjectPtr( const EObjectTag Tag, const int No )
{
	return GetInstance()->m_pObjectList[Tag][No];
}

//---------------------------.
// �g�����X�t�H�[���̎擾.
//---------------------------.
std::vector<STransform*> WindowObjectManager::GetTransform( const EObjectTag Tag )
{
	return GetInstance()->m_pObjectTransformList[Tag];
}
STransform* WindowObjectManager::GetTransform( const EObjectTag Tag, const int No )
{
	return GetInstance()->m_pObjectTransformList[Tag][No];
}

//---------------------------.
// �}�E�X���I�u�W�F�N�g�̏�ɂ���ꍇ���̃I�u�W�F�N�g���擾(��ɂȂ��ꍇ:nullptr).
//---------------------------.
CWindowObject* WindowObjectManager::GetMouseOverTheObject()
{
	WindowObjectManager* pI = GetInstance();

	// �ʒu�̎擾.
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// �I�u�W�F�N�g�ɓ������Ă��邩���ׂ�.
	for ( auto& [Tag, List] : pI->m_pObjectList ) {
		for ( auto& Obj : List ) {
			if ( Obj->GetIsMouseOver() )
			{
				return Obj;
			}
		}
	}
	return nullptr;
}

//---------------------------.
// �}�E�X���E�B���h�E�̃I�u�W�F�N�g�̏�ɂ��邩�擾.
//---------------------------.
bool WindowObjectManager::GetIsMouseOverTheObject()
{
	return GetMouseOverTheObject() != nullptr;
}

//---------------------------.
// �E�B���h�E�I�u�W�F�N�g�̒ǉ�.
//---------------------------.
void WindowObjectManager::PushObject( CWindowObject* pActor, STransform* pTransform )
{
	WindowObjectManager* pI = GetInstance();

	// �^�O�̎擾.
	const EObjectTag Tag = pActor->GetObjectTag();

	// ����ǉ�����.
	pI->m_pObjectList[Tag].emplace_back( pActor );
	pI->m_pObjectTransformList[Tag].emplace_back( pTransform );
}
