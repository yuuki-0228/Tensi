#include "ImGuiManager.h"
#include "DebugWindow\DebugWindow.h"
#include "MessageWindow\MessageWindow.h"
#include "..\Bool\DebugFlagWindow\DebugFlagWindow.h"
#include "..\..\Common\DirectX\DirectX11.h"
#include "..\..\System\SystemWindowManager\SystemWindowManager.h"

namespace
{
	constexpr	char	FONT_FILE_PATH[]	= "Data\\Sprite\\Font\\NasuM.ttf";
	constexpr	float	FONT_SIZE			= 18.0f;
};

ImGuiManager::ImGuiManager()
	: m_RenderQueue		()
#ifdef _DEBUG
	, m_IsRender		( false )
#else	// #ifdef _DEBUG.
	, m_IsRender		( false )
#endif	// #ifdef _DEBUG.
#ifdef ENABLE_CLASS_BOOL
	, m_IsDispSample	( false, u8"ImGuiのサンプルを表示/非表示", "System" )
#endif
{
}

ImGuiManager::~ImGuiManager()
{
	// ImGuiの解放.
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
ImGuiManager* ImGuiManager::GetInstance()
{
	static std::unique_ptr<ImGuiManager> pInstance = std::make_unique<ImGuiManager>();
	return pInstance.get();
}

//----------------------------.
// 初期化.
//----------------------------.
HRESULT ImGuiManager::Init( HWND hWnd )
{
	IMGUI_CHECKVERSION();
	
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// フォントを読み込む.
	ImFont* font = io.Fonts->AddFontFromFileTTF(
		FONT_FILE_PATH, FONT_SIZE,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese() );

	// ゲームパッドの使用を許可する.
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGuiStyle& style = ImGui::GetStyle();

	style.ScaleAllSizes( FONT_SIZE * 0.06f );	// UIの大きさを一括で変更できます。
	io.FontGlobalScale = FONT_SIZE * 0.06f;		// フォントの大きさを一括で変更できます。
	ImGui::StyleColorsDark();

	if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
		style.WindowRounding				= 0.0f;
		style.Colors[ImGuiCol_WindowBg].w	= 0.9f;
	}
	
	if ( ImGui_ImplWin32_Init( hWnd ) == false ) return E_FAIL;
	if ( ImGui_ImplDX11_Init( DirectX11::GetDevice(), DirectX11::GetContext() ) == false ) return E_FAIL;

	Log::PushLog( "Imguiの初期化 : 成功" );

	return S_OK;
}

//----------------------------.
// フレームの設定.
//----------------------------.
void ImGuiManager::SetingNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

