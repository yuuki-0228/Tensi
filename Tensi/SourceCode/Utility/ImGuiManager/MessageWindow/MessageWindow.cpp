#include "MessageWindow.h"
#include "..\..\FileManager\FileManager.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace{
	// ���b�Z�[�W�E�B���h�E�̕ۑ����.
	constexpr int	MESSAGE_MAX				= 20;
	// �V�������b�Z�[�W���������̐F�̕ύX����.
	constexpr int	NEW_MESSAGE_COLOR_TIME	= 120;
	// �e�L�X�g�̃p�X.
	constexpr char	TEXT_PATH[]				= "Data\\Parameter\\Config\\MessageWindow.json";

	// �g�p�F�̗񋓑�.
	enum enColor{
		Red,		// �d�v�Ȃ��ƂȂǂɎg�p.
		Green,		// �ʒm�₨���点�ȂǂɎg�p.
		Blue,		// ���g�p.
		Yellow,		// ��ʂ̐ؑւɎg�p.
		Cyan,		// �����֌W�Ŏg�p.
		Magenta,	// �G�f�B�^�Ŏg�p.
		white,		// �f�t�H���g.
		Gray,		// �S�Ẵ��b�Z�[�W�̒ǉ����~.

		Max
	} typedef EColor;
}

MessageWindow::MessageWindow()
	: m_MessageList			()
	, m_ColorBlockFlags		()
	, m_ColorTable			()
	, m_FindString			( "" )
	, m_IsFindTime			( false )
	, m_IsNewMsg			( false )
{
	m_MessageList.reserve( MESSAGE_MAX );
	SetColorTable();
}

