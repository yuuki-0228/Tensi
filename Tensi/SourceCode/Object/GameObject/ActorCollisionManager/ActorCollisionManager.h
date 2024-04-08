#pragma once
#include "..\..\GameObject\GameObject.h"

class CActor;

// �����蔻�������I�u�W�F�N�g�̎��.
enum class enCollObjType : unsigned char {
	Both,	// ����.
	Hit,	// ���Ă�.
	BeAHit	// ���Ă���( Collision()���Ăяo����� ).

} typedef ECollObjType;

/************************************************
*	�I�u�W�F�N�g�̓����蔻��}�l�[�W���[�N���X.
**/
class ActorCollisionManager final
{
public:
	// �����蔻����s��Ȃ��^�O���X�g.
	using SkipTagList = std::vector<bool>;

public:
	ActorCollisionManager();
	~ActorCollisionManager();

	// �����蔻��.
	static void Collision();

	// �����蔻��̃��X�g��������.
	//	�V�[���؂�ւ����Ɏg�p����.
	static void Reset();

	// �����蔻����s���I�u�W�F�N�g��ǉ�..
	inline static void PushObject( CActor* pActor, const ECollObjType& Type = ECollObjType::Both ) {
		if ( Type == ECollObjType::Both || Type == ECollObjType::Hit ) {
			GetInstance()->m_HitActorList.emplace_back( pActor );
		}
		if ( Type == ECollObjType::Both || Type == ECollObjType::BeAHit ) {
			GetInstance()->m_BeAHitActorList.emplace_back( pActor );
		}
	}

	// ����̃^�O�̓����蔻��̗L��/����.
	inline static void SetSkipTag( EObjectTag tag, bool Flag ) {
		GetInstance()->m_SkipTag[static_cast<int>( tag )] = Flag;
	}
	// ����̃^�O���m�̓����蔻��̗L��/����.
	inline static void SetSkipTagToTag( EObjectTag mtag, EObjectTag ytag, bool Flag ) {
		GetInstance()->m_SkipTagToTag[static_cast<int>( mtag )][static_cast<int>( ytag )] = Flag;
		GetInstance()->m_SkipTagToTag[static_cast<int>( ytag )][static_cast<int>( mtag )] = Flag;
	}

private:
	// �C���X�^���X�̎擾.
	static ActorCollisionManager* GetInstance();

private:
	std::vector<CActor*>		m_HitActorList;		// ���Ă�I�u�W�F�N�g���X�g.
	std::vector<CActor*>		m_BeAHitActorList;	// ���Ă���I�u�W�F�N�g���X�g.
	SkipTagList					m_SkipTag;			// �����蔻����s��Ȃ���.
	std::vector<SkipTagList>	m_SkipTagToTag;		// ����̃^�O���m�̓����蔻����Ȃ���.
};
