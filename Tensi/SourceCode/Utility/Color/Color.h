#pragma once
#include "..\..\.\Global.h"
#include <algorithm>

using D3DXCOLOR3 = D3DXVECTOR3;
using D3DXCOLOR4 = D3DXVECTOR4;

/************************************************
*	色一覧( R,G,B ).
*	https://www.webcreatorbox.com/webinfo/color-name
*		﨑田友輝.
**/
namespace Color3 {
	constexpr float COLOR_MAX = 1.0f; // 色の最大値.
	constexpr float COLOR_MID = 0.5f; // 色の中間.
	constexpr float COLOR_MIN = 0.0f; // 色の最小値.
	constexpr float ALPHA_MAX = 1.0f; // アルファの最大値.
	constexpr float ALPHA_MID = 0.5f; // アルファの中間.
	constexpr float ALPHA_MIN = 0.0f; // アルファの最小値.

	// 初期化用.
	const D3DXCOLOR3 INIT = { COLOR_MAX, COLOR_MAX, COLOR_MAX };
	const D3DXCOLOR3 None = { COLOR_MIN, COLOR_MIN, COLOR_MIN };

	// アルファ値を範囲内に収める.
	inline float AlphaClamp( float  Alpha ) { return std::clamp( Alpha, ALPHA_MIN, ALPHA_MAX ); }
	inline void	 AlphaClamp( float* Alpha ) { *Alpha = AlphaClamp( *Alpha ); }
	
	// 色( R,G,B )を作成.
	inline D3DXCOLOR3 RGB_( const int	r, const int	g, const int	b ) { return D3DXCOLOR3( r / 255.0f, g / 255.0f, b / 255.0f ); }
	inline D3DXCOLOR3 RGB_( const float	r, const float	g, const float	b ) { return D3DXCOLOR3( r, g, b ); }

	// 赤系の色.
	namespace RedSeries {
		const D3DXCOLOR3 Indianred				= RGB_( 205,  92,  92 );
		const D3DXCOLOR3 Lightcoral				= RGB_( 240, 128, 128 );
		const D3DXCOLOR3 Salmon					= RGB_( 250, 128, 114 );
		const D3DXCOLOR3 Darksalmon				= RGB_( 233, 150, 122 );
		const D3DXCOLOR3 Crimson				= RGB_( 220,  20,  60 );
		const D3DXCOLOR3 Firebrick				= RGB_( 178,  34,  34 );
		const D3DXCOLOR3 Red					= RGB_( 255,   0,   0 );
		const D3DXCOLOR3 Darkred				= RGB_( 139,   0,   0 );
	}
	using namespace RedSeries;

	// ピンク系の色.
	namespace PinkSeries {
		const D3DXCOLOR3 Pink					= RGB_( 255, 192, 203 );
		const D3DXCOLOR3 Lightpink				= RGB_( 255, 182, 193 );
		const D3DXCOLOR3 Hotpink				= RGB_( 255, 105, 180 );
		const D3DXCOLOR3 Deeppink				= RGB_( 255,  20, 147 );
		const D3DXCOLOR3 Mediumvioletred		= RGB_( 199,  21, 133 );
		const D3DXCOLOR3 Palevioletred			= RGB_( 219, 112, 147 );
	}
	using namespace PinkSeries;

	// オレンジ系の色.
	namespace OrangeSeries {
		const D3DXCOLOR3 Lightsalmon			= RGB_( 255, 160, 122 );
		const D3DXCOLOR3 Coral					= RGB_( 255, 127,  80 );
		const D3DXCOLOR3 Tomato					= RGB_( 255,  99,  71 );
		const D3DXCOLOR3 Orangered				= RGB_( 255,  69,   0 );
		const D3DXCOLOR3 Darkorange				= RGB_( 255, 140,   0 );
		const D3DXCOLOR3 Orange					= RGB_( 255, 165,   0 );
	}
	using namespace OrangeSeries;

