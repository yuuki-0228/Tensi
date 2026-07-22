#include "FontTagParser.h"
#ifdef ENABLE_FONT
#include <algorithm>
#include <cstdlib>

namespace {
	// タグとして解析する最大の長さ.
	constexpr size_t TAG_LENGTH_MAX = 64;

	// <sup><sub>のサイズ倍率.
	constexpr float SUPSUB_SCALE = 0.6f;
	// <sup>の縦オフセット(文字高に対する比率).
	constexpr float SUP_OFFSET = 0.45f;
	// <sub>の縦オフセット(文字高に対する比率).
	constexpr float SUB_OFFSET = -0.15f;

	// 色名テーブル.
	struct SColorName {
		const char* pName;
		float		r, g, b;
	};
	constexpr SColorName COLOR_NAMES[] = {
		{ "black",		0.0f,	0.0f,	0.0f	},
		{ "white",		1.0f,	1.0f,	1.0f	},
		{ "red",		1.0f,	0.0f,	0.0f	},
		{ "green",		0.0f,	1.0f,	0.0f	},
		{ "blue",		0.0f,	0.0f,	1.0f	},
		{ "yellow",		1.0f,	1.0f,	0.0f	},
		{ "orange",		1.0f,	0.647f,	0.0f	},
		{ "purple",		0.5f,	0.0f,	0.5f	},
		{ "grey",		0.5f,	0.5f,	0.5f	},
		{ "gray",		0.5f,	0.5f,	0.5f	},
		{ "cyan",		0.0f,	1.0f,	1.0f	},
		{ "magenta",	1.0f,	0.0f,	1.0f	},
		{ "pink",		1.0f,	0.753f,	0.796f	},
		{ "brown",		0.647f,	0.165f,	0.165f	},
		{ "lightblue",	0.678f,	0.847f,	0.902f	},
	};

