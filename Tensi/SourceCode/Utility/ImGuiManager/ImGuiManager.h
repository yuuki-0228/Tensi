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
*	Imgui�}�l�[�W���[�N���X.
**/
class ImGuiManager final
{
public:
	ImGuiManager();
	~ImGuiManager();

	// �������֐�.
	static HRESULT Init( HWND hWnd );

	// �t���[���̐ݒ�.
	static void SetingNewFrame();

	// �`��.
	static void Render();

	// �`��֐���ǉ�����.
	static void PushRenderProc( const std::function<void()>& Proc );

	// int�^�̕\��.
	static void SliderInt(			const char* Name, int* i, const int Min, const int Max,		const int Init = 0 );
	static void InputInt(			const char* Name, int* i,									const int Init = 0 );
	static void SliderInputInt(		const char* Name, int* i, const int Min, const int Max,		const int Init = 0 );

	// float�^�̕\��.
	static void SliderFloat(		const char* Name, float* f, const float Min, const float Max,	const float Init = 0.0f );
	static void InputFloat(			const char* Name, float* f,										const float Init = 0.0f );
	static void SliderInputFloat(	const char* Name, float* f, const float Min, const float Max,	const float Init = 0.0f );

	// D3DXVECTOR2�^�̕\��.
	static void SliderVec2(			const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max,	const D3DXVECTOR2& Init = { 0.0f, 0.0f } );
	static void InputVec2(			const char* Name, D3DXVECTOR2* Vec2,									const D3DXVECTOR2& Init = { 0.0f, 0.0f } );
	static void SliderInputVec2(	const char* Name, D3DXVECTOR2* Vec2, const float Min, const float Max,	const D3DXVECTOR2& Init = { 0.0f, 0.0f } );

	// D3DXVECTOR3�^�̕\��.
	static void SliderVec3(			const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max,	const D3DXVECTOR3& Init = { 0.0f, 0.0f, 0.0f } );
	static void InputVec3(			const char* Name, D3DXVECTOR3* Vec3,									const D3DXVECTOR3& Init = { 0.0f, 0.0f, 0.0f } );
	static void SliderInputVec3(	const char* Name, D3DXVECTOR3* Vec3, const float Min, const float Max,	const D3DXVECTOR3& Init = { 0.0f, 0.0f, 0.0f } );

	// D3DXVECTOR4�^�̕\��.
	static void SliderVec4(			const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max,	const D3DXVECTOR4& Init = { 0.0f, 0.0f, 0.0f, 0.0f } );
	static void InputVec4(			const char* Name, D3DXVECTOR4* Vec4,									const D3DXVECTOR4& Init = { 0.0f, 0.0f, 0.0f, 0.0f } );
	static void SliderInputVec4(	const char* Name, D3DXVECTOR4* Vec4, const float Min, const float Max,	const D3DXVECTOR4& Init = { 0.0f, 0.0f, 0.0f, 0.0f } );

	// �w���v�}�[�N�̕\��.
	static void HelpMarker( const char* Desc );

	// �O���[�v.
	static void Group( const std::function<void()>& Proc, const float Size_y, const float Size_x = INIT_FLOAT );
	static void Group( const std::function<void()>& Proc, const D3DXSCALE2& Size = INIT_FLOAT2 );

	// �����t���̃{�^��.
	static bool Button( const char* Label,					D3DXCOLOR4 Color = Color4::None, const D3DXSCALE2& Size = { 0.0f, 0.0f } );
	static bool Button( const char* Label, const bool Flag, D3DXCOLOR4 Color = Color4::None, const D3DXSCALE2& Size = { 0.0f, 0.0f } );

	// �`�F�b�N�{�b�N�X.
	static bool CheckBox( const char* Label, bool* v, D3DXCOLOR4 Color = Color4::None );

	// ���W�I�{�^��.
	template<class T>
	static T			RadioButton( const char* Label, T* Out, std::vector<T> SkipEnum = {} );
	static std::string	RadioButton( const char* Label, const std::string& NowItem, const std::vector<std::string>& List );
	static std::string	RadioButton( const char* Label, std::string* Out,			const std::vector<std::string>& List );

	// �R���{�{�b�N�X.
	template<class T>
	static T			Combo( const char* Label, T* Out, std::vector<T> SkipEnum = {} );
	static std::string	Combo( const char* Label, const std::string& NowItem,	const std::vector<std::string>& List );
	static std::string	Combo( const char* Label, std::string* Out,				const std::vector<std::string>& List );

	// �x�N�^�[�ϊ��֐�.
	static ImVec2		ConvertVector2ToIm( const D3DXVECTOR2&	pSrcVec2Dx );
	static D3DXVECTOR2	ConvertVector2ToDx( const ImVec2&		pSrcVec2Im );
	static ImVec4		ConvertVector4ToIm( const D3DXVECTOR4&	pSrcVec4Dx );
	static D3DXVECTOR4	ConvertVector4ToDx( const ImVec4&		pSrcVec4Im );

private:
	// �C���X�^���X�̎擾.
	static ImGuiManager* GetInstance();

private:
	std::queue<std::function<void()>>	m_RenderQueue;
	bool								m_IsRender;
#ifdef ENABLE_CLASS_BOOL
	CBool								m_IsDispSample;
#endif

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	ImGuiManager( const ImGuiManager & )				= delete;
	ImGuiManager& operator = ( const ImGuiManager & ) = delete;
	ImGuiManager( ImGuiManager && )					= delete;
	ImGuiManager& operator = ( ImGuiManager && )		= delete;
};


//-----------------------------.
// ���W�I�{�^��.
//-----------------------------.
template<class T>
T ImGuiManager::RadioButton( const char* Label, T* Out, std::vector<T> SkipEnum )
{
	int v = static_cast<int>( *Out );
	// ���W�I�{�^���̕\��.
	if ( std::string( Label ).find( "##NoName" ) == std::string::npos ) ImGui::Text( Label );
	for ( int i = 0; StringConversion::EnumCheck<T>( i ); i++ ) {
		bool Skip = false;
		for ( auto e : SkipEnum )
			if ( e == static_cast<T>( i ) ) Skip = true;
		// �񋓑̖̂��O�̃��W�I�{�^���̍쐬.
		if ( !Skip ) {
			std::string ButName = StringConversion::Enum( static_cast<T>( i ) );
			ImGui::RadioButton( std::string( ButName + "##" + std::to_string( i ) ).c_str(), &v, i);
		}
	}
	if ( *Out != static_cast<T>( v ) ) SoundManager::PlaySE( "ImguiSE" );
	return *Out = static_cast<T>( v );
}

//-----------------------------.
// �R���{�{�b�N�X.
//-----------------------------.
template<class T>
T ImGuiManager::Combo( const char* Label, T* Out, std::vector<T> SkipEnum )
{
	int v = static_cast<int>( *Out );
	std::string Current_Item = StringConversion::Enum( *Out );
	std::vector<std::pair<int,std::string>> Items;

	// �R���{�{�b�N�X�̒��̕����̍쐬.
	for ( int i = 0; StringConversion::EnumCheck<T>( i ); i++ ){
		bool Skip = false;
		for ( auto e : SkipEnum )
			if ( e == static_cast<T>( i ) ) Skip = true;
		// ������ǉ�.
		if ( !Skip ) {
			std::string Name = StringConversion::Enum( static_cast<T>( i ) );
			Items.emplace_back( std::make_pair( i, Name ) );
		}
	}

	// �R���{�{�b�N�X�̕\��.
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