	// 黄系の色.
	namespace YellowSeries {
		const D3DXCOLOR3 Gold					= RGB_( 255, 215,   0 );
		const D3DXCOLOR3 Yellow					= RGB_( 255, 255,   0 );
		const D3DXCOLOR3 Lightyellow			= RGB_( 255, 255, 224 );
		const D3DXCOLOR3 Lemonchiffon			= RGB_( 255, 250, 205 );
		const D3DXCOLOR3 Lightgoldenrodyellow	= RGB_( 250, 250, 210 );
		const D3DXCOLOR3 Papayawhip				= RGB_( 255, 239, 213 );
		const D3DXCOLOR3 Moccasin				= RGB_( 255, 228, 181 );
		const D3DXCOLOR3 Peachpuff				= RGB_( 255, 218, 185 );
		const D3DXCOLOR3 Palegoldenrod			= RGB_( 238, 232, 170 );
		const D3DXCOLOR3 Khaki					= RGB_( 240, 230, 140 );
		const D3DXCOLOR3 Darkkhaki				= RGB_( 189, 183, 107 );
	}
	using namespace YellowSeries;

	// 緑系の色.
	namespace GreenSeries {
		const D3DXCOLOR3 Greenyellow			= RGB_( 173, 255,  47 );
		const D3DXCOLOR3 Chartreuse				= RGB_( 127, 255,   0 );
		const D3DXCOLOR3 Lawngreen				= RGB_( 124, 252,   0 );
		const D3DXCOLOR3 Lime					= RGB_(   0, 255,   0 );
		const D3DXCOLOR3 Limegreen				= RGB_(  50, 205,  50 );
		const D3DXCOLOR3 Palegreen				= RGB_( 152, 251, 152 );
		const D3DXCOLOR3 Lightgreen				= RGB_( 144, 238, 144 );
		const D3DXCOLOR3 Mediumspringgreen		= RGB_(   0, 250, 154 );
		const D3DXCOLOR3 Springgreen			= RGB_(   0, 255, 127 );
		const D3DXCOLOR3 Mediumseagreen			= RGB_(  60, 179, 113 );
		const D3DXCOLOR3 Seagreen				= RGB_(  46, 139,  87 );
		const D3DXCOLOR3 Forestgreen			= RGB_(  34, 139,  34 );
		const D3DXCOLOR3 Green					= RGB_(   0, 128,   0 );
		const D3DXCOLOR3 Darkgreen				= RGB_(   0, 100,   0 );
		const D3DXCOLOR3 Yellowgreen			= RGB_( 154, 205,  50 );
		const D3DXCOLOR3 Olivedrab				= RGB_( 107, 142,  35 );
		const D3DXCOLOR3 Olive					= RGB_( 128, 128,   0 );
		const D3DXCOLOR3 Darkolivegreen			= RGB_(  85, 107,  47 );
		const D3DXCOLOR3 Mediumaquamarine		= RGB_( 102, 205, 170 );
		const D3DXCOLOR3 Darkseagreen			= RGB_( 143, 188, 139 );
		const D3DXCOLOR3 Lightseagreen			= RGB_(  32, 178, 170 );
		const D3DXCOLOR3 Darkcyan				= RGB_(   0, 139, 139 );
		const D3DXCOLOR3 Teal					= RGB_(   0, 128, 128 );
	}
	using namespace GreenSeries;

