#include "ActorCollisionManager.h"
#include "..\Actor\Actor.h"

namespace{
	const int OBJECT_TAG_MIN = static_cast<int>( EObjectTag::None );	// �I�u�W�F�N�g�^�O�̍ŏ��l.
	const int OBJECT_TAG_MAX = static_cast<int>( EObjectTag::Max );		// �I�u�W�F�N�g�^�O�̍ő�l.
	const std::vector<bool> INIT_LIST( OBJECT_TAG_MAX, false );			// �������p���X�g.
}

ActorCollisionManager::ActorCollisionManager()
	: m_HitActorList	()
	, m_BeAHitActorList	()
	, m_SkipTag			()
	, m_SkipTagToTag	()
{
	// �I�u�W�F�N�g�^�O���z���p��.
	m_SkipTag.resize( OBJECT_TAG_MAX );
	m_SkipTagToTag.resize( OBJECT_TAG_MAX );
	for ( auto& t : m_SkipTagToTag ) t.resize( OBJECT_TAG_MAX );
}

ActorCollisionManager::~ActorCollisionManager()
{
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
ActorCollisionManager* ActorCollisionManager::GetInstance()
{
	static std::unique_ptr<ActorCollisionManager> pInstance = std::make_unique<ActorCollisionManager>();
	return pInstance.get();
}

//---------------------------.
// �����蔻��.
//---------------------------.
void ActorCollisionManager::Collision()
{
	ActorCollisionManager* pI = GetInstance();

	// �����蔻����s���I�u�W�F�N�g���Ȃ��ꍇ.
	//	�s���K�v���Ȃ����ߔ�����.
	if ( pI->m_HitActorList.empty() ||
		 pI->m_BeAHitActorList.empty() )
	{
		// ���X�g�̏�����.
		pI->m_HitActorList.clear();
		pI->m_BeAHitActorList.clear();
		return;
	}

	// ���Ă�I�u�W�F�N�g.
	for ( auto& BaAHit_a : pI->m_BeAHitActorList ){
		if ( BaAHit_a == nullptr		) continue;

		// �^�O��ۑ�.
		const EObjectTag BaAHit_Tag		= BaAHit_a->GetObjectTag();
		const int		 BaAHit_TagNo	= static_cast<int>( BaAHit_Tag );

		// �^�O���͈͊O/�X�L�b�v����^�O��.
		if ( BaAHit_TagNo <= OBJECT_TAG_MIN	) continue;
		if ( BaAHit_TagNo >= OBJECT_TAG_MAX	) continue;
		if ( pI->m_SkipTag[BaAHit_TagNo]	) continue;

		// ���Ă���I�u�W�F�N�g.
		for ( auto& Hit_a : pI->m_HitActorList ){
			if ( Hit_a == nullptr		) continue;

			// �������g���m�œ����蔻��͍s��Ȃ��Ă������ߔ�����.
			if ( BaAHit_a == Hit_a ) continue;

			// �^�O��ۑ�.
			const EObjectTag Hit_Tag		= Hit_a->GetObjectTag();
			const int		 Hit_TagNo	= static_cast<int>( Hit_Tag );

			// �^�O���͈͊O/�X�L�b�v����^�O��.
			if ( Hit_TagNo <= OBJECT_TAG_MIN					) continue;
			if ( Hit_TagNo >= OBJECT_TAG_MAX					) continue;
			if ( pI->m_SkipTag[Hit_TagNo]						) continue;
			if ( pI->m_SkipTagToTag[BaAHit_TagNo][Hit_TagNo]	) continue;

			// �����蔻����s��.
			BaAHit_a->Collision( Hit_a );
		}
	}
	// ���X�g�̏�����.
	pI->m_HitActorList.clear();
	pI->m_BeAHitActorList.clear();
}

//---------------------------.
// �����蔻��̃��X�g��������.
//---------------------------.
void ActorCollisionManager::Reset()
{
	ActorCollisionManager* pI = GetInstance();

	// ���X�g�Ɠ����蔻����s��Ȃ����X�g�̏�����.
	pI->m_HitActorList.clear();
	pI->m_BeAHitActorList.clear();
	pI->m_SkipTag = INIT_LIST;
	for ( auto& vt : pI->m_SkipTagToTag )
		vt = INIT_LIST;
}
