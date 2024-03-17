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
*	スプライトリソースクラス.
*		﨑田友輝.
**/
class SpriteResource final
{
public:
	using Sprite_map	= std::unordered_map<std::string, std::shared_ptr<CSprite>>;
	using Sprite_List	= std::vector<std::string>;

public:
	SpriteResource();
	~SpriteResource();

	// スプライトの読み込み.
	static HRESULT SpriteLoad();

	// スプライト取得関数.
	static CSprite* GetSprite( const std::string& FileName, SSpriteRenderState*		pState = nullptr );
	static CSprite* GetSprite( const std::string& FileName, SSpriteRenderStateList* pState );

	// 保存しているスプライトの名前リストの取得.
	static Sprite_List GetSpriteNames() { return GetInstance()->m_SpriteNames; }

private:
	// インスタンスの取得.
	static SpriteResource* GetInstance();

private:
	Sprite_map	m_SpriteList;	// スプライトリスト.
	Sprite_List	m_SpriteNames;	// スプライトの名前リスト.
	std::mutex	m_Mutex;
	bool		m_IsLoadEnd;	// 読み込みが終わったか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	SpriteResource( const SpriteResource& )				= delete;
	SpriteResource& operator= ( const SpriteResource& )	= delete;
	SpriteResource( SpriteResource&& )					= delete;
	SpriteResource& operator = ( SpriteResource&& )		= delete;
};