	// 青系の色.
	namespace BlueSeries {
		const D3DXCOLOR3 Aqua					= RGB_(   0, 255, 255 );
		const D3DXCOLOR3 Cyan					= RGB_(   0, 255, 255 );
		const D3DXCOLOR3 Lightcyan				= RGB_( 224, 255, 255 );
		const D3DXCOLOR3 Paleturquoise			= RGB_( 175, 238, 238 );
		const D3DXCOLOR3 Aquamarine				= RGB_( 127, 255, 212 );
		const D3DXCOLOR3 Turquoise				= RGB_(  64, 224, 208 );
		const D3DXCOLOR3 Mediumturquoise		= RGB_(  72, 209, 204 );
		const D3DXCOLOR3 Darkturquoise			= RGB_(  95, 158, 160 );
		const D3DXCOLOR3 Steelblue				= RGB_(  70, 130, 180 );
		const D3DXCOLOR3 Lightsteelblue			= RGB_( 176, 196, 222 );
		const D3DXCOLOR3 Powderblue				= RGB_( 176, 224, 230 );
		const D3DXCOLOR3 Lightblue				= RGB_( 173, 216, 230 );
		const D3DXCOLOR3 Skyblue				= RGB_( 135, 206, 235 );
		const D3DXCOLOR3 Lightskyblue			= RGB_( 135, 206, 250 );
		const D3DXCOLOR3 Deepskyblue			= RGB_(   0, 191, 255 );
		const D3DXCOLOR3 Dodgerblue				= RGB_(  30, 144, 255 );
		const D3DXCOLOR3 Cornflowerblue			= RGB_( 100, 149, 237 );
		const D3DXCOLOR3 Royalblue				= RGB_(  65, 105, 225 );
		const D3DXCOLOR3 Blue					= RGB_(   0,   0, 255 );
		const D3DXCOLOR3 Mediumblue				= RGB_(   0,   0, 205 );
		const D3DXCOLOR3 Darkblue				= RGB_(   0,   0, 139 );
		const D3DXCOLOR3 Navy					= RGB_(   0,   0, 128 );
		const D3DXCOLOR3 Midnightblue			= RGB_(  25,  25, 112 );
	}
	using namespace BlueSeries;

	// 紫系の色.
	namespace PurpleSeries {
		const D3DXCOLOR3 Lavender				= RGB_( 230, 230, 250 );
		const D3DXCOLOR3 Thistle				= RGB_( 216, 191, 216 );
		const D3DXCOLOR3 Plum					= RGB_( 221, 160, 221 );
		const D3DXCOLOR3 Violet					= RGB_( 238, 130, 238 );
		const D3DXCOLOR3 Orchid					= RGB_( 218, 112, 214 );
		const D3DXCOLOR3 Fuchsia				= RGB_( 255,   0, 255 );
		const D3DXCOLOR3 Magenta				= RGB_( 255,   0, 255 );
		const D3DXCOLOR3 Mediumorchid			= RGB_( 186,  85, 211 );
		const D3DXCOLOR3 Mediumpurple			= RGB_( 147, 112, 219 );
		const D3DXCOLOR3 Rebeccapurple			= RGB_( 102,  51, 153 );
		const D3DXCOLOR3 Blueviolet				= RGB_( 138,  43, 226 );
		const D3DXCOLOR3 Darkviolet				= RGB_( 148,   0, 211 );
		const D3DXCOLOR3 Darkorchid				= RGB_( 153,  50, 204 );
		const D3DXCOLOR3 Darkmagenta			= RGB_( 139,   0, 139 );
		const D3DXCOLOR3 Purple					= RGB_( 128,   0, 128 );
		const D3DXCOLOR3 Indigo					= RGB_(  75,   0, 130 );
		const D3DXCOLOR3 Slateblue				= RGB_( 106,  90, 205 );
		const D3DXCOLOR3 Darkslateblue			= RGB_(  72,  61, 139 );
		const D3DXCOLOR3 Mediumslateblue		= RGB_( 123, 104, 238 );
	}
	using namespace PurpleSeries;

