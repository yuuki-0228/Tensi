#pragma once
#include "..\..\Global.h"
#include "..\..\Common\Font\Font.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <filesystem>

class CFont;

/***********************************
*	�t�H���g���\�[�X�N���X.
**/
class FontResource
{
public:
	using Font_map			= std::unordered_map<std::string, std::shared_ptr<CFont>>;
	using Font_List			= std::vector<std::string>;
	using Font_Texture_map	= std::unordered_map<std::string, std::unordered_map<std::string, ID3D11ShaderResourceView*>>;
	using Font_Data_map		= std::unordered_map<std::string, std::string>;

public:
	FontResource();
	~FontResource();

	// �t�H���g�̓ǂݍ���.
	static HRESULT FontLoad();

	// �t�H���g�̎擾.
	static CFont* GetFont( const std::string& FileName, SFontRenderState*		pState = nullptr );
	static CFont* GetFont( const std::string& FileName, SFontRenderStateList*	pState );

	// �t�H���g�e�N�X�`���擾�֐�.
	static ID3D11ShaderResourceView* GetFontTexture( const std::string& FileName, const std::string& Key );

	// �ۑ����Ă���X�v���C�g�̖��O���X�g�̎擾.
	static Font_List GetFontNames() { return GetInstance()->m_FontNames; }

private:
	// �C���X�^���X�̎擾.
	static FontResource* GetInstance();

	// �e�N�X�`���̍쐬.
	static HRESULT CreateTexture( const std::string& FileName, const std::string& Key );

private:
	Font_map			m_FontList;			// �t�H���g���X�g.
	Font_Texture_map	m_FontTextureList;	// �t�H���g�e�N�X�`�����X�g.
	Font_Data_map		m_FontDataList;		// �t�H���g�f�[�^���X�g.
	Font_List			m_FontNames;		// �t�H���g�̖��O���X�g.
	std::mutex			m_Mutex;
	bool				m_IsLoadEnd;		// �ǂݍ��݂��I�������.
};