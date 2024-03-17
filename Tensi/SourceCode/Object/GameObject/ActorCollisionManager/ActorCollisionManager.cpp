#include "ActorCollisionManager.h"
#include "..\Actor\Actor.h"

namespace{
	const int OBJECT_TAG_MIN = static_cast<int>( EObjectTag::None );	// オブジェクトタグの最小値.
	const int OBJECT_TAG_MAX = static_cast<int>( EObjectTag::Max );		// オブジェクトタグの最大値.
	const std::vector<bool> INIT_LIST( OBJECT_TAG_MAX, false );			// 初期化用リスト.
}

ActorCollisionManager::ActorCollisionManager()
	: m_HitActorList	()
	, m_BeAHitActorList	()
	, m_SkipTag			()
	, m_SkipTagToTag	()
{
	// オブジェクトタグ分配列を用意.
	m_SkipTag.resize( OBJECT_TAG_MAX );
	m_SkipTagToTag.resize( OBJECT_TAG_MAX );
	for ( auto& t : m_SkipTagToTag ) t.resize( OBJECT_TAG_MAX );
}

ActorCollisionManager::~ActorCollisionManager()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
ActorCollisionManager* ActorCollisionManager::GetInstance()
{
	static std::unique_ptr<ActorCollisionManager> pInstance = std::make_unique<ActorCollisionManager>();
	return pInstance.get();
}

//---------------------------.
// 当たり判定.
//---------------------------.
void ActorCollisionManager::Collision()
{
	ActorCollisionManager* pI = GetInstance();

	// 当たり判定を行うオブジェクトがない場合.
	//	行う必要がないため抜ける.
	if ( pI->m_HitActorList.empty() ||
		 pI->m_BeAHitActorList.empty() )
	{
		// リストの初期化.
		pI->m_HitActorList.clear();
		pI->m_BeAHitActorList.clear();
		return;
	}

	// 当てるオブジェクト.
	for ( auto& BaAHit_a : pI->m_BeAHitActorList ){
		if ( BaAHit_a == nullptr		) continue;

		// タグを保存.
		const EObjectTag BaAHit_Tag		= BaAHit_a->GetObjectTag();
		const int		 BaAHit_TagNo	= static_cast<int>( BaAHit_Tag );

		// タグが範囲外/スキップするタグか.
		if ( BaAHit_TagNo <= OBJECT_TAG_MIN	) continue;
		if ( BaAHit_TagNo >= OBJECT_TAG_MAX	) continue;
		if ( pI->m_SkipTag[BaAHit_TagNo]	) continue;

		// 当てられるオブジェクト.
		for ( auto& Hit_a : pI->m_HitActorList ){
			if ( Hit_a == nullptr		) continue;

			// 自分自身同士で当たり判定は行わなくていいため抜ける.
			if ( BaAHit_a == Hit_a ) continue;

			// タグを保存.
			const EObjectTag Hit_Tag		= Hit_a->GetObjectTag();
			const int		 Hit_TagNo	= static_cast<int>( Hit_Tag );

			// タグが範囲外/スキップするタグか.
			if ( Hit_TagNo <= OBJECT_TAG_MIN					) continue;
			if ( Hit_TagNo >= OBJECT_TAG_MAX					) continue;
			if ( pI->m_SkipTag[Hit_TagNo]						) continue;
			if ( pI->m_SkipTagToTag[BaAHit_TagNo][Hit_TagNo]	) continue;

			// 当たり判定を行う.
			BaAHit_a->Collision( Hit_a );
		}
	}
	// リストの初期化.
	pI->m_HitActorList.clear();
	pI->m_BeAHitActorList.clear();
}

//---------------------------.
// 当たり判定のリストを初期化.
//---------------------------.
void ActorCollisionManager::Reset()
{
	ActorCollisionManager* pI = GetInstance();

	// リストと当たり判定を行わないリストの初期化.
	pI->m_HitActorList.clear();
	pI->m_BeAHitActorList.clear();
	pI->m_SkipTag = INIT_LIST;
	for ( auto& vt : pI->m_SkipTagToTag )
		vt = INIT_LIST;
}