	// 茶系の色.
	namespace BrownSeries {
		const D3DXCOLOR3 Cornsilk				= RGB_( 255, 248, 220 );
		const D3DXCOLOR3 Blanchedalmond			= RGB_( 255, 235, 205 );
		const D3DXCOLOR3 Bisque					= RGB_( 255, 228, 196 );
		const D3DXCOLOR3 Navajowhite			= RGB_( 255, 222, 173 );
		const D3DXCOLOR3 Wheat					= RGB_( 245, 222, 179 );
		const D3DXCOLOR3 Burlywood				= RGB_( 222, 184, 135 );
		const D3DXCOLOR3 Tan					= RGB_( 210, 180, 140 );
		const D3DXCOLOR3 Rosybrown				= RGB_( 188, 143, 143 );
		const D3DXCOLOR3 Sandybrown				= RGB_( 244, 164,  96 );
		const D3DXCOLOR3 Goldenrod				= RGB_( 218, 165,  32 );
		const D3DXCOLOR3 Darkgoldenrod			= RGB_( 184, 134,  11 );
		const D3DXCOLOR3 Peru					= RGB_( 205, 133,  63 );
		const D3DXCOLOR3 Chocolate				= RGB_( 210, 105,  30 );
		const D3DXCOLOR3 Saddlebrown			= RGB_( 139,  69,  19 );
		const D3DXCOLOR3 Sienna					= RGB_( 160,  82,  45 );
		const D3DXCOLOR3 Brown					= RGB_( 165,  42,  42 );
		const D3DXCOLOR3 Maroon					= RGB_( 128,   0,   0 );
	}
	using namespace BrownSeries;

	// 白系の色.
	namespace WhiteSeries {
		const D3DXCOLOR3 White					= RGB_( 255, 255, 255 );
		const D3DXCOLOR3 Snow					= RGB_( 255, 250, 250 );
		const D3DXCOLOR3 Honeydew				= RGB_( 240, 255, 240 );
		const D3DXCOLOR3 Mintcream				= RGB_( 245, 255, 250 );
		const D3DXCOLOR3 Azure					= RGB_( 240, 255, 255 );
		const D3DXCOLOR3 Aliceblue				= RGB_( 240, 248, 255 );
		const D3DXCOLOR3 Ghostwhite				= RGB_( 248, 248, 255 );
		const D3DXCOLOR3 Whitesmoke				= RGB_( 245, 245, 245 );
		const D3DXCOLOR3 Seashell				= RGB_( 255, 245, 238 );
		const D3DXCOLOR3 Beige					= RGB_( 245, 245, 220 );
		const D3DXCOLOR3 Oldlace				= RGB_( 253, 245, 230 );
		const D3DXCOLOR3 Floralwhite			= RGB_( 255, 250, 240 );
		const D3DXCOLOR3 Ivory					= RGB_( 255, 255, 240 );
		const D3DXCOLOR3 Antiquewhite			= RGB_( 250, 235, 215 );
		const D3DXCOLOR3 Linen					= RGB_( 250, 240, 230 );
		const D3DXCOLOR3 Lavenderblush			= RGB_( 255, 240, 245 );
		const D3DXCOLOR3 Mistyrose				= RGB_( 255, 228, 225 );
	}
	using namespace WhiteSeries;

	// 黒系の色.
	namespace BlackSeries {
		const D3DXCOLOR3 Gainsboro				= RGB_( 220, 220, 220 );
		const D3DXCOLOR3 Lightgray				= RGB_( 211, 211, 211 );
		const D3DXCOLOR3 Silver					= RGB_( 192, 192, 192 );
		const D3DXCOLOR3 Darkgray				= RGB_(  73,  73,  73 );
		const D3DXCOLOR3 Gray					= RGB_( 121, 121, 121 );
		const D3DXCOLOR3 Dimgray				= RGB_( 105, 105, 105 );
		const D3DXCOLOR3 Lightslategray			= RGB_( 119, 136, 153 );
		const D3DXCOLOR3 Slategray				= RGB_( 112, 128, 144 );
		const D3DXCOLOR3 Darkslategray			= RGB_(  47,  79,  79 );
		const D3DXCOLOR3 Black					= RGB_(   0,   0,   0 );
	}
	using namespace BlackSeries;
}
// 色一覧( R,G,B ).
namespace Color = Color3;

