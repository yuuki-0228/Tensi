#pragma once

// _stprintf関数で使用.
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tchar.h>

//警告についてのコード分析を無効にする.4005:再定義.
#pragma warning(disable:4005)
#include <Windows.h>
#include <crtdbg.h>
#include <memory>
#include <limits>

#include <d3dx9.h>
#include <D3DX11.h>
#include <D3D11.h>
#include <D3DX10.h>
#include <D3D10.h>
#include <d3d10.h>

#include "Utility\Log\Log.h"
#include "Utility\Bool\Bool.h"
#include "Utility\Color\Color.h"
#include "Utility\Transform\Transform.h"

//ライブラリ読み込み.
#pragma comment( lib, "winmm.lib"	)
#pragma comment( lib, "d3d9.lib"	)
#pragma comment( lib, "d3dx9.lib"	)
#pragma comment( lib, "d3dx11.lib"	)
#pragma comment( lib, "d3d11.lib"	)
#pragma comment( lib, "d3dx10.lib"	)

//----------------------------.
//	定数.
//----------------------------.
// ウィンドウ関係.
constexpr int		WND_W			= 1280;		// ウィンドウの幅.
constexpr int		WND_H			= 720;		// ウィンドウの高さ.
constexpr int		FPS				= 60;		// フレームレート.
constexpr float		FWND_W			= static_cast<float>( WND_W );
constexpr float		FWND_H			= static_cast<float>( WND_H );
constexpr float		FFPS			= static_cast<float>( FPS );

// 初期化用.
constexpr int		INIT_INT		= 0;
constexpr int		INIT_INT2[2]	= { INIT_INT, INIT_INT };
constexpr int		INIT_INT3[3]	= { INIT_INT, INIT_INT, INIT_INT };
constexpr int		INIT_INT4[4]	= { INIT_INT, INIT_INT, INIT_INT, INIT_INT };
constexpr float		INIT_FLOAT		= 0.0f;
constexpr float		INIT_FLOAT2[2]	= { INIT_FLOAT, INIT_FLOAT };
constexpr float		INIT_FLOAT3[3]	= { INIT_FLOAT, INIT_FLOAT, INIT_FLOAT };
constexpr float		INIT_FLOAT4[4]	= { INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT };

// エラー用.
constexpr int		ERROR_INT		= std::numeric_limits<int>::infinity();
constexpr int		ERROR_INT2[2]	= { ERROR_INT, ERROR_INT };
constexpr int		ERROR_INT3[3]	= { ERROR_INT, ERROR_INT, ERROR_INT };
constexpr int		ERROR_INT4[4]	= { ERROR_INT, ERROR_INT, ERROR_INT, ERROR_INT };
constexpr float 	ERROR_FLOAT		= std::numeric_limits<float>::infinity();
constexpr float		ERROR_FLOAT2[2]	= { ERROR_FLOAT, ERROR_FLOAT };
constexpr float 	ERROR_FLOAT3[3]	= { ERROR_FLOAT, ERROR_FLOAT, ERROR_FLOAT };
constexpr float 	ERROR_FLOAT4[4]	= { ERROR_FLOAT, ERROR_FLOAT, ERROR_FLOAT, ERROR_FLOAT };

//----------------------------.
//	列挙体.
//----------------------------.
// 方向.
enum class enDirection : unsigned char{
	Left,		 L	= Left,			// 左.
	Right,		 R	= Right,		// 右.
	Up,			 U	= Up,			// 上.
	Down,		 D	= Down,			// 下.

	Front,		 F	= Front,		// 前.
	Center,		 C	= Center,		// 真ん中.
	Back,		 B	= Back,			// 後ろ.

	LeftUp,		 LU	= LeftUp,		// 左の上.
	LeftDown,	 LD	= LeftDown,		// 左の下.
	LeftFront,	 LF	= LeftFront,	// 左の前.
	LeftCenter,	 LC	= LeftCenter,	// 左の真ん中.
	LeftBack,	 LB	= LeftBack,		// 左の後ろ.

	RightUp,	 RU	= RightUp,		// 右の上.
	RightDown,	 RD	= RightDown,	// 右の下.
	RightFront,	 RF	= RightFront,	// 右の前.
	RightCenter, RC	= RightCenter,	// 右の真ん中.
	RightBack,	 RB	= RightBack,	// 右の後ろ.

	UpLeft,		 UL	= UpLeft,		// 上の左.
	UpRight,	 UR	= UpRight,		// 上の右.
	UpFront,	 UF	= UpFront,		// 上の前.
	UpCenter,	 UC	= UpCenter,		// 上の真ん中.
	UpBack,		 UB	= UpBack,		// 上の後ろ.

