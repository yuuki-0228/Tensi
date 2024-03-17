#pragma once
#include "..\..\..\..\Global.h"
#include "..\..\GameObjectList.h"

class CWindowObject;

/************************************************
*	ウィンドウのオブジェクトマネージャー.
**/
class WindowObjectManager
{
public:
	WindowObjectManager();
	~WindowObjectManager();
	
	// オブジェクトの取得.
	static std::vector<CWindowObject*>	GetObjectPtr( const EObjectTag Tag );
	static CWindowObject*				GetObjectPtr( const EObjectTag Tag, const int No );

	// トランスフォームの取得.
	static std::vector<STransform*>		GetTransform( const EObjectTag Tag );
	static STransform*					GetTransform( const EObjectTag Tag, const int No );

	// マウスがオブジェクトの上にある場合そのオブジェクトを取得(上にない場合:nullptr).
	static CWindowObject* GetMouseOverTheObject();
	// マウスがオブジェクトの上にあるか取得.
	static bool GetIsMouseOverTheObject();

	// ウィンドウオブジェクトの追加.
	static void PushObject( CWindowObject* pActor, STransform* pTransform );

private:
	// インスタンスの取得.
	static WindowObjectManager* GetInstance();

private:
	std::unordered_map<EObjectTag, std::vector<CWindowObject*>>	m_pObjectList;			// オブジェクトリスト.
	std::unordered_map<EObjectTag, std::vector<STransform*>>	m_pObjectTransformList;	// オブジェクトのトランスフォームリスト.
};