/************************************************
*	色一覧( R,G,B,A ).
*	https://www.webcreatorbox.com/webinfo/color-name
*		﨑田友輝.
**/
namespace Color4 {
	constexpr float COLOR_MAX = 1.0f; // 色の最大値.
	constexpr float COLOR_MIN = 0.0f; // 色の最小値.
	constexpr float ALPHA_MAX = 1.0f; // アルファの最大値.
	constexpr float ALPHA_MIN = 0.0f; // アルファの最小値.

	// 初期化用.
	const D3DXCOLOR4 INIT = { COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX };
	const D3DXCOLOR4 None = { COLOR_MIN, COLOR_MIN, COLOR_MIN, ALPHA_MIN };

	// アルファ値を範囲内に収める.
	inline float AlphaClamp( float  Alpha ) { return std::clamp( Alpha, ALPHA_MIN, ALPHA_MAX ); }
	inline void	 AlphaClamp( float* Alpha ) { *Alpha = AlphaClamp( *Alpha ); }

	// 色( R,G,B,A )を作成.
	inline D3DXCOLOR4 RGBA( const int	r, const int	g, const int	b, const int	a = ALPHA_MAX ) { return D3DXCOLOR4( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f ); }
	inline D3DXCOLOR4 RGBA( const float r, const float	g, const float	b, const float	a = ALPHA_MAX ) { return D3DXCOLOR4( r, g, b, a ); }
	inline D3DXCOLOR4 RGBA( const D3DXCOLOR3& Color, const float a = ALPHA_MAX ) { return D3DXCOLOR4( Color, a ); }

	// 赤系の色.
	namespace RedSeries {
		const D3DXCOLOR4 Indianred				= RGBA( Color3::Indianred				);
		const D3DXCOLOR4 Lightcoral				= RGBA( Color3::Lightcoral				);
		const D3DXCOLOR4 Salmon					= RGBA( Color3::Salmon					);
		const D3DXCOLOR4 Darksalmon				= RGBA( Color3::Darksalmon				);
		const D3DXCOLOR4 Crimson				= RGBA( Color3::Crimson					);
		const D3DXCOLOR4 Firebrick				= RGBA( Color3::Firebrick				);
		const D3DXCOLOR4 Red					= RGBA( Color3::Red						);
		const D3DXCOLOR4 Darkred				= RGBA( Color3::Darkred					);
	}
	using namespace RedSeries;

	// ピンク系の色.
	namespace PinkSeries {
		const D3DXCOLOR4 Pink					= RGBA( Color3::Pink					);
		const D3DXCOLOR4 Lightpink				= RGBA( Color3::Lightpink				);
		const D3DXCOLOR4 Hotpink				= RGBA( Color3::Hotpink					);
		const D3DXCOLOR4 Deeppink				= RGBA( Color3::Deeppink				);
		const D3DXCOLOR4 Mediumvioletred		= RGBA( Color3::Mediumvioletred			);
		const D3DXCOLOR4 Palevioletred			= RGBA( Color3::Palevioletred			);
	}
	using namespace PinkSeries;

	// オレンジ系の色.
	namespace OrangeSeries {
		const D3DXCOLOR4 Lightsalmon			= RGBA( Color3::Lightsalmon				);
		const D3DXCOLOR4 Coral					= RGBA( Color3::Coral					);
		const D3DXCOLOR4 Tomato					= RGBA( Color3::Tomato					);
		const D3DXCOLOR4 Orangered				= RGBA( Color3::Orangered				);
		const D3DXCOLOR4 Darkorange				= RGBA( Color3::Darkorange				);
		const D3DXCOLOR4 Orange					= RGBA( Color3::Orange					);
	}
	using namespace OrangeSeries;

