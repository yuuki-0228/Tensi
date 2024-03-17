#pragma once
#include "..\..\Global.h"
#include "..\..\Utility\Input\KeyBind.h"

/************************************************
*	入力リスト.
*		﨑田友輝.
**/
namespace InputList{
	// キーボード入力リスト<入力名, 値>.
	const std::vector<std::pair<std::string, int>> KeyList = {
		{ "None",				0			},
		{ "A",					'A'			},
		{ "B",					'B'			},
		{ "C",					'C'			},
		{ "D",					'D'			},
		{ "E",					'E'			},
		{ "F",					'F'			},
		{ "G",					'G'			},
		{ "H",					'H'			},
		{ "I",					'I'			},
		{ "J",					'J'			},
		{ "K",					'K'			},
		{ "L",					'L'			},
		{ "M",					'M'			},
		{ "N",					'N'			},
		{ "O",					'O'			},
		{ "P",					'P'			},
		{ "Q",					'Q'			},
		{ "R",					'R'			},
		{ "S",					'S'			},
		{ "T",					'T'			},
		{ "U",					'U'			},
		{ "V",					'V'			},
		{ "W",					'W'			},
		{ "X",					'X'			},
		{ "Y",					'Y'			},
		{ "Z",					'Z'			},
		{ "0",					'0'			},
		{ "1",					'1'			},
		{ "2",					'2'			},
		{ "3",					'3'			},
		{ "4",					'4'			},
		{ "5",					'5'			},
		{ "6",					'6'			},
		{ "7",					'7'			},
		{ "8",					'8'			},
		{ "9",					'9'			},
		{ "-",					'-'			},
		{ "^",					'^'			},
		{ "@",					'@'			},
		{ "[",					'['			},
		{ ";",					';'			},
		{ ":",					':'			},
		{ "]",					']'			},
		{ "/",					'/'			},
		{ "Up",					VK_UP		},
		{ "Down",				VK_DOWN		},
		{ "Left",				VK_LEFT		},
		{ "Right",				VK_RIGHT	},
		{ "Space",				VK_SPACE	},
		{ "Enter",				VK_RETURN	},
		{ "Escape",				VK_ESCAPE	},
		{ "Tab",				VK_TAB		},
		{ "Shift",				VK_SHIFT	},
		{ "LeftShift",			VK_LSHIFT	},
		{ "RightShift",			VK_RSHIFT	},
		{ "Shift",				VK_SHIFT	},
		{ "LeftControl",		VK_LCONTROL	},
		{ "RightControl",		VK_RCONTROL	},
		{ "Control",			VK_CONTROL	},
		{ "Alt",				VK_MENU		},
		{ "BackSpace",			VK_BACK		},
		{ "Delete",				VK_DELETE	},
		{ "Home",				VK_HOME		},
		{ "End",				VK_END		},
		{ "CapsLock",			VK_CAPITAL	},
		{ "Insert",				VK_INSERT	},
		{ "PageUp",				VK_PRIOR	},
		{ "PageDown",			VK_NEXT		},
		{ "LeftWindows",		VK_LWIN		},
		{ "RightWindows",		VK_RWIN		},
		{ "Application",		VK_APPS		},
		{ "NumLock",			VK_NUMLOCK	},
		{ "PrintScreen",		VK_SNAPSHOT },
		{ "Scroll",				VK_SCROLL	},
		{ "Pause",				VK_PAUSE	},
		{ "LeftClick",			VK_LBUTTON	},
		{ "CenterClick",		VK_MBUTTON	},
		{ "RightClick",			VK_RBUTTON	},
		{ "F1",					VK_F1		},
		{ "F2",					VK_F2		},
		{ "F3",					VK_F3		},
		{ "F4",					VK_F4		},
		{ "F5",					VK_F5		},
		{ "F6",					VK_F6		},
		{ "F7",					VK_F7		},
		{ "F8",					VK_F8		},
		{ "F9",					VK_F9		},
		{ "F10",				VK_F10		},
		{ "F11",				VK_F11		},
		{ "F12",				VK_F12		},
		{ "NumericKeyPad[0]",	VK_NUMPAD0	},
		{ "NumericKeyPad[1]",	VK_NUMPAD1	},
		{ "NumericKeyPad[2]",	VK_NUMPAD2	},
		{ "NumericKeyPad[3]",	VK_NUMPAD3	},
		{ "NumericKeyPad[4]",	VK_NUMPAD4	},
		{ "NumericKeyPad[5]",	VK_NUMPAD5	},
		{ "NumericKeyPad[6]",	VK_NUMPAD6	},
		{ "NumericKeyPad[7]",	VK_NUMPAD7	},
		{ "NumericKeyPad[8]",	VK_NUMPAD8	},
		{ "NumericKeyPad[9]",	VK_NUMPAD9	},
		{ "NumericKeyPad[+]",	VK_ADD		},
		{ "NumericKeyPad[-]",	VK_SUBTRACT	},
		{ "NumericKeyPad[*]",	VK_MULTIPLY	},
		{ "NumericKeyPad[/]",	VK_DIVIDE	},
		{ "NumericKeyPad[.]",	VK_DECIMAL	},
	};

