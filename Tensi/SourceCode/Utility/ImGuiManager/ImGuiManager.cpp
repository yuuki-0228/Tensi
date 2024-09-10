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
	, m_IsDispSample	( false, u8"ImGui�̃T���v����\��/��\��", "System" )
#endif
{
}

ImGuiManager::~ImGuiManager()
{
	// ImGui�̉��.
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
ImGuiManager* ImGuiManager::GetInstance()
{
	static std::unique_ptr<ImGuiManager> pInstance = std::make_unique<ImGuiManager>();
	return pInstance.get();
}

//----------------------------.
// ������.
//----------------------------.
HRESULT ImGuiManager::Init( HWND hWnd )
{
	IMGUI_CHECKVERSION();
	
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// �t�H���g��ǂݍ���.
	ImFont* font = io.Fonts->AddFontFromFileTTF(
		FONT_FILE_PATH, FONT_SIZE,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese() );

	// �Q�[���p�b�h�̎g�p��������.
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGuiStyle& style = ImGui::GetStyle();

	style.ScaleAllSizes( FONT_SIZE * 0.06f );	// UI�̑傫�����ꊇ�ŕύX�ł��܂��B
	io.FontGlobalScale = FONT_SIZE * 0.06f;		// �t�H���g�̑傫�����ꊇ�ŕύX�ł��܂��B
	ImGui::StyleColorsDark();

	if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
		style.WindowRounding				= 0.0f;
		style.Colors[ImGuiCol_WindowBg].w	= 0.9f;
	}
	
	if ( ImGui_ImplWin32_Init( hWnd ) == false ) return E_FAIL;
	if ( ImGui_ImplDX11_Init( DirectX11::GetDevice(), DirectX11::GetContext() ) == false ) return E_FAIL;

	Log::PushLog( "Imgui�̏����� : ����" );

	return S_OK;
}

//----------------------------.
// �t���[���̐ݒ�.
//----------------------------.
void ImGuiManager::SetingNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

//----------------------------.
// �`��.
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
// �`��֐���ǉ�����.
//	�`�悵�Ă��Ȃ��ꍇ�͏����͍s��Ȃ�.
//----------------------------.
void ImGuiManager::PushRenderProc( const std::function<void()>& Proc )
{
	ImGuiManager* pI = GetInstance();

	if( pI->m_IsRender == false ) return;
	pI->m_RenderQueue.push( Proc );
}

