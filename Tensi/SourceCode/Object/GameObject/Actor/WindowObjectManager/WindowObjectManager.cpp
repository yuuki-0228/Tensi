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
// インスタンスの取得.
//---------------------------.
WindowObjectManager* WindowObjectManager::GetInstance()
{
	static std::unique_ptr<WindowObjectManager> pInstance = std::make_unique<WindowObjectManager>();
	return pInstance.get();
}

//---------------------------.
// オブジェクトの取得.
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
// トランスフォームの取得.
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
// マウスがオブジェクトの上にある場合そのオブジェクトを取得(上にない場合:nullptr).
//---------------------------.
CWindowObject* WindowObjectManager::GetMouseOverTheObject()
{
	WindowObjectManager* pI = GetInstance();

	// 位置の取得.
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// オブジェクトに当たっているか調べる.
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
// マウスがウィンドウのオブジェクトの上にあるか取得.
//---------------------------.
bool WindowObjectManager::GetIsMouseOverTheObject()
{
	return GetMouseOverTheObject() != nullptr;
}

//---------------------------.
// ウィンドウオブジェクトの追加.
//---------------------------.
void WindowObjectManager::PushObject( CWindowObject* pActor, STransform* pTransform )
{
	WindowObjectManager* pI = GetInstance();

	// タグの取得.
	const EObjectTag Tag = pActor->GetObjectTag();

	// 情報を追加する.
	pI->m_pObjectList[Tag].emplace_back( pActor );
	pI->m_pObjectTransformList[Tag].emplace_back( pTransform );
}
