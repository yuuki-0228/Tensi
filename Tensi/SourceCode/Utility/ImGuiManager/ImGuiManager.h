#pragma once
#include "..\..\Global.h"
#include "..\..\Common\SoundManeger\SoundManeger.h"
#include "..\StringConversion\StringConversion.h"
#include <ImGui\imgui.h>
#include <ImGui\imgui_impl_dx11.h>
#include <ImGui\imgui_impl_win32.h>
#include <ImGui\imgui_internal.h>
#include <ImGui\imgui_stdlib.h>
#include <memory>
#include <queue>
#include <functional>
#include <string>

/************************************************
*	Imguiマネージャークラス.
**/
class ImGuiManager final
{
public:
	ImGuiManager();
	~ImGuiManager();

	// 初期化関数.
	static HRESULT Init( HWND hWnd );

	// フレームの設定.
	static void SetingNewFrame();

	// 描画.
	static void Render();

	// 描画関数を追加する.
	static void PushRenderProc( const std::function<void()>& Proc );

	// int型の表示.
	static void SliderInt(			const char* Name, int* i, const int Min, const int Max,		const int Init = 0 );
	static void InputInt(			const char* Name, int* i,									const int Init = 0 );
	static void SliderInputInt(		const char* Name, int* i, const int Min, const int Max,		const int Init = 0 );

	// float型の表示.
	static void SliderFloat(		const char* Name, float* f, const float Min, const float Max,	const float Init = 0.0f );
	static void InputFloat(			const char* Name, float* f,										const float Init = 0.0f );
	static void SliderInputFloat(	const char* Name, float* f, const float Min, const float Max,	const float Init = 0.0f );

	// D3DXVECTOR2型の表示.
	static void SliderVec2(			const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max,	const D3DXVECTOR2& Init = { 0.0f, 0.0f } );
	static void InputVec2(			const char* Name, D3DXVECTOR2* Vec2,									const D3DXVECTOR2& Init = { 0.0f, 0.0f } );
	static void SliderInputVec2(	const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max,	const D3DXVECTOR2& Init = { 0.0f, 0.0f } );

	// D3DXVECTOR3型の表示.
	static void SliderVec3(			const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max,	const D3DXVECTOR3& Init = { 0.0f, 0.0f, 0.0f } );
	static void InputVec3(			const char* Name, D3DXVECTOR3* Vec3,									const D3DXVECTOR3& Init = { 0.0f, 0.0f, 0.0f } );
	static void SliderInputVec3(	const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max,	const D3DXVECTOR3& Init = { 0.0f, 0.0f, 0.0f } );

	// D3DXVECTOR4型の表示.
	static void SliderVec4(			const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max,	const D3DXVECTOR4& Init = { 0.0f, 0.0f, 0.0f, 0.0f } );
	static void InputVec4(			const char* Name, D3DXVECTOR4* Vec4,									const D3DXVECTOR4& Init = { 0.0f, 0.0f, 0.0f, 0.0f } );
	static void SliderInputVec4(	const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max,	const D3DXVECTOR4& Init = { 0.0f, 0.0f, 0.0f, 0.0f } );

	// ヘルプマークの表示.
	static void HelpMarker( const char* Desc );

	// グループ.
	static void Group( const std::function<void()>& Proc, const float Size_y, const float Size_x = INIT_FLOAT );
	static void Group( const std::function<void()>& Proc, const D3DXSCALE2& Size = INIT_FLOAT2 );

	// 条件付きのボタン.
	static bool Button( const char* Label,					D3DXCOLOR4 Color = Color4::None, const D3DXSCALE2& Size = { 0.0f, 0.0f } );
	static bool Button( const char* Label, const bool Flag, D3DXCOLOR4 Color = Color4::None, const D3DXSCALE2& Size = { 0.0f, 0.0f } );

	// チェックボックス.
	static bool CheckBox( const char* Label, bool* v, D3DXCOLOR4 Color = Color4::None );

	// ラジオボタン.
	template<class T>
	static T			RadioButton( const char* Label, T* Out, std::vector<T> SkipEnum = {} );
	static std::string	RadioButton( const char* Label, const std::string& NowItem, const std::vector<std::string>& List );
	static std::string	RadioButton( const char* Label, std::string* Out,			const std::vector<std::string>& List );

