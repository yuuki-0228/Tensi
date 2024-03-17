#pragma once
#include "..\SystemBase.h"

/************************************************
*	バグ報告クラス.
*		﨑田友輝.
**/
class CBugReport final
	: public CSystemBase
{
public:
	using BugList_map = std::unordered_map<std::string, std::vector<std::string>>;

private:
	// バグレベル列挙体.
	enum class enBugLv : unsigned char {
		S,	// ゲーム進行に問題が出る( 普通にゲームしていて遭遇する ).
		A,	// プレイヤーに著しい不利益をもたらす( 特定の手順で発生する ).
		B,	// プレイヤーにある程度不利益をもたらす( ゲーム進行に問題はない ).
		C,	// 明らかに不具合だが致命的ではない.
		D	// 不具合とまではいかないが、気になる.
	} typedef EBugLv;

public:
	CBugReport();
	~CBugReport();

	// バグが無いか取得.
	bool GetBugListEmpty() { return m_BugList.empty(); }

protected:
	// 初期化関数.
	virtual void Init() override;

private:
	// バグリストを読み込む.
	HRESULT BugListLoad();

private:
	std::string	m_BugTitle;		// バグのタイトル.
	std::string	m_BugContents;	// バグの説明.
	std::string	m_Discoverer;	// 発見者.
	EBugLv		m_BugLv;		// バグのレベル.
	BugList_map	m_BugList;		// バグリスト.
};
