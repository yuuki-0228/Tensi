#pragma once
#include "..\..\Actor.h"

class CWateringEffect;

/************************************************
*	花の水やりエフェクトマネージャークラス.
**/
class CWateringEffectManager final
	: public CActor
{
public:
	CWateringEffectManager();
	~CWateringEffectManager();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// 再生
	void Play( const D3DXPOSITION3& Pos );

private:
	// 水リストの削除.
	void WateringListDelete();

private:
	std::vector<std::unique_ptr<CWateringEffect>> m_pWateringList;	// 水やり.

};
