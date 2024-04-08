#pragma once
#include "..\..\Global.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <filesystem>

class CEffect;

/************************************************
*	エフェクトリソースクラス.
**/
class EffectResource final
{
public:
	using Effect_map	= std::unordered_map<std::string, std::unique_ptr<CEffect>>;
	using Effect_List	= std::vector<std::string>;

public:
	EffectResource();
	~EffectResource();

	// エフェクトの読み込み.
	static HRESULT EffectLoad();

	// エフェクト取得関数.
	static CEffect* GetEffect( const std::string& FileName );

	// 保存しているエフェクトリストの取得.
	static Effect_List GetEffectList() { return GetInstance()->m_EffectNames; }

private:
	// インスタンスの取得.
	static EffectResource* GetInstance();

private:
	Effect_map	m_EffectList;	// エフェクトリスト.
	Effect_List	m_EffectNames;	// エフェクトの名前リスト.
	std::mutex	m_Mutex;
	bool		m_IsLoadEnd;	// 読み込みが終わったか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	EffectResource( const EffectResource& )				= delete;
	EffectResource& operator = ( const EffectResource& )	= delete;
	EffectResource( EffectResource&& )					= delete;
	EffectResource& operator = ( EffectResource&& )		= delete;
};