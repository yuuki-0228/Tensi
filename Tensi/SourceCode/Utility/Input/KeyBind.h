#pragma once
#include <Xinput.h>

// キーバインドの設定で使用.
namespace{
	constexpr WORD XINPUT_LSTICK_U = 0x0050;	// 左 スティックの 上方向入力
	constexpr WORD XINPUT_LSTICK_D = 0x0051;	// 左 スティックの 下方向入力.
	constexpr WORD XINPUT_LSTICK_L = 0x0052;	// 左 スティックの 左方向入力.
	constexpr WORD XINPUT_LSTICK_R = 0x0053;	// 左 スティックの 右方向入力.
	constexpr WORD XINPUT_RSTICK_U = 0x0054;	// 右 スティックの 上方向入力.
	constexpr WORD XINPUT_RSTICK_D = 0x0055;	// 右 スティックの 下方向入力.
	constexpr WORD XINPUT_RSTICK_L = 0x0056;	// 右 スティックの 左方向入力.
	constexpr WORD XINPUT_RSTICK_R = 0x0057;	// 右 スティックの 右方向入力.
	constexpr WORD XINPUT_LTRIGGER = 0x0058;	// 左 トリガーの	  入力.
	constexpr WORD XINPUT_RTRIGGER = 0x0059;	// 右 トリガーの	  入力.
}

// キーバインドのペア構造体.
struct stKeyBindPair {
	std::vector<int>	Key;	// キー.
	std::vector<WORD>	But;	// Xinputのボタン.

} typedef SKeyBindPair;

// キーバインドのマップ.
using KeyBind_map = std::unordered_map<std::string, SKeyBindPair>;