	// 黄系の色.
	namespace YellowSeries {
		const D3DXCOLOR4 Gold					= RGBA( Color3::Gold					);
		const D3DXCOLOR4 Yellow					= RGBA( Color3::Yellow					);
		const D3DXCOLOR4 Lightyellow			= RGBA( Color3::Lightyellow				);
		const D3DXCOLOR4 Lemonchiffon			= RGBA( Color3::Lemonchiffon			);
		const D3DXCOLOR4 Lightgoldenrodyellow	= RGBA( Color3::Lightgoldenrodyellow	);
		const D3DXCOLOR4 Papayawhip				= RGBA( Color3::Papayawhip				);
		const D3DXCOLOR4 Moccasin				= RGBA( Color3::Moccasin				);
		const D3DXCOLOR4 Peachpuff				= RGBA( Color3::Peachpuff				);
		const D3DXCOLOR4 Palegoldenrod			= RGBA( Color3::Palegoldenrod			);
		const D3DXCOLOR4 Khaki					= RGBA( Color3::Khaki					);
		const D3DXCOLOR4 Darkkhaki				= RGBA( Color3::Darkkhaki				);
	}
	using namespace YellowSeries;

	// 緑系の色.
	namespace GreenSeries {
		const D3DXCOLOR4 Greenyellow			= RGBA( Color3::Greenyellow				);
		const D3DXCOLOR4 Chartreuse				= RGBA( Color3::Chartreuse				);
		const D3DXCOLOR4 Lawngreen				= RGBA( Color3::Lawngreen				);
		const D3DXCOLOR4 Lime					= RGBA( Color3::Lime					);
		const D3DXCOLOR4 Limegreen				= RGBA( Color3::Limegreen				);
		const D3DXCOLOR4 Palegreen				= RGBA( Color3::Palegreen				);
		const D3DXCOLOR4 Lightgreen				= RGBA( Color3::Lightgreen				);
		const D3DXCOLOR4 Mediumspringgreen		= RGBA( Color3::Mediumspringgreen		);
		const D3DXCOLOR4 Springgreen			= RGBA( Color3::Springgreen				);
		const D3DXCOLOR4 Mediumseagreen			= RGBA( Color3::Mediumseagreen			);
		const D3DXCOLOR4 Seagreen				= RGBA( Color3::Seagreen				);
		const D3DXCOLOR4 Forestgreen			= RGBA( Color3::Forestgreen				);
		const D3DXCOLOR4 Green					= RGBA( Color3::Green					);
		const D3DXCOLOR4 Darkgreen				= RGBA( Color3::Darkgreen				);
		const D3DXCOLOR4 Yellowgreen			= RGBA( Color3::Yellowgreen				);
		const D3DXCOLOR4 Olivedrab				= RGBA( Color3::Olivedrab				);
		const D3DXCOLOR4 Olive					= RGBA( Color3::Olive					);
		const D3DXCOLOR4 Darkolivegreen			= RGBA( Color3::Darkolivegreen			);
		const D3DXCOLOR4 Mediumaquamarine		= RGBA( Color3::Mediumaquamarine		);
		const D3DXCOLOR4 Darkseagreen			= RGBA( Color3::Darkseagreen			);
		const D3DXCOLOR4 Lightseagreen			= RGBA( Color3::Lightseagreen			);
		const D3DXCOLOR4 Darkcyan				= RGBA( Color3::Darkcyan				);
		const D3DXCOLOR4 Teal					= RGBA( Color3::Teal					);
	}
	using namespace GreenSeries;

