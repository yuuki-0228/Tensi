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
*	フォントリソースクラス.
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

	// フォントの読み込み.
	static HRESULT FontLoad();

	// フォントの取得.
	static CFont* GetFont( const std::string& FileName, SFontRenderState*		pState = nullptr );
	static CFont* GetFont( const std::string& FileName, SFontRenderStateList*	pState );

	// フォントテクスチャ取得関数.
	static ID3D11ShaderResourceView* GetFontTexture( const std::string& FileName, const std::string& Key );

	// 保存しているスプライトの名前リストの取得.
	static Font_List GetFontNames() { return GetInstance()->m_FontNames; }

private:
	// インスタンスの取得.
	static FontResource* GetInstance();

	// テクスチャの作成.
	static HRESULT CreateTexture( const std::string& FileName, const std::string& Key );

private:
	Font_map			m_FontList;			// フォントリスト.
	Font_Texture_map	m_FontTextureList;	// フォントテクスチャリスト.
	Font_Data_map		m_FontDataList;		// フォントデータリスト.
	Font_List			m_FontNames;		// フォントの名前リスト.
	std::mutex			m_Mutex;
	bool				m_IsLoadEnd;		// 読み込みが終わったか.
};