//----------------------------.
// 描画.
//----------------------------.
void ImGuiManager::Render()
{
	ImGuiManager* pI = GetInstance();

	if ( pI->m_IsRender == true ) {
		const int queueSize = static_cast<int>( pI->m_RenderQueue.size() );
		for ( int i = 0; i < queueSize; i++ ) {
			std::function<void()> proc = pI->m_RenderQueue.front();
			proc();
			pI->m_RenderQueue.pop();
		}
		SystemWindowManager::Render();
		MessageWindow::Render();
#ifdef ENABLE_CLASS_BOOL
		DebugFlagWindow::Render();
#endif
		DebugWindow::Render();

#ifdef ENABLE_CLASS_BOOL
		if ( pI->m_IsDispSample.get() ) {
			bool IsDispSample = pI->m_IsDispSample.get();
			ImGui::ShowDemoWindow( &IsDispSample );
			pI->m_IsDispSample = IsDispSample;
		}
#endif
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
	ImGuiIO& io = ImGui::GetIO();
	if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

//----------------------------.
// 描画関数を追加する.
//	描画していない場合は処理は行わない.
//----------------------------.
void ImGuiManager::PushRenderProc( const std::function<void()>& Proc )
{
	ImGuiManager* pI = GetInstance();

	if( pI->m_IsRender == false ) return;
	pI->m_RenderQueue.push( Proc );
}

//----------------------------.
// スライダーでint型の表示.
//----------------------------.
void ImGuiManager::SliderInt( const char* Name, int* i, const int Min, const int Max, const int Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderInt( std::string( "##Sl_" + std::string( Name ) ).c_str(), i, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *i = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスでint型の表示.
//----------------------------.
void ImGuiManager::InputInt( const char* Name, int* i, const int Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// ボックスの表示.
		ImGui::InputInt( std::string( "##In_" + std::string( Name ) ).c_str(), i );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *i = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスとスライダーでint型の表示.
//----------------------------.
void ImGuiManager::SliderInputInt( const char* Name, int* i, const int Min, const int Max, const int Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderInt( std::string( "##Sl_" + std::string( Name ) ).c_str(), i, Min, Max );
		// ボックスの表示.
		ImGui::InputInt(  std::string( "##In_" + std::string( Name ) ).c_str(), i );
		*i = std::clamp( *i, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *i = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// スライダーでfloat型の表示.
//----------------------------.
void ImGuiManager::SliderFloat( const char* Name, float* f, const float Min, const float Max, const float Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat( std::string( "##Sl_" + std::string( Name ) ).c_str(), f, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *f = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスでfloat型の表示.
//----------------------------.
void ImGuiManager::InputFloat( const char* Name, float* f, const float Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// ボックスの表示.
		ImGui::InputFloat( std::string( "##In_" + std::string( Name ) ).c_str(), f );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *f = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスとスライダーでfloat型の表示.
//----------------------------.
void ImGuiManager::SliderInputFloat( const char* Name, float* f, const float Min, const float Max, const float Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat( std::string( "##Sl_" + std::string( Name ) ).c_str(), f, Min, Max );
		// ボックスの表示.
		ImGui::InputFloat(  std::string( "##In_" + std::string( Name ) ).c_str(), f );
		*f = std::clamp( *f, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *f = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// スライダーでD3DXVECTOR2型の表示.
//----------------------------.
void ImGuiManager::SliderVec2( const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max, const D3DXVECTOR2& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat2( std::string( "##Sl2_" + std::string( Name ) ).c_str(), *Vec2, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec2 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスでD3DXVECTOR2型の表示.
//----------------------------.
void ImGuiManager::InputVec2( const char* Name, D3DXVECTOR2* Vec2, const D3DXVECTOR2& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// ボックスの表示.
		ImGui::InputFloat2( std::string( "##In2_" + std::string( Name ) ).c_str(), *Vec2 );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec2 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスとスライダーでD3DXVECTOR2型の表示.
//----------------------------.
void ImGuiManager::SliderInputVec2( const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max, const D3DXVECTOR2& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat2( std::string( "##Sl2_" + std::string( Name ) ).c_str(), *Vec2, Min, Max );
		// ボックスの表示.
		ImGui::InputFloat2(  std::string( "##In2_" + std::string( Name ) ).c_str(), *Vec2 );
		Vec2->x = std::clamp( Vec2->x, Min, Max );
		Vec2->y = std::clamp( Vec2->y, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec2 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// スライダーでD3DXVECTOR3型の表示.
//----------------------------.
void ImGuiManager::SliderVec3( const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max, const D3DXVECTOR3& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat3( std::string( "##Sl3_" + std::string( Name ) ).c_str(), *Vec3, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec3 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスでD3DXVECTOR3型の表示.
//----------------------------.
void ImGuiManager::InputVec3( const char* Name, D3DXVECTOR3* Vec3, const D3DXVECTOR3& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// ボックスの表示.
		ImGui::InputFloat3( std::string( "##In3_" + std::string( Name ) ).c_str(), *Vec3 );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec3 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスとスライダーでD3DXVECTOR3型の表示.
//----------------------------.
void ImGuiManager::SliderInputVec3( const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max, const D3DXVECTOR3& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat3( std::string( "##Sl3_" + std::string( Name ) ).c_str(), *Vec3, Min, Max );
		// ボックスの表示.
		ImGui::InputFloat3(  std::string( "##In3_" + std::string( Name ) ).c_str(), *Vec3 );
		Vec3->x = std::clamp( Vec3->x, Min, Max );
		Vec3->y = std::clamp( Vec3->y, Min, Max );
		Vec3->z = std::clamp( Vec3->z, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec3 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// スライダーでD3DXVECTOR4型の表示.
//----------------------------.
void ImGuiManager::SliderVec4( const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max, const D3DXVECTOR4& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat4( std::string( "##Sl4_" + std::string( Name ) ).c_str(), *Vec4, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec4 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスでD3DXVECTOR4型の表示.
//----------------------------.
void ImGuiManager::InputVec4( const char* Name, D3DXVECTOR4* Vec4, const D3DXVECTOR4& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// ボックスの表示.
		ImGui::InputFloat4( std::string( "##In4_" + std::string( Name ) ).c_str(), *Vec4 );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec4 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// 入力ボックスとスライダーでD3DXVECTOR4型の表示.
//----------------------------.
void ImGuiManager::SliderInputVec4( const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max, const D3DXVECTOR4& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// スライダーの表示.
		ImGui::SliderFloat4( std::string( "##Sl4_" + std::string( Name ) ).c_str(), *Vec4, Min, Max );
		// ボックスの表示.
		ImGui::InputFloat4(  std::string( "##In4_" + std::string( Name ) ).c_str(), *Vec4 );
		Vec4->x = std::clamp( Vec4->x, Min, Max );
		Vec4->y = std::clamp( Vec4->y, Min, Max );
		Vec4->z = std::clamp( Vec4->z, Min, Max );
		Vec4->w = std::clamp( Vec4->w, Min, Max );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec4 = Init;
		ImGui::SameLine();
		HelpMarker( u8"初期状態に戻します" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ヘルプマークの表示.
//----------------------------.
void ImGuiManager::HelpMarker( const char* Desc )
{
	// コントローラーにも対応させたいので,
	//	ボタンで表示する。その際、ボタンの色をすべて消す.
	const ImVec4 ImNone = ConvertVector4ToIm( Color4::None );
	ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] );
	ImGui::PushStyleColor( ImGuiCol_Button,			ImNone );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered,	ImNone );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive,	ImNone );
	ImGui::PushID( Desc );
	ImGui::Button( "(?)" );
	ImGui::PopID();
	ImGui::PopStyleColor( 4 );
	if ( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
		ImGui::TextUnformatted( Desc );
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

//----------------------------.
// グループ.
//----------------------------.
void ImGuiManager::Group( const std::function<void()>& Proc, const float Size_y, const float Size_x )
{
	ImGuiManager::Group( Proc, { Size_x, Size_y } );
}
void ImGuiManager::Group( const std::function<void()>& Proc, const D3DXSCALE2& Size )
{
	ImGui::BeginChild( ImGui::GetID( (void*) 0 ), ConvertVector2ToIm( Size ), ImGuiWindowFlags_NoTitleBar );
	Proc();
	ImGui::EndChild();
}

//----------------------------.
// 条件付きのボタン.
//----------------------------.
bool ImGuiManager::Button( const char* Label, D3DXCOLOR4 Color, const D3DXSCALE2& Size )
{
	return ImGuiManager::Button( Label, true, Color, Size );
}
bool ImGuiManager::Button( const char* Label, const bool Flag, D3DXCOLOR4 Color, const D3DXSCALE2& Size )
{
	const ImVec2 ImSize = ConvertVector2ToIm( Size );
	if ( Flag ) {
		if ( Color != Color4::None ) {
			const ImVec4 ImColor = ConvertVector4ToIm( Color );
			ImGui::PushStyleColor( ImGuiCol_Button,			ImVec4( Color.x, Color.y, Color.z, 0.6f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered,	ImVec4( Color.x, Color.y, Color.z, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive,	ImVec4( Color.x, Color.y, Color.z, 0.8f ) );
		}
		const bool Result = ImGui::Button( Label, ImSize );
		if ( Color != Color4::None	) ImGui::PopStyleColor( 3 );
		if ( Result					) SoundManager::PlaySE( "ImguiSE", 0.0f, false, true );
		return Result;
	}
	else {
		// 押せない状態のボタン.
		const ImVec4 ImGray = ConvertVector4ToIm( Color4::Gray );
		ImGui::PushStyleColor( ImGuiCol_Button,			ImGray );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered,	ImGray );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive,	ImGray );
		ImGui::Button( Label, ImSize );
		ImGui::PopStyleColor( 3 );
	}
	return false;
}

//----------------------------.
// チェックボックス.
//----------------------------.
bool ImGuiManager::CheckBox( const char* Label, bool* v, D3DXCOLOR4 Color )
{
	if ( Color != Color4::None ){
		ImGui::PushStyleColor( ImGuiCol_FrameBg,		ImVec4( Color.x, Color.y, Color.z, 0.3f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( Color.x, Color.y, Color.z, 0.5f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive,	ImVec4( Color.x, Color.y, Color.z, 0.4f ) );
		D3DXVECTOR4 CheckColor = Color;
		if ( Color == Color4::Gray || Color == Color::Black )
			CheckColor = Color4::White;
		ImGui::PushStyleColor( ImGuiCol_CheckMark, ConvertVector4ToIm( CheckColor ) );
	}
	const bool Oldv = *v;
	ImGui::Checkbox( Label, v );
	if ( Color	!= Color4::None	) ImGui::PopStyleColor( 4 );
	if ( Oldv	!= *v			)
		SoundManager::PlaySE( "ImguiSE", 0.0f, false, true );
	return *v;
}

//----------------------------.
// ラジオボタン.
//----------------------------.
std::string ImGuiManager::RadioButton( const char* Label, const std::string& NowItem, const std::vector<std::string>& List )
{
	std::string Now = NowItem;
	return RadioButton( Label, &Now, List );
}
std::string ImGuiManager::RadioButton( const char* Label, std::string* Out, const std::vector<std::string>& List )
{
	// 現在の番号を取得.
	const int& Size = static_cast<int>( List.size() );
	int v = 0;
	for ( int i = 0; i < Size; ++i ) {
		if ( List[i] == *Out ) {
			v = i;
			break;
		}
	}

	// ラジオボタンの表示.
	if( std::string( Label ).find( "##NoName" ) == std::string::npos ) ImGui::Text( Label );
	for ( int i = 0; i < Size; ++i ) {
		// ラジオボタンの作成.
		ImGui::RadioButton( std::string( List[i] + "##" + std::string( Label ) + std::to_string( i ) ).c_str(), &v, i);
	}
	if ( *Out != List[v] ) SoundManager::PlaySE( "ImguiSE", 0.0f, false, true );
	return *Out = List[v];
}

//----------------------------.
// コンボボックス.
//----------------------------.
std::string ImGuiManager::Combo( const char* Label, const std::string& NowItem, const std::vector<std::string>& List )
{
	std::string Now = NowItem;
	return Combo( Label, &Now, List );
}
std::string ImGuiManager::Combo( const char* Label, std::string* Out, const std::vector<std::string>& List )
{
	// 現在の番号を取得.
	const int& Size = static_cast<int>( List.size() );
	int v = 0;
	for ( int i = 0; i < Size; ++i ) {
		if ( List[i] == *Out ) {
			v = i;
			break;
		}
	}

	// コンボボックスの中の文字の作成.
	std::vector<std::pair<int, std::string>> Items;
	Items.resize( Size );
	for ( int i = 0; i < Size; ++i ) {
		Items[i] = std::make_pair( i, List[i] );
	}

	// コンボボックスの表示.
	if ( std::string( Label ).find( "##NoName" ) == std::string::npos ) ImGui::Text( Label );
	if ( ImGui::BeginCombo( std::string( "##" + std::string( Label ) ).c_str(), Out->c_str() ) ) {
		const int Size = static_cast<int>( Items.size() );
		for ( int i = 0; i < Size; i++ ) {
			bool is_selected = ( *Out == Items[i].second );
			if ( ImGui::Selectable( Items[i].second.c_str(), is_selected ) ) v = Items[i].first;
			if ( is_selected ) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if ( *Out != Items[v].second ) SoundManager::PlaySE( "ImguiSE", 0.0f, false, true );
	return *Out = Items[v].second;
}

//----------------------------.
// DirectX Vector2 を ImGui Vector2 に変換する.
//----------------------------.
ImVec2 ImGuiManager::ConvertVector2ToIm( const D3DXVECTOR2& pSrcVec2Dx )
{
	return ImVec2( pSrcVec2Dx.x, pSrcVec2Dx.y );
}

//----------------------------.
// ImGui Vector2 を DirectX Vector2 に変換する.
//----------------------------.
D3DXVECTOR2 ImGuiManager::ConvertVector2ToDx( const ImVec2& pSrcVec2Im )
{
	return D3DXVECTOR2( pSrcVec2Im.x, pSrcVec2Im.y );
}

//----------------------------.
// DirectX Vector4 を ImGui Vector4 に変換する.
//----------------------------.
ImVec4 ImGuiManager::ConvertVector4ToIm( const D3DXVECTOR4& pSrcVec4Dx )
{
	return ImVec4( pSrcVec4Dx.x, pSrcVec4Dx.y, pSrcVec4Dx.z, pSrcVec4Dx.w );
}

//----------------------------.
// ImGui Vector4 を DirectX Vector4 に変換する.
//----------------------------.
D3DXVECTOR4 ImGuiManager::ConvertVector4ToDx( const ImVec4& pSrcVec4Im )
{
	return D3DXVECTOR4( pSrcVec4Im.x, pSrcVec4Im.y, pSrcVec4Im.z, pSrcVec4Im.w );
}