//----------------------------.
// �X���C�_�[��int�^�̕\��.
//----------------------------.
void ImGuiManager::SliderInt( const char* Name, int* i, const int Min, const int Max, const int Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderInt( std::string( "##Sl_" + std::string( Name ) ).c_str(), i, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *i = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X��int�^�̕\��.
//----------------------------.
void ImGuiManager::InputInt( const char* Name, int* i, const int Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �{�b�N�X�̕\��.
		ImGui::InputInt( std::string( "##In_" + std::string( Name ) ).c_str(), i );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *i = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X�ƃX���C�_�[��int�^�̕\��.
//----------------------------.
void ImGuiManager::SliderInputInt( const char* Name, int* i, const int Min, const int Max, const int Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderInt( std::string( "##Sl_" + std::string( Name ) ).c_str(), i, Min, Max );
		// �{�b�N�X�̕\��.
		ImGui::InputInt(  std::string( "##In_" + std::string( Name ) ).c_str(), i );
		*i = std::clamp( *i, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *i = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// �X���C�_�[��float�^�̕\��.
//----------------------------.
void ImGuiManager::SliderFloat( const char* Name, float* f, const float Min, const float Max, const float Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat( std::string( "##Sl_" + std::string( Name ) ).c_str(), f, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *f = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X��float�^�̕\��.
//----------------------------.
void ImGuiManager::InputFloat( const char* Name, float* f, const float Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �{�b�N�X�̕\��.
		ImGui::InputFloat( std::string( "##In_" + std::string( Name ) ).c_str(), f );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *f = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X�ƃX���C�_�[��float�^�̕\��.
//----------------------------.
void ImGuiManager::SliderInputFloat( const char* Name, float* f, const float Min, const float Max, const float Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat( std::string( "##Sl_" + std::string( Name ) ).c_str(), f, Min, Max );
		// �{�b�N�X�̕\��.
		ImGui::InputFloat(  std::string( "##In_" + std::string( Name ) ).c_str(), f );
		*f = std::clamp( *f, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *f = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// �X���C�_�[��D3DXVECTOR2�^�̕\��.
//----------------------------.
void ImGuiManager::SliderVec2( const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max, const D3DXVECTOR2& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat2( std::string( "##Sl2_" + std::string( Name ) ).c_str(), *Vec2, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec2 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X��D3DXVECTOR2�^�̕\��.
//----------------------------.
void ImGuiManager::InputVec2( const char* Name, D3DXVECTOR2* Vec2, const D3DXVECTOR2& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �{�b�N�X�̕\��.
		ImGui::InputFloat2( std::string( "##In2_" + std::string( Name ) ).c_str(), *Vec2 );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec2 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X�ƃX���C�_�[��D3DXVECTOR2�^�̕\��.
//----------------------------.
void ImGuiManager::SliderInputVec2( const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max, const D3DXVECTOR2& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat2( std::string( "##Sl2_" + std::string( Name ) ).c_str(), *Vec2, Min, Max );
		// �{�b�N�X�̕\��.
		ImGui::InputFloat2(  std::string( "##In2_" + std::string( Name ) ).c_str(), *Vec2 );
		Vec2->x = std::clamp( Vec2->x, Min, Max );
		Vec2->y = std::clamp( Vec2->y, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec2 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// �X���C�_�[��D3DXVECTOR3�^�̕\��.
//----------------------------.
void ImGuiManager::SliderVec3( const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max, const D3DXVECTOR3& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat3( std::string( "##Sl3_" + std::string( Name ) ).c_str(), *Vec3, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec3 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X��D3DXVECTOR3�^�̕\��.
//----------------------------.
void ImGuiManager::InputVec3( const char* Name, D3DXVECTOR3* Vec3, const D3DXVECTOR3& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �{�b�N�X�̕\��.
		ImGui::InputFloat3( std::string( "##In3_" + std::string( Name ) ).c_str(), *Vec3 );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec3 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X�ƃX���C�_�[��D3DXVECTOR3�^�̕\��.
//----------------------------.
void ImGuiManager::SliderInputVec3( const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max, const D3DXVECTOR3& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat3( std::string( "##Sl3_" + std::string( Name ) ).c_str(), *Vec3, Min, Max );
		// �{�b�N�X�̕\��.
		ImGui::InputFloat3(  std::string( "##In3_" + std::string( Name ) ).c_str(), *Vec3 );
		Vec3->x = std::clamp( Vec3->x, Min, Max );
		Vec3->y = std::clamp( Vec3->y, Min, Max );
		Vec3->z = std::clamp( Vec3->z, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec3 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// �X���C�_�[��D3DXVECTOR4�^�̕\��.
//----------------------------.
void ImGuiManager::SliderVec4( const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max, const D3DXVECTOR4& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat4( std::string( "##Sl4_" + std::string( Name ) ).c_str(), *Vec4, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec4 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X��D3DXVECTOR4�^�̕\��.
//----------------------------.
void ImGuiManager::InputVec4( const char* Name, D3DXVECTOR4* Vec4, const D3DXVECTOR4& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �{�b�N�X�̕\��.
		ImGui::InputFloat4( std::string( "##In4_" + std::string( Name ) ).c_str(), *Vec4 );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec4 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// ���̓{�b�N�X�ƃX���C�_�[��D3DXVECTOR4�^�̕\��.
//----------------------------.
void ImGuiManager::SliderInputVec4( const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max, const D3DXVECTOR4& Init )
{
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( Name ) ) {
		// �X���C�_�[�̕\��.
		ImGui::SliderFloat4( std::string( "##Sl4_" + std::string( Name ) ).c_str(), *Vec4, Min, Max );
		// �{�b�N�X�̕\��.
		ImGui::InputFloat4(  std::string( "##In4_" + std::string( Name ) ).c_str(), *Vec4 );
		Vec4->x = std::clamp( Vec4->x, Min, Max );
		Vec4->y = std::clamp( Vec4->y, Min, Max );
		Vec4->z = std::clamp( Vec4->z, Min, Max );
		Vec4->w = std::clamp( Vec4->w, Min, Max );

		// �������{�^��.
		if ( ImGuiManager::Button( "Reset" ) ) *Vec4 = Init;
		ImGui::SameLine();
		HelpMarker( u8"������Ԃɖ߂��܂�" );
		ImGui::TreePop();
	}
}

//----------------------------.
// �w���v�}�[�N�̕\��.
//----------------------------.
void ImGuiManager::HelpMarker( const char* Desc )
{
	// �R���g���[���[�ɂ��Ή����������̂�,
	//	�{�^���ŕ\������B���̍ہA�{�^���̐F�����ׂď���.
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
// �O���[�v.
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
// �����t���̃{�^��.
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
		// �����Ȃ���Ԃ̃{�^��.
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
// �`�F�b�N�{�b�N�X.
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
// ���W�I�{�^��.
//----------------------------.
std::string ImGuiManager::RadioButton( const char* Label, const std::string& NowItem, const std::vector<std::string>& List )
{
	std::string Now = NowItem;
	return RadioButton( Label, &Now, List );
}
std::string ImGuiManager::RadioButton( const char* Label, std::string* Out, const std::vector<std::string>& List )
{
	// ���݂̔ԍ����擾.
	const int& Size = static_cast<int>( List.size() );
	int v = 0;
	for ( int i = 0; i < Size; ++i ) {
		if ( List[i] == *Out ) {
			v = i;
			break;
		}
	}

	// ���W�I�{�^���̕\��.
	if( std::string( Label ).find( "##NoName" ) == std::string::npos ) ImGui::Text( Label );
	for ( int i = 0; i < Size; ++i ) {
		// ���W�I�{�^���̍쐬.
		ImGui::RadioButton( std::string( List[i] + "##" + std::string( Label ) + std::to_string( i ) ).c_str(), &v, i);
	}
	if ( *Out != List[v] ) SoundManager::PlaySE( "ImguiSE", 0.0f, false, true );
	return *Out = List[v];
}

//----------------------------.
// �R���{�{�b�N�X.
//----------------------------.
std::string ImGuiManager::Combo( const char* Label, const std::string& NowItem, const std::vector<std::string>& List )
{
	std::string Now = NowItem;
	return Combo( Label, &Now, List );
}
std::string ImGuiManager::Combo( const char* Label, std::string* Out, const std::vector<std::string>& List )
{
	// ���݂̔ԍ����擾.
	const int& Size = static_cast<int>( List.size() );
	int v = 0;
	for ( int i = 0; i < Size; ++i ) {
		if ( List[i] == *Out ) {
			v = i;
			break;
		}
	}

	// �R���{�{�b�N�X�̒��̕����̍쐬.
	std::vector<std::pair<int, std::string>> Items;
	Items.resize( Size );
	for ( int i = 0; i < Size; ++i ) {
		Items[i] = std::make_pair( i, List[i] );
	}

	// �R���{�{�b�N�X�̕\��.
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
// DirectX Vector2 �� ImGui Vector2 �ɕϊ�����.
//----------------------------.
ImVec2 ImGuiManager::ConvertVector2ToIm( const D3DXVECTOR2& pSrcVec2Dx )
{
	return ImVec2( pSrcVec2Dx.x, pSrcVec2Dx.y );
}

//----------------------------.
// ImGui Vector2 �� DirectX Vector2 �ɕϊ�����.
//----------------------------.
D3DXVECTOR2 ImGuiManager::ConvertVector2ToDx( const ImVec2& pSrcVec2Im )
{
	return D3DXVECTOR2( pSrcVec2Im.x, pSrcVec2Im.y );
}

//----------------------------.
// DirectX Vector4 �� ImGui Vector4 �ɕϊ�����.
//----------------------------.
ImVec4 ImGuiManager::ConvertVector4ToIm( const D3DXVECTOR4& pSrcVec4Dx )
{
	return ImVec4( pSrcVec4Dx.x, pSrcVec4Dx.y, pSrcVec4Dx.z, pSrcVec4Dx.w );
}

//----------------------------.
// ImGui Vector4 �� DirectX Vector4 �ɕϊ�����.
//----------------------------.
D3DXVECTOR4 ImGuiManager::ConvertVector4ToDx( const ImVec4& pSrcVec4Im )
{
	return D3DXVECTOR4( pSrcVec4Im.x, pSrcVec4Im.y, pSrcVec4Im.z, pSrcVec4Im.w );
}