	// コンボボックス.
	template<class T>
	static T			Combo( const char* Label, T* Out, std::vector<T> SkipEnum = {} );
	static std::string	Combo( const char* Label, const std::string& NowItem,	const std::vector<std::string>& List );
	static std::string	Combo( const char* Label, std::string* Out,				const std::vector<std::string>& List );

	// ベクター変換関数.
	static ImVec2		ConvertVector2ToIm( const D3DXVECTOR2&	pSrcVec2Dx );
	static D3DXVECTOR2	ConvertVector2ToDx( const ImVec2&		pSrcVec2Im );
	static ImVec4		ConvertVector4ToIm( const D3DXVECTOR4&	pSrcVec4Dx );
	static D3DXVECTOR4	ConvertVector4ToDx( const ImVec4&		pSrcVec4Im );

private:
	// インスタンスの取得.
	static ImGuiManager* GetInstance();

private:
	std::queue<std::function<void()>>	m_RenderQueue;
	bool								m_IsRender;
#ifdef ENABLE_CLASS_BOOL
	CBool								m_IsDispSample;
#endif

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	ImGuiManager( const ImGuiManager & )				= delete;
	ImGuiManager& operator = ( const ImGuiManager & ) = delete;
	ImGuiManager( ImGuiManager && )					= delete;
	ImGuiManager& operator = ( ImGuiManager && )		= delete;
};


//-----------------------------.
// ラジオボタン.
//-----------------------------.
template<class T>
T ImGuiManager::RadioButton( const char* Label, T* Out, std::vector<T> SkipEnum )
{
	int v = static_cast<int>( *Out );
	// ラジオボタンの表示.
	if ( std::string( Label ).find( "##NoName" ) == std::string::npos ) ImGui::Text( Label );
	for ( int i = 0; StringConversion::EnumCheck<T>( i ); i++ ) {
		bool Skip = false;
		for ( auto e : SkipEnum )
			if ( e == static_cast<T>( i ) ) Skip = true;
		// 列挙体の名前のラジオボタンの作成.
		if ( !Skip ) {
			std::string ButName = StringConversion::Enum( static_cast<T>( i ) );
			ImGui::RadioButton( std::string( ButName + "##" + std::to_string( i ) ).c_str(), &v, i);
		}
	}
	if ( *Out != static_cast<T>( v ) ) SoundManager::PlaySE( "ImguiSE" );
	return *Out = static_cast<T>( v );
}

//-----------------------------.
// コンボボックス.
//-----------------------------.
template<class T>
T ImGuiManager::Combo( const char* Label, T* Out, std::vector<T> SkipEnum )
{
	int v = static_cast<int>( *Out );
	std::string Current_Item = StringConversion::Enum( *Out );
	std::vector<std::pair<int,std::string>> Items;

	// コンボボックスの中の文字の作成.
	for ( int i = 0; StringConversion::EnumCheck<T>( i ); i++ ){
		bool Skip = false;
		for ( auto e : SkipEnum )
			if ( e == static_cast<T>( i ) ) Skip = true;
		// 文字を追加.
		if ( !Skip ) {
			std::string Name = StringConversion::Enum( static_cast<T>( i ) );
			Items.emplace_back( std::make_pair( i, Name ) );
		}
	}

	// コンボボックスの表示.
	if ( std::string( Label ).find( "##NoName" ) == std::string::npos ) ImGui::Text( Label );
	if ( ImGui::BeginCombo( std::string( "##" + std::string( Label ) ).c_str(), Current_Item.c_str() ) ) {
		const int Size = static_cast<int>( Items.size() );
		for ( int i = 0; i < Size; i++ ) {
			bool is_selected = ( Current_Item == Items[i].second );
			if ( ImGui::Selectable( Items[i].second.c_str(), is_selected ) ) v = Items[i].first;
			if ( is_selected ) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if ( *Out != static_cast<T>( v ) ) SoundManager::PlaySE( "ImguiSE" );
	return *Out = static_cast<T>( v );
}
