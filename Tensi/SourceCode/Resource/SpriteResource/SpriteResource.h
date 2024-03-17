#pragma once
#include "..\..\Global.h"
#include "..\..\Common\Sprite\Sprite.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <filesystem>

class CSprite;

/************************************************
*	�X�v���C�g���\�[�X�N���X.
*		���c�F�P.
**/
class SpriteResource final
{
public:
	using Sprite_map	= std::unordered_map<std::string, std::shared_ptr<CSprite>>;
	using Sprite_List	= std::vector<std::string>;

public:
	SpriteResource();
	~SpriteResource();

	// �X�v���C�g�̓ǂݍ���.
	static HRESULT SpriteLoad();

	// �X�v���C�g�擾�֐�.
	static CSprite* GetSprite( const std::string& FileName, SSpriteRenderState*		pState = nullptr );
	static CSprite* GetSprite( const std::string& FileName, SSpriteRenderStateList* pState );

	// �ۑ����Ă���X�v���C�g�̖��O���X�g�̎擾.
	static Sprite_List GetSpriteNames() { return GetInstance()->m_SpriteNames; }

private:
	// �C���X�^���X�̎擾.
	static SpriteResource* GetInstance();

private:
	Sprite_map	m_SpriteList;	// �X�v���C�g���X�g.
	Sprite_List	m_SpriteNames;	// �X�v���C�g�̖��O���X�g.
	std::mutex	m_Mutex;
	bool		m_IsLoadEnd;	// �ǂݍ��݂��I�������.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	SpriteResource( const SpriteResource& )				= delete;
	SpriteResource& operator= ( const SpriteResource& )	= delete;
	SpriteResource( SpriteResource&& )					= delete;
	SpriteResource& operator = ( SpriteResource&& )		= delete;
};