	// 小文字に変換する.
	std::string ToLower( const std::string& Text )
	{
		std::string Out = Text;
		std::transform( Out.begin(), Out.end(), Out.begin(),
			[]( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
		return Out;
	}

	// 16進数2桁を数値に変換する(失敗時は-1).
	int HexToByte( const std::string& Hex, const size_t Pos )
	{
		auto ToNum = []( const char c ) -> int {
			if ( c >= '0' && c <= '9' ) return c - '0';
			if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
			if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
			return -1;
		};
		if ( Pos + 1 >= Hex.size() ) return -1;
		const int Hi = ToNum( Hex[Pos] );
		const int Lo = ToNum( Hex[Pos + 1] );
		if ( Hi < 0 || Lo < 0 ) return -1;
		return Hi * 16 + Lo;
	}

	// カンマ区切りで分割する.
	std::vector<std::string> SplitComma( const std::string& Value )
	{
		std::vector<std::string> Out;
		size_t Begin = 0;
		while ( Begin <= Value.size() ) {
			const size_t End = Value.find( ',', Begin );
			if ( End == std::string::npos ) {
				Out.emplace_back( Value.substr( Begin ) );
				break;
			}
			Out.emplace_back( Value.substr( Begin, End - Begin ) );
			Begin = End + 1;
		}
		return Out;
	}
}

//---------------------------.
// テキストを解析して描画アイテムリストを作成する.
//---------------------------.
SFontDrawItemList CFontTagParser::Parse( const std::string& Text, const SFontRenderState& State, const float BaseCharH )
{
	SFontDrawItemList Out;
	Out.reserve( Text.size() );

	// 基本スタイル(SFontRenderStateの設定値).
	SFontCharStyle Base;
	Base.Color			= State.Color;
	Base.OutLineColor	= State.OutLineColor;
	Base.GlowColor		= State.GlowColor;
	Base.MarkColor		= Color4::None;
	Base.Align			= State.TextAlign;
	Base.SizeScale		= 1.0f;
	Base.OutLineSize	= State.OutLineSize;
	Base.GlowSize		= State.GlowSize;
	Base.GlowPower		= State.GlowPower;
	Base.CharSpace		= 0.0f;
	Base.VOffset		= 0.0f;
	Base.IsBold			= State.IsBold;
	Base.IsItalic		= State.IsItalic;
	Base.IsUnderline	= State.IsUnderline;
	Base.IsStrikeout	= State.IsStrikeout;
	Base.IsOutLine		= State.IsOutLine;
	Base.IsGlow			= State.IsGlow;
	Base.IsMark			= false;

	SFontCharStyle Style = Base;

	// アウトライン/グロー/マーカーの復元用情報.
	struct SOutLineState { D3DXCOLOR4 Color; float Size; bool IsUse; };
	struct SGlowState	 { D3DXCOLOR4 Color; float Size; float Power; bool IsUse; };
	struct SMarkState	 { D3DXCOLOR4 Color; bool IsUse; };
	struct SSupSubState	 { float SizeScale; float VOffset; };

	// ネスト対応用のスタック.
	std::vector<D3DXCOLOR4>		ColorStack;
	std::vector<float>			AlphaStack;
	std::vector<float>			SizeStack;
	std::vector<float>			CSpaceStack;
	std::vector<SOutLineState>	OutLineStack;
	std::vector<SGlowState>		GlowStack;
	std::vector<SMarkState>		MarkStack;
	std::vector<SSupSubState>	SupSubStack;
	int		BoldCnt		= 0;
	int		ItalicCnt	= 0;
	int		UnderCnt	= 0;
	int		StrikeCnt	= 0;
	bool	NoParse		= false;

	// 文字アイテムの追加.
	auto PushChar = [&]( const std::string& c )
	{
		SFontDrawItem Item;
		Item.Char		= c;
		Item.Style		= Style;
		Item.Space		= 0.0f;
		Item.IsNewLine	= false;
		Out.emplace_back( Item );
	};
	// 改行アイテムの追加.
	auto PushNewLine = [&]()
	{
		SFontDrawItem Item;
		Item.Char		= "";
		Item.Style		= Style;
		Item.Space		= 0.0f;
		Item.IsNewLine	= true;
		Out.emplace_back( Item );
	};
	// 送りのみのアイテムの追加.
	auto PushSpace = [&]( const float Space )
	{
		SFontDrawItem Item;
		Item.Char		= "";
		Item.Style		= Style;
		Item.Space		= Space;
		Item.IsNewLine	= false;
		Out.emplace_back( Item );
	};

	// タグの適用(解析できた場合はtrueを返す).
	auto ApplyTag = [&]( const std::string& Inner ) -> bool
	{
		if ( Inner.empty() ) return false;

		// タグ名と値に分割する.
		std::string Tag		= ToLower( Inner );
		std::string Value	= "";
		const size_t Eq = Tag.find( '=' );
		if ( Eq != std::string::npos ) {
			Value	= Inner.substr( Eq + 1 );
			Tag		= Tag.substr( 0, Eq );
		}
		// 終了タグか.
		const bool IsClose = ( Tag.empty() == false && Tag[0] == '/' );
		if ( IsClose ) Tag = Tag.substr( 1 );

		// 太文字.
		if ( Tag == "b" ) {
			BoldCnt += IsClose ? -1 : 1; if ( BoldCnt < 0 ) BoldCnt = 0;
			Style.IsBold = Base.IsBold || BoldCnt > 0;
			return true;
		}
		// イタリック体.
		if ( Tag == "i" ) {
			ItalicCnt += IsClose ? -1 : 1; if ( ItalicCnt < 0 ) ItalicCnt = 0;
			Style.IsItalic = Base.IsItalic || ItalicCnt > 0;
			return true;
		}
		// 下線.
		if ( Tag == "u" ) {
			UnderCnt += IsClose ? -1 : 1; if ( UnderCnt < 0 ) UnderCnt = 0;
			Style.IsUnderline = Base.IsUnderline || UnderCnt > 0;
			return true;
		}
		// 取り消し線.
		if ( Tag == "s" ) {
			StrikeCnt += IsClose ? -1 : 1; if ( StrikeCnt < 0 ) StrikeCnt = 0;
			Style.IsStrikeout = Base.IsStrikeout || StrikeCnt > 0;
			return true;
		}
		// 文字色.
		if ( Tag == "color" ) {
			if ( IsClose ) {
				Style.Color = ColorStack.empty() ? Base.Color : ColorStack.back();
				if ( ColorStack.empty() == false ) ColorStack.pop_back();
				return true;
			}
			D3DXCOLOR4 NewColor;
			if ( ParseColor( Value, &NewColor ) == false ) return false;
			ColorStack.emplace_back( Style.Color );
			// 全体のフェードを反映できるように基本のアルファを乗算する.
			NewColor.w *= Base.Color.w;
			Style.Color = NewColor;
			return true;
		}
		// アルファ.
		if ( Tag == "alpha" ) {
			if ( IsClose ) {
				Style.Color.w = AlphaStack.empty() ? Base.Color.w : AlphaStack.back();
				if ( AlphaStack.empty() == false ) AlphaStack.pop_back();
				return true;
			}
			float Alpha = 0.0f;
			if ( Value.empty() == false && Value[0] == '#' ) {
				const int Byte = HexToByte( Value, 1 );
				if ( Byte < 0 ) return false;
				Alpha = Byte / 255.0f;
			}
			else {
				if ( ParseFloat( Value, &Alpha ) == false ) return false;
			}
			AlphaStack.emplace_back( Style.Color.w );
			Style.Color.w = Alpha * Base.Color.w;
			return true;
		}
		// 文字サイズ.
		if ( Tag == "size" ) {
			if ( IsClose ) {
				Style.SizeScale = SizeStack.empty() ? Base.SizeScale : SizeStack.back();
				if ( SizeStack.empty() == false ) SizeStack.pop_back();
				return true;
			}
			float Size	= 0.0f;
			bool  IsPer	= false;
			std::string v = Value;
			if ( v.empty() == false && v.back() == '%' ) {
				IsPer = true;
				v.pop_back();
			}
			if ( ParseFloat( v, &Size ) == false ) return false;
			SizeStack.emplace_back( Style.SizeScale );
			// %指定は倍率、数値指定は基準の高さに対するピクセル指定.
			Style.SizeScale = IsPer ? Size / 100.0f : ( BaseCharH > 0.0f ? Size / BaseCharH : 1.0f );
			return true;
		}
		// 行揃え.
		if ( Tag == "align" ) {
			if ( IsClose ) {
				Style.Align = Base.Align;
				return true;
			}
			const std::string v = ToLower( Value );
			if		( v == "left"	) Style.Align = ETextAlign::Left;
			else if ( v == "center" ) Style.Align = ETextAlign::Center;
			else if ( v == "right"	) Style.Align = ETextAlign::Right;
			else return false;
			return true;
		}
		// 追加送り.
		if ( Tag == "space" ) {
			if ( IsClose ) return true;
			float Space = 0.0f;
			if ( ParseFloat( Value, &Space ) == false ) return false;
			PushSpace( Space );
			return true;
		}
		// 字間.
		if ( Tag == "cspace" ) {
			if ( IsClose ) {
				Style.CharSpace = CSpaceStack.empty() ? Base.CharSpace : CSpaceStack.back();
				if ( CSpaceStack.empty() == false ) CSpaceStack.pop_back();
				return true;
			}
			float Space = 0.0f;
			if ( ParseFloat( Value, &Space ) == false ) return false;
			CSpaceStack.emplace_back( Style.CharSpace );
			Style.CharSpace = Space;
			return true;
		}
		// アウトライン.
		if ( Tag == "outline" ) {
			if ( IsClose ) {
				if ( OutLineStack.empty() ) {
					Style.OutLineColor	= Base.OutLineColor;
					Style.OutLineSize	= Base.OutLineSize;
					Style.IsOutLine		= Base.IsOutLine;
				}
				else {
					Style.OutLineColor	= OutLineStack.back().Color;
					Style.OutLineSize	= OutLineStack.back().Size;
					Style.IsOutLine		= OutLineStack.back().IsUse;
					OutLineStack.pop_back();
				}
				return true;
			}
			SOutLineState Old = { Style.OutLineColor, Style.OutLineSize, Style.IsOutLine };
			D3DXCOLOR4	NewColor	= Style.OutLineColor;
			float		NewSize		= Style.OutLineSize;
			// 値は「色」「太さ」「色,太さ」のいずれかを受け付ける.
			if ( Value.empty() == false ) {
				const std::vector<std::string> Values = SplitComma( Value );
				for ( const auto& v : Values ) {
					if ( v.empty() ) continue;
					float Size = 0.0f;
					if		( ParseColor( v, &NewColor )	) continue;
					else if ( ParseFloat( v, &Size )		) NewSize = Size;
					else return false;
				}
			}
			OutLineStack.emplace_back( Old );
			Style.OutLineColor	= NewColor;
			Style.OutLineSize	= NewSize;
			Style.IsOutLine		= true;
			return true;
		}
		// グロー.
		if ( Tag == "glow" ) {
			if ( IsClose ) {
				if ( GlowStack.empty() ) {
					Style.GlowColor	= Base.GlowColor;
					Style.GlowSize	= Base.GlowSize;
					Style.GlowPower	= Base.GlowPower;
					Style.IsGlow	= Base.IsGlow;
				}
				else {
					Style.GlowColor	= GlowStack.back().Color;
					Style.GlowSize	= GlowStack.back().Size;
					Style.GlowPower	= GlowStack.back().Power;
					Style.IsGlow	= GlowStack.back().IsUse;
					GlowStack.pop_back();
				}
				return true;
			}
			SGlowState Old = { Style.GlowColor, Style.GlowSize, Style.GlowPower, Style.IsGlow };
			D3DXCOLOR4	NewColor	= Style.GlowColor;
			float		NewSize		= Style.GlowSize;
			float		NewPower	= Style.GlowPower;
			// 値は「色」「色,幅」「色,幅,減衰」のいずれかを受け付ける.
			if ( Value.empty() == false ) {
				const std::vector<std::string> Values = SplitComma( Value );
				int NumCnt = 0;
				for ( const auto& v : Values ) {
					if ( v.empty() ) continue;
					float Num = 0.0f;
					if ( ParseColor( v, &NewColor ) ) continue;
					if ( ParseFloat( v, &Num ) == false ) return false;
					if ( NumCnt == 0 )	NewSize	 = Num;
					else				NewPower = Num;
					++NumCnt;
				}
			}
			GlowStack.emplace_back( Old );
			Style.GlowColor	= NewColor;
			Style.GlowSize	= NewSize;
			Style.GlowPower	= NewPower;
			Style.IsGlow	= true;
			return true;
		}
		// マーカー(背景色).
		if ( Tag == "mark" ) {
			if ( IsClose ) {
				if ( MarkStack.empty() ) {
					Style.MarkColor	= Base.MarkColor;
					Style.IsMark	= Base.IsMark;
				}
				else {
					Style.MarkColor	= MarkStack.back().Color;
					Style.IsMark	= MarkStack.back().IsUse;
					MarkStack.pop_back();
				}
				return true;
			}
			D3DXCOLOR4 NewColor;
			if ( ParseColor( Value, &NewColor ) == false ) return false;
			MarkStack.emplace_back( SMarkState{ Style.MarkColor, Style.IsMark } );
			Style.MarkColor	= NewColor;
			Style.IsMark	= true;
			return true;
		}
		// 縦方向のオフセット.
		if ( Tag == "voffset" ) {
			if ( IsClose ) {
				Style.VOffset = Base.VOffset;
				return true;
			}
			float Offset = 0.0f;
			if ( ParseFloat( Value, &Offset ) == false ) return false;
			Style.VOffset = Offset;
			return true;
		}
		// 上付き文字/下付き文字.
		if ( Tag == "sup" || Tag == "sub" ) {
			if ( IsClose ) {
				if ( SupSubStack.empty() ) {
					Style.SizeScale	= Base.SizeScale;
					Style.VOffset	= Base.VOffset;
				}
				else {
					Style.SizeScale	= SupSubStack.back().SizeScale;
					Style.VOffset	= SupSubStack.back().VOffset;
					SupSubStack.pop_back();
				}
				return true;
			}
			SupSubStack.emplace_back( SSupSubState{ Style.SizeScale, Style.VOffset } );
			const float Offset = ( Tag == "sup" ) ? SUP_OFFSET : SUB_OFFSET;
			Style.VOffset	+= BaseCharH * Style.SizeScale * Offset;
			Style.SizeScale	*= SUPSUB_SCALE;
			return true;
		}
		// 改行.
		if ( Tag == "br" ) {
			if ( IsClose ) return true;
			PushNewLine();
			return true;
		}
		// タグ解析の無効化.
		if ( Tag == "noparse" ) {
			if ( IsClose == false ) NoParse = true;
			return true;
		}
		return false;
	};

	// 1文字ずつ解析する.
	const int Length = static_cast<int>( Text.length() );
	for ( int i = 0; i < Length; ++i ) {
		// 全角文字はタグとして扱わない.
		if ( IsDBCSLeadByte( Text[i] ) == TRUE ) {
			PushChar( Text.substr( i, 2 ) );
			++i;
			continue;
		}

		const char c = Text[i];

		// 改行.
		if ( c == '\n' ) {
			PushNewLine();
			continue;
		}

		// noparse中は終了タグのみ確認する.
		if ( NoParse ) {
			if ( c == '<' && ToLower( Text.substr( i, 10 ) ) == "</noparse>" ) {
				NoParse = false;
				i += 9;
				continue;
			}
			PushChar( std::string( 1, c ) );
			continue;
		}

		// タグ解析.
		if ( c == '<' && State.IsRichText ) {
			const size_t End = Text.find( '>', i );
			if ( End != std::string::npos && End - i <= TAG_LENGTH_MAX ) {
				const std::string Inner = Text.substr( i + 1, End - i - 1 );
				if ( ApplyTag( Inner ) ) {
					i = static_cast<int>( End );
					continue;
				}
			}
			// 解析できないタグはそのまま文字として表示する.
		}

		PushChar( std::string( 1, c ) );
	}
	return Out;
}

//---------------------------.
// 色文字列の解析.
//---------------------------.
bool CFontTagParser::ParseColor( const std::string& Value, D3DXCOLOR4* pOut )
{
	if ( pOut == nullptr ) return false;

	// 引用符を取り除く.
	std::string v = Value;
	v.erase( std::remove( v.begin(), v.end(), '"' ), v.end() );
	if ( v.empty() ) return false;

	// 16進数指定(#RRGGBB/#RRGGBBAA).
	if ( v[0] == '#' ) {
		const std::string Hex = v.substr( 1 );
		if ( Hex.size() != 6 && Hex.size() != 8 ) return false;
		const int r = HexToByte( Hex, 0 );
		const int g = HexToByte( Hex, 2 );
		const int b = HexToByte( Hex, 4 );
		const int a = ( Hex.size() == 8 ) ? HexToByte( Hex, 6 ) : 255;
		if ( r < 0 || g < 0 || b < 0 || a < 0 ) return false;
		*pOut = D3DXCOLOR4( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );
		return true;
	}

	// 色名指定.
	const std::string Name = ToLower( v );
	for ( const auto& Color : COLOR_NAMES ) {
		if ( Name != Color.pName ) continue;
		*pOut = D3DXCOLOR4( Color.r, Color.g, Color.b, 1.0f );
		return true;
	}
	return false;
}

//---------------------------.
// 数値文字列の解析.
//---------------------------.
bool CFontTagParser::ParseFloat( const std::string& Value, float* pOut )
{
	if ( pOut == nullptr || Value.empty() ) return false;

	char* pEnd = nullptr;
	const float Num = std::strtof( Value.c_str(), &pEnd );
	if ( pEnd == nullptr || *pEnd != '\0' ) return false;

	*pOut = Num;
	return true;
}
#endif // ENABLE_FONT