MessageWindow::~MessageWindow()
{
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
MessageWindow* MessageWindow::GetInstance()
{
	static std::unique_ptr<MessageWindow> pInstance = std::make_unique<MessageWindow>();
	return pInstance.get();
}

//-----------------------------.
// �`��.
//-----------------------------.
void MessageWindow::Render()
{
	MessageWindow* pI = GetInstance();

	if ( pI->m_IsNewMsg ) {
		// �F��ԐF�ɕύX����.
		ImGui::PushStyleColor( ImGuiCol_TitleBgCollapsed,	ImVec4( 0.3f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TitleBg,			ImVec4( 0.4f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TitleBgActive,		ImVec4( 0.6f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_Tab,				ImVec4( 0.3f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabActive,			ImVec4( 0.4f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabHovered,			ImVec4( 0.6f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabUnfocused,		ImVec4( 0.4f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabUnfocusedActive,	ImVec4( 0.6f, 0.0f, 0.0f, 1.0f ) );
	}
	ImGui::Begin( "MessageWindow" );

	// ���b�Z�[�W�����̕\��.
	MessageFind();

	// �J���[�u���b�N�̕\��.
	ColorBlock();

	// �����̕\��.
	const int QueueSize = static_cast<int>( pI->m_MessageList.size() );
	ImGuiManager::Group( [&]() {
		for ( int i = QueueSize - 1; i >= 0; i-- ) {
			std::function<void()> proc = pI->m_MessageList[i];
			proc();
		}
	} );
	if ( pI->m_IsNewMsg ) {
		// �ύX�����F�̌����������s��.
		for( int i = 0; i < 8; ++i ) 
			ImGui::PopStyleColor();

		// �J�E���g�����炷.
		pI->m_ColorResetCnt--;
		if ( pI->m_ColorResetCnt <= 0 ) {
			pI->m_IsNewMsg = false;
		}
	}
	ImGui::End();
}


//----------------------.
// ���b�Z�[�W�E�B���h�E�ɒǉ�.
//	�S�Ẵ��b�Z�[�W���u���b�N���͏����͍s��Ȃ�.
//----------------------.
void MessageWindow::PushMessage( const std::string& Message, const D3DXCOLOR3& Color )
{
	PushMessage( Message, Color4::RGBA( Color ) );
}
void MessageWindow::PushMessage( const std::string& Message, const D3DXCOLOR4& Color )
{
	MessageWindow* pI = GetInstance();

	if ( pI->m_ColorBlockFlags[Gray] ) return;

	// ���݂̐F�Ƀu���b�N�t���O�������Ă���Ȃ甲����.
	for ( int i = 0; i < EColor::Max; i++ ){
		if ( Color == pI->m_ColorTable[i] &&
			!pI->m_ColorBlockFlags[i] ) return;
	}

	// ������.
	pI->m_IsNewMsg		= true;
	pI->m_ColorResetCnt = NEW_MESSAGE_COLOR_TIME;

	// ���Ԃ̎擾.
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "[%H:%M:%S]" );

	// �e�L�X�g�̍쐬.
	std::string Text;
	Text = Time.str() + " : " + Message;

	// ���b�Z�[�W�̐�������ȏ�Ȃ�Â����b�Z�[�W���폜����.
	if ( pI->m_MessageList.size() >= MESSAGE_MAX )
		pI->m_MessageList.erase( pI->m_MessageList.begin() );

	// �֐����쐬.
	pI->m_MessageList.emplace_back(
	[=]() {
		// �������镶���̎w��.
		std::string FindString;
		if ( pI->m_IsFindTime )	FindString = Text;		// ���Ԃ��܂�.
		else					FindString = Message;	// ���Ԃ��܂܂Ȃ�.

		// ���b�Z�[�W�Ɍ������镶�����܂܂�Ă���ꍇ.
		if ( FindString.find( pI->m_FindString ) != std::string::npos ){
			// �����̕\��.
			ImGui::TextColored( ImGuiManager::ConvertVector4ToIm( Color ), Text.c_str() );
		}
	} );
}

//----------------------.
// ���b�Z�[�W�E�B���h�E.
//----------------------.
void MessageWindow::MessageFind()
{
	MessageWindow* pI = GetInstance();

	ImGui::Text( u8"���� :" );
	ImGui::SameLine();
	ImGui::InputText( "##MsgWnd_InputText", &pI->m_FindString );
	ImGui::SameLine();
	ImGuiManager::CheckBox( u8"##MsgWnd_CheckBox", &pI->m_IsFindTime );
	ImGui::SameLine();
	if ( ImGuiManager::Button( u8"�폜" ) ) pI->m_MessageList.clear();
	ImGui::Separator();
}

//----------------------.
// ���b�Z�[�W�E�B���h�E�̃J���[�u���b�N����.
//----------------------.
void MessageWindow::ColorBlock()
{
	MessageWindow* pI = GetInstance();

	const	std::vector<D3DXCOLOR4> Color = pI->m_ColorTable;
	bool	Flags[EColor::Max];

	// �`�F�b�N�{�b�N�X�̕\��.
	ImGui::Text( u8"�ݒ� : " );
	ImGui::SameLine();
	for ( int i = 0; i < EColor::Max; i++ ){
		Flags[i] = pI->m_ColorBlockFlags[i];
		const std::string& No = std::to_string( i );
		ImGuiManager::CheckBox( No.c_str(), &Flags[i], Color[i] );
		pI->m_ColorBlockFlags[i] = Flags[i];
		ImGui::SameLine();
	}
	ImGuiManager::HelpMarker(
		u8"---------------------------------------------------------------------\n"
		u8"1:�ԐF : �d�v�Ȃ��ƂȂǂɎg�p     / [ �G���[/�x���Ȃ� ]\n"
		u8"2:�ΐF : �ʒm�₨���点�ȂǂɎg�p / [ BGM�̒�~/�{�X�̍s���ω��Ȃ� ]\n"
		u8"3:�F : ���g�p\n"
		u8"4:���F : ��ʂ̐ؑւɎg�p         / [ �V�[���ؑ�/�t�F�[�h�Ȃ� ]\n"
		u8"5:���F : �����֌W�Ŏg�p           / [ BGM/SE�̍Đ� ]\n"
		u8"6:���F : �G�f�B�^�Ŏg�p           / [ UI�G�f�B�^�Ȃ� ]\n"
		u8"7:���F : �f�t�H���g\n"
		u8"8:�D�F : �S�Ẵ��b�Z�[�W�̒ǉ����~\n"
		u8"---------------------------------------------------------------------\n"
	);
	ImGui::Separator();
}

//----------------------.
// �J���[�e�[�u���̐ݒ�.
//----------------------.
void MessageWindow::SetColorTable()
{
	const json& Config = FileManager::JsonLoad( TEXT_PATH );
	m_ColorBlockFlags.resize( EColor::Max, true );
	m_ColorBlockFlags[Red]		= Config["Red"];
	m_ColorBlockFlags[Green]	= Config["Green"];
	m_ColorBlockFlags[Blue]		= Config["Blue"];
	m_ColorBlockFlags[Yellow]	= Config["Yellow"];
	m_ColorBlockFlags[Cyan]		= Config["Cyan"];
	m_ColorBlockFlags[Magenta]	= Config["Magenta"];
	m_ColorBlockFlags[white]	= Config["white"];
	m_ColorBlockFlags[Gray]		= Config["Gray"];

	m_ColorTable = {
		Color4::Red,
		Color4::Green,
		Color4::Blue,
		Color4::Yellow,
		Color4::Cyan,
		Color4::Magenta,
		Color4::White,
		Color4::Gray
	};
}