	// 青系の色.
	namespace BlueSeries {
		const D3DXCOLOR4 Aqua					= RGBA( Color3::Aqua					);
		const D3DXCOLOR4 Cyan					= RGBA( Color3::Cyan					);
		const D3DXCOLOR4 Lightcyan				= RGBA( Color3::Lightcyan				);
		const D3DXCOLOR4 Paleturquoise			= RGBA( Color3::Paleturquoise			);
		const D3DXCOLOR4 Aquamarine				= RGBA( Color3::Aquamarine				);
		const D3DXCOLOR4 Turquoise				= RGBA( Color3::Turquoise				);
		const D3DXCOLOR4 Mediumturquoise		= RGBA( Color3::Mediumturquoise			);
		const D3DXCOLOR4 Darkturquoise			= RGBA( Color3::Darkturquoise			);
		const D3DXCOLOR4 Steelblue				= RGBA( Color3::Steelblue				);
		const D3DXCOLOR4 Lightsteelblue			= RGBA( Color3::Lightsteelblue			);
		const D3DXCOLOR4 Powderblue				= RGBA( Color3::Powderblue				);
		const D3DXCOLOR4 Lightblue				= RGBA( Color3::Lightblue				);
		const D3DXCOLOR4 Skyblue				= RGBA( Color3::Skyblue					);
		const D3DXCOLOR4 Lightskyblue			= RGBA( Color3::Lightskyblue			);
		const D3DXCOLOR4 Deepskyblue			= RGBA( Color3::Deepskyblue				);
		const D3DXCOLOR4 Dodgerblue				= RGBA( Color3::Dodgerblue				);
		const D3DXCOLOR4 Cornflowerblue			= RGBA( Color3::Cornflowerblue			);
		const D3DXCOLOR4 Royalblue				= RGBA( Color3::Royalblue				);
		const D3DXCOLOR4 Blue					= RGBA( Color3::Blue					);
		const D3DXCOLOR4 Mediumblue				= RGBA( Color3::Mediumblue				);
		const D3DXCOLOR4 Darkblue				= RGBA( Color3::Darkblue				);
		const D3DXCOLOR4 Navy					= RGBA( Color3::Navy					);
		const D3DXCOLOR4 Midnightblue			= RGBA( Color3::Midnightblue			);
	}
	using namespace BlueSeries;

	// 紫系の色.
	namespace PurpleSeries {
		const D3DXCOLOR4 Lavender				= RGBA( Color3::Lavender				);
		const D3DXCOLOR4 Thistle				= RGBA( Color3::Thistle					);
		const D3DXCOLOR4 Plum					= RGBA( Color3::Plum					);
		const D3DXCOLOR4 Violet					= RGBA( Color3::Violet					);
		const D3DXCOLOR4 Orchid					= RGBA( Color3::Orchid					);
		const D3DXCOLOR4 Fuchsia				= RGBA( Color3::Fuchsia					);
		const D3DXCOLOR4 Magenta				= RGBA( Color3::Magenta					);
		const D3DXCOLOR4 Mediumorchid			= RGBA( Color3::Mediumorchid			);
		const D3DXCOLOR4 Mediumpurple			= RGBA( Color3::Mediumpurple			);
		const D3DXCOLOR4 Rebeccapurple			= RGBA( Color3::Rebeccapurple			);
		const D3DXCOLOR4 Blueviolet				= RGBA( Color3::Blueviolet				);
		const D3DXCOLOR4 Darkviolet				= RGBA( Color3::Darkviolet				);
		const D3DXCOLOR4 Darkorchid				= RGBA( Color3::Darkorchid				);
		const D3DXCOLOR4 Darkmagenta			= RGBA( Color3::Darkmagenta				);
		const D3DXCOLOR4 Purple					= RGBA( Color3::Purple					);
		const D3DXCOLOR4 Indigo					= RGBA( Color3::Indigo					);
		const D3DXCOLOR4 Slateblue				= RGBA( Color3::Slateblue				);
		const D3DXCOLOR4 Darkslateblue			= RGBA( Color3::Darkslateblue			);
		const D3DXCOLOR4 Mediumslateblue		= RGBA( Color3::Mediumslateblue			);
	}
	using namespace PurpleSeries;