	DownLeft,	 DL	= DownLeft,		// 下の左.
	DownRight,	 DR	= DownRight,	// 下の右.
	DownFront,	 DF	= DownFront,	// 下の前.
	DownCenter,	 DC	= DownCenter,	// 下の真ん中.
	DownBack,	 DB	= DownBack,		// 下の後ろ.

	FrontLeft,	 FL	= FrontLeft,	// 前の左.
	FrontRight,  FR	= FrontRight,	// 前の右.
	FrontUp,	 FU	= FrontUp,		// 前の上.
	FrontDown,	 FD	= FrontDown,	// 前の下.

	CenterLeft,	 CL	= CenterLeft,	// 真ん中の左.
	CenterRight, CR	= CenterRight,	// 真ん中の右.
	CenterUp,	 CU	= CenterUp,		// 真ん中の上.
	CenterDown,	 CD	= CenterDown,	// 真ん中の下.

	BackLeft,	 BL	= BackLeft,		// 後ろの左.
	BackRight,	 BR	= BackRight,	// 後ろの右.
	BackUp,		 BU	= BackUp,		// 後ろの上.
	BackDown,	 BD	= BackDown,		// 後ろの下.

	East,		 E	= East,			// 東.
	West,		 W	= West,			// 西.
	South,		 S	= South,		// 南.
	North,		 N	= North,		// 北.
	NorthEast,	 NE	= NorthEast,	// 北東.
	NorthWest,	 NW	= NorthWest,	// 北西.
	SouthEast,	 SE	= SouthEast,	// 南東.
	SouthWest,	 SW	= SouthWest,	// 南西.

	None,
} typedef EDirection, EDire;

// 数字.
enum class enNumbers : unsigned char{
	Zero,	_0	= Zero,		// 0.
	One,	_1	= One,		// 1. 
	Two,	_2	= Two,		// 2. 
	Three,	_3	= Three,	// 3.
	Four,	_4	= Four,		// 4. 
	Five,	_5	= Five,		// 5. 
	Six,	_6	= Six,		// 6. 
	Seven,	_7	= Seven,	// 7. 
	Eight,	_8	= Eight,	// 8.
	Nine,	_9	= Nine,		// 9
	Ten,	_10	= Ten,		// 10. 

	None = 0
} typedef ENumbers, ENum;

//----------------------------.
//	マクロ.
//----------------------------.
//解放.
#define SAFE_RELEASE(p)	if(p!=nullptr){(p)->Release();(p)=nullptr;}
//破棄.
#define SAFE_DELETE(p) if(p!=nullptr){delete (p);(p)=nullptr;}
#define SAFE_DELETE_ARRAY(p)	\
{								\
	if(p!=nullptr){				\
		delete[] (p);			\
		(p) = nullptr;			\
	}							\
}

// _declspec():DLLから(関数,クラス,クラスのメンバ関数を)エクスポートする.
//	algn():(強制的に)16byteで使用する.
#define ALIGN16 _declspec( align( 16 ) )

//----------------------------.
// エラーメッセージ.
//----------------------------.
#include <string>
#include <codecvt>
#include <iostream>
#include <sstream>
#include "Utility\StringConversion\StringConversion.h"

template<class T = std::string>
void ErrorMessage( const std::string& ErrorMsg, const HRESULT r = S_OK )
{
	// エラーメッセージが空の場合.
	if ( ErrorMsg.empty() ) {
		_ASSERT_EXPR( false, L"Not Error Message" );
	}
	
	// エラーメッセージの表示.
	const std::wstring wMsg = StringConversion::to_wString( ErrorMsg );
	Log::PushLog( ErrorMsg.c_str() );
	if ( FAILED( r ) ) {
		// エラーコード出力
		std::ostringstream ss;
		ss << "ErrorCode : 0x" << std::hex << static_cast< int >( r );
		Log::PushLog( ss.str() );
	}
	_ASSERT_EXPR( false, wMsg.c_str() );
	MessageBox( nullptr, wMsg.c_str(), _T( "Warning" ), MB_OK );

#ifndef _DEBUG
	// フリーズ対策のためウィンドウを破棄する
	PostQuitMessage( 0 );
#endif
}

template<class T = std::string>
void InfoMessage( const std::string& ErrorMsg )
{
	// エラーメッセージが空の場合.
	if ( ErrorMsg.empty() ) {
		_ASSERT_EXPR( false, L"Not Error Message" );
	}

	// エラーメッセージの表示.
	const std::wstring wMsg = StringConversion::to_wString( ErrorMsg );
	MessageBox( nullptr, wMsg.c_str(), _T( "Info" ), MB_OK );
}