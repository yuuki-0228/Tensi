#pragma once
#include "..\..\..\..\Global.h"
#include "..\..\GameObjectList.h"

class CWindowObject;

/************************************************
*	�E�B���h�E�̃I�u�W�F�N�g�}�l�[�W���[.
**/
class WindowObjectManager
{
public:
	WindowObjectManager();
	~WindowObjectManager();
	
	// �I�u�W�F�N�g�̎擾.
	static std::vector<CWindowObject*>	GetObjectPtr( const EObjectTag Tag );
	static CWindowObject*				GetObjectPtr( const EObjectTag Tag, const int No );

	// �g�����X�t�H�[���̎擾.
	static std::vector<STransform*>		GetTransform( const EObjectTag Tag );
	static STransform*					GetTransform( const EObjectTag Tag, const int No );

	// �}�E�X���I�u�W�F�N�g�̏�ɂ���ꍇ���̃I�u�W�F�N�g���擾(��ɂȂ��ꍇ:nullptr).
	static CWindowObject* GetMouseOverTheObject();
	// �}�E�X���I�u�W�F�N�g�̏�ɂ��邩�擾.
	static bool GetIsMouseOverTheObject();

	// �E�B���h�E�I�u�W�F�N�g�̒ǉ�.
	static void PushObject( CWindowObject* pActor, STransform* pTransform );

private:
	// �C���X�^���X�̎擾.
	static WindowObjectManager* GetInstance();

private:
	std::unordered_map<EObjectTag, std::vector<CWindowObject*>>	m_pObjectList;			// �I�u�W�F�N�g���X�g.
	std::unordered_map<EObjectTag, std::vector<STransform*>>	m_pObjectTransformList;	// �I�u�W�F�N�g�̃g�����X�t�H�[�����X�g.
};