	// 茶系の色.
	namespace BrownSeries {
		const D3DXCOLOR4 Cornsilk				= RGBA( Color3::Cornsilk				);
		const D3DXCOLOR4 Blanchedalmond			= RGBA( Color3::Blanchedalmond			);
		const D3DXCOLOR4 Bisque					= RGBA( Color3::Bisque					);
		const D3DXCOLOR4 Navajowhite			= RGBA( Color3::Navajowhite				);
		const D3DXCOLOR4 Wheat					= RGBA( Color3::Wheat					);
		const D3DXCOLOR4 Burlywood				= RGBA( Color3::Burlywood				);
		const D3DXCOLOR4 Tan					= RGBA( Color3::Tan						);
		const D3DXCOLOR4 Rosybrown				= RGBA( Color3::Rosybrown				);
		const D3DXCOLOR4 Sandybrown				= RGBA( Color3::Sandybrown				);
		const D3DXCOLOR4 Goldenrod				= RGBA( Color3::Goldenrod				);
		const D3DXCOLOR4 Darkgoldenrod			= RGBA( Color3::Darkgoldenrod			);
		const D3DXCOLOR4 Peru					= RGBA( Color3::Peru					);
		const D3DXCOLOR4 Chocolate				= RGBA( Color3::Chocolate				);
		const D3DXCOLOR4 Saddlebrown			= RGBA( Color3::Saddlebrown				);
		const D3DXCOLOR4 Sienna					= RGBA( Color3::Sienna					);
		const D3DXCOLOR4 Brown					= RGBA( Color3::Brown					);
		const D3DXCOLOR4 Maroon					= RGBA( Color3::Maroon					);
	}
	using namespace BrownSeries;

	// 白系の色.
	namespace WhiteSeries {
		const D3DXCOLOR4 White					= RGBA( Color3::White					);
		const D3DXCOLOR4 Snow					= RGBA( Color3::Snow					);
		const D3DXCOLOR4 Honeydew				= RGBA( Color3::Honeydew				);
		const D3DXCOLOR4 Mintcream				= RGBA( Color3::Mintcream				);
		const D3DXCOLOR4 Azure					= RGBA( Color3::Azure					);
		const D3DXCOLOR4 Aliceblue				= RGBA( Color3::Aliceblue				);
		const D3DXCOLOR4 Ghostwhite				= RGBA( Color3::Ghostwhite				);
		const D3DXCOLOR4 Whitesmoke				= RGBA( Color3::Whitesmoke				);
		const D3DXCOLOR4 Seashell				= RGBA( Color3::Seashell				);
		const D3DXCOLOR4 Beige					= RGBA( Color3::Beige					);
		const D3DXCOLOR4 Oldlace				= RGBA( Color3::Oldlace					);
		const D3DXCOLOR4 Floralwhite			= RGBA( Color3::Floralwhite				);
		const D3DXCOLOR4 Ivory					= RGBA( Color3::Ivory					);
		const D3DXCOLOR4 Antiquewhite			= RGBA( Color3::Antiquewhite			);
		const D3DXCOLOR4 Linen					= RGBA( Color3::Linen					);
		const D3DXCOLOR4 Lavenderblush			= RGBA( Color3::Lavenderblush			);
		const D3DXCOLOR4 Mistyrose				= RGBA( Color3::Mistyrose				);
	}
	using namespace WhiteSeries;

	// 黒系の色.
	namespace BlackSeries {
		const D3DXCOLOR4 Gainsboro				= RGBA( Color3::Gainsboro				);
		const D3DXCOLOR4 Lightgray				= RGBA( Color3::Lightgray				);
		const D3DXCOLOR4 Silver					= RGBA( Color3::Silver					);
		const D3DXCOLOR4 Darkgray				= RGBA( Color3::Darkgray				);
		const D3DXCOLOR4 Gray					= RGBA( Color3::Gray					);
		const D3DXCOLOR4 Dimgray				= RGBA( Color3::Dimgray					);
		const D3DXCOLOR4 Lightslategray			= RGBA( Color3::Lightslategray			);
		const D3DXCOLOR4 Slategray				= RGBA( Color3::Slategray				);
		const D3DXCOLOR4 Darkslategray			= RGBA( Color3::Darkslategray			);
		const D3DXCOLOR4 Black					= RGBA( Color3::Black					);
	}
	using namespace BlackSeries;
}