	//--------------------------.
	// キーのリストを取得.
	//--------------------------.
	inline std::vector<std::string> GetKeyList() {
		std::vector<std::string> Out;
		Out.reserve( InputList::KeyList.size() );
		for ( auto& l : InputList::KeyList ) Out.emplace_back( l.first );
		return Out;
	}

	//---------------------------.
	// キーデータをキー文字列に変換.
	//---------------------------.
	inline std::string KeyDataToString( int Key ) {
		for ( auto& l : InputList::KeyList ) {
			if ( l.second == Key ) return l.first;
		}
		return "";
	}

	//---------------------------.
	// キー文字列をキーデータに変換.
	//---------------------------.
	inline int StringToKeyData( std::string Key ) {
		for ( auto& l : InputList::KeyList ) {
			if ( l.first == Key ) return l.second;
		}
		return 0;
	}

	// コントローラで使用する入力リスト<入力名, 値>.
	const std::vector<std::pair<std::string, WORD>> ButList = {
		{ "None",				0								},
		{ "A",					XINPUT_GAMEPAD_A				},
		{ "B",					XINPUT_GAMEPAD_B				},
		{ "X",					XINPUT_GAMEPAD_X				},
		{ "Y",					XINPUT_GAMEPAD_Y				},
		{ "Up",					XINPUT_GAMEPAD_DPAD_UP			},
		{ "Down",				XINPUT_GAMEPAD_DPAD_DOWN		},
		{ "Left",				XINPUT_GAMEPAD_DPAD_LEFT		},
		{ "Right",				XINPUT_GAMEPAD_DPAD_RIGHT		},
		{ "Start",				XINPUT_GAMEPAD_START			},
		{ "Back",				XINPUT_GAMEPAD_BACK				},
		{ "LeftShoulder",		XINPUT_GAMEPAD_LEFT_SHOULDER	},
		{ "RightShoulder",		XINPUT_GAMEPAD_RIGHT_SHOULDER	},
		{ "LeftStick[Up]",		XINPUT_LSTICK_U					},
		{ "LeftStick[Down]",	XINPUT_LSTICK_D					},
		{ "LeftStick[Left]",	XINPUT_LSTICK_L					},
		{ "LeftStick[Right]",	XINPUT_LSTICK_R					},
		{ "RightStick[Up]",		XINPUT_RSTICK_U					},
		{ "RightStick[Down]",	XINPUT_RSTICK_D					},
		{ "RightStick[Left]",	XINPUT_RSTICK_L					},
		{ "RightStick[Right]",	XINPUT_RSTICK_R					},
		{ "LeftTrigger",		XINPUT_LTRIGGER					},
		{ "RightTrigger",		XINPUT_RTRIGGER					},
	};

	//-----------------------.
	// ボタンのリストを取得.
	//-----------------------.
	inline std::vector<std::string> GetButList() {
		std::vector<std::string> Out;
		Out.reserve( InputList::ButList.size() );
		for ( auto& l : InputList::ButList ) Out.emplace_back( l.first );
		return Out;
	}

	//-----------------------.
	// ボタンデータをボタン文字に変換.
	//-----------------------.
	inline std::string ButDataToString( WORD But ) {
		for ( auto& l : InputList::ButList ) {
			if ( l.second == But ) return l.first;
		}
		return "";
	}

	//-----------------------.
	// ボタン文字をボタンデータに変換.
	//-----------------------.
	inline WORD StringToButData( std::string But ) {
		for ( auto& l : InputList::ButList ) {
			if ( l.first == But ) return l.second;
		}
		return 0;
	}
}