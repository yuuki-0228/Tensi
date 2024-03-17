#include "UIEditor.h"
#include "..\..\Common\DirectX\DirectX11.h"
#include "..\..\Object\GameObject\Widget\Widget.h"
#include "..\..\Object\GameObject\Widget\SceneWidget\GameMainWidget\GameMainWidget.h"
#include "..\..\Utility\Input\Input.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"

namespace {
	// �t�@�C���p�X.
	constexpr char	TEXT_PATH[]				= "Data\\Parameter\\Config\\UIEdtor.json";
	constexpr char	UNDO_LOG_FILE_PATH[]	= "Data\\UIEditor\\UndoLog\\";
	constexpr char	REDU_LOG_FILE_PATH[]	= "Data\\UIEditor\\ReduLog\\";

	constexpr int	LOG_FILE_MAX_NONE	= -1;		// ���O�t�@�C���̏����݂��Ȃ�.
	constexpr int	GRID_MIN			= 1;		// �O���b�h���̉���.
	constexpr int	GRID_MAX			= WND_W;	// �O���b�h���̏��.
}

CUIEditor::CUIEditor()
	: m_pUI				( nullptr )
	, m_DispScene		( ESceneList::GameMain )
	, m_UIScene			( ESceneList::GameMain )
	, m_OldBackColor	( DirectX11::GetBackColor() )
	, m_BackColor		( Color4::White )
	, m_HitBoxColor		( Color4::Red )
	, m_GridSpace		( 1 )
	, m_pGrid			( nullptr )
	, m_GridState		()
	, m_SelectUI		( "" )
	, m_UndoLogList		()
	, m_ReduLogList		()
	, m_LogMax			( 100 )
	, m_IsAutoSave		( false )
	, m_IsCreaterLog	( false )
	, m_IsDrag			( false )
	, m_IsDispHitBox	( false )
	, m_IsWidgetUpdate	( false )
	, m_IsFileDelete	( true )
{
}

CUIEditor::~CUIEditor()
{
	// �w�i�̐F�����ɖ߂�.
	DirectX11::SetBackColor( m_OldBackColor );

	// �V�[������ۑ�����.
	m_IsCreaterLog = false;
	if ( m_IsAutoSave ) {
		if ( FAILED( m_pUI->AllSpriteStateDataSave() ) ) return;
		Log::PushLog( "������" + StringConversion::Enum( m_DispScene ) + "�V�[���̃X�v���C�g����S�ĕۑ����܂����B" );
	}

	// �t�@�C���̍폜.
	if ( m_IsFileDelete ) {
		for ( auto& f : m_UndoLogList ) {
			if ( std::remove( f.c_str() ) ) continue;
		}
		for ( auto& f : m_ReduLogList ) {
			if ( std::remove( f.c_str() ) ) continue;
		}
	}
}

//---------------------------.
// ������.
//---------------------------.
bool CUIEditor::Init()
{
	// UI�̍쐬.
	m_pUI = std::make_unique<CGameMainWidget>();
	m_pUI->Init();
	m_pUI->SetIsCreaterLog( &m_IsCreaterLog );
	m_pUI->SetLogList( &m_UndoLogList );

	// �w�i�̃O���b�h���̎擾.
	m_pGrid = SpriteResource::GetSprite( "EdtorGrid" );
	m_GridState = m_pGrid->GetRenderState();

	// �e�L�X�g�̓ǂݍ���.
	const json& Config = FileManager::JsonLoad( TEXT_PATH );
	m_GridState.Color = {
		Config["GridColor"]["R"],
		Config["GridColor"]["G"],
		Config["GridColor"]["B"],
		Config["GridColor"]["A"]
	};
	m_BackColor = {
		Config["BackColor"]["R"],
		Config["BackColor"]["G"],
		Config["BackColor"]["B"],
		Config["BackColor"]["A"]
	};
	m_HitBoxColor = {
		Config["HitBoxColor"]["R"],
		Config["HitBoxColor"]["G"],
		Config["HitBoxColor"]["B"],
		Config["HitBoxColor"]["A"]
	};
	m_GridSpace			= Config["GridSpace"];
	m_IsAutoSave		= Config["IsAutoSave"];
	m_IsCreaterLog		= Config["IsCreaterLog"];
	m_IsDrag			= Config["IsDrag"];
	m_IsDispHitBox		= Config["IsDispHitBox"];
	m_IsFileDelete		= Config["IsFileDelete"];
	m_IsWidgetUpdate	= Config["IsWidgetUpdate"];
	m_LogMax			= Config["LogMax"];

	// ���O�t�@�C���̓ǂݍ���.
	ZLogLoad();
	YLogLoad();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CUIEditor::Update( const float& DeltaTime )
{
	if ( m_IsWidgetUpdate ) m_pUI->Update( DeltaTime );
	m_pUI->UIEdtorUpdate( m_IsDrag, m_IsAutoSave );

	// ���O�t�@�C���̏������.
	const int Size = static_cast<int>( m_UndoLogList.size() );
	if ( m_LogMax != LOG_FILE_MAX_NONE && Size > m_LogMax ) {
		// ����𒴂������Â����O���폜����.
		const int OverSize = Size - m_LogMax;
		for ( int i = 0; i < OverSize; ++i ) {
			std::remove( m_UndoLogList[0].c_str() );
			m_UndoLogList.erase( m_UndoLogList.begin() );
		}
	}

	// �ۑ�.
	if ( KeyInput::IsANDKeyDown( VK_CONTROL, 'S' ) ) Save();

	// ���ɖ߂�.
	Undo();

	// ��蒼��.
	Redu();

	// �w�i�F�̐ݒ�.
	DirectX11::SetBackColor( m_BackColor );
}

//---------------------------.
// ImGui�`��.
//---------------------------.
void CUIEditor::ImGuiRender()
{
	// ImGui�̃E�B���h�E�T�C�Y�ƍ��W��ݒ�.
	ImGui::Begin( "UIEdtor" );
	if ( ImGui::BeginTabBar( "TabBar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_TabListPopupButton ) ) {
		// �V�[���^�u.
		if ( ImGui::BeginTabItem( "Scene" ) ) {
			// �\���V�[����ύX�ł���R���{�{�b�N�X.
			const ESceneList OldScene = m_DispScene;
			ImGuiManager::Combo( u8"�\���V�[���ݒ�", &m_DispScene, { ESceneList::None, ESceneList::Max, ESceneList::UIEdit } );
			ImGui::SameLine();
			// ���݂̃V�[���̃X�v���C�g�̑S�ĕۑ�����{�^��.
			if ( ImGuiManager::Button( "AllSave" ) ) Save();
			ImGui::SameLine();
			// ���Z�b�g�{�^��.
			if ( ImGuiManager::Button( "Reset" ) ) {
				// �ꎞ�I�ɃI�[�g�Z�[�u�𖳌��ɂ���.
				const bool IsAutoSave = m_IsAutoSave;
				if ( IsAutoSave ) m_IsAutoSave = false;

				// �ēǂݍ���.
				ChangeDispScene();
				if ( IsAutoSave ) m_IsAutoSave = true;
			}

			// �\������V�[�����ύX����Ă���ꍇ�X�V����.
			if ( OldScene != m_DispScene ) 
				ChangeDispScene();
			ImGui::Separator();

			// �\�����Ă���UI�̐ݒ�̕\��.
			ImGui::BeginChild( ImGui::GetID( (void*) 0 ), ImVec2( 0.0f, 0.0f ), ImGuiWindowFlags_NoTitleBar );
			m_pUI->UIEdtorImGuiRender( m_IsAutoSave );
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		// UI�ǉ��^�u.
		if ( ImGui::BeginTabItem( "NewUI" ) ) {
			// �ǂݍ���ł�S�Ă�UI�̃R���{�{�b�N�X.
			ImGuiManager::Combo( u8"UI���X�g", &m_SelectUI, SpriteResource::GetSpriteNames() );
			ImGui::SameLine();
			// �X�v���C�g�̒ǉ��{�^��.
			if ( ImGuiManager::Button( "Add" ) ) {
				CSprite* pUI = SpriteResource::GetSprite( m_SelectUI );
				pUI->AddDispSprite( m_IsAutoSave, StringConversion::Enum( m_DispScene ) );
				m_pUI->AddUISpreite( m_SelectUI, pUI );
			}
			ImGui::Separator();
			const SSpriteState& State = SpriteResource::GetSprite( m_SelectUI )->GetSpriteState();
			// �摜���.
			ImGui::Text( u8"�t�@�C���p�X : %s", State.FilePath.c_str() );
			ImGui::Text( u8"���[�J�����W : %d ( %s )", static_cast<int>( State.LocalPosNo ), StringConversion::Enum( State.LocalPosNo ).c_str() );
			ImGui::Text( u8"�摜�T�C�Y   : w = %4.3f, h = %4.3f", State.Base.w, State.Base.h );
			ImGui::Text( u8"�\���T�C�Y   : w = %4.3f, h = %4.3f", State.Disp.w, State.Disp.h );
			ImGui::Text( u8"1�R�}�T�C�Y  : w = %4.3f, h = %4.3f", State.Stride.w, State.Stride.h );
			ImGui::Separator();
			
			// �T���v���摜.
			ImGui::BeginChild( ImGui::GetID( (void*) 0 ), ImVec2( 0.0f, 0.0f ), ImGuiWindowFlags_NoTitleBar );
			const float& n = State.Base.w > 520 ? 520.0f / State.Base.w : 1.0f;
			ImGui::Image( SpriteResource::GetSprite( m_SelectUI )->GetTexture(), ImVec2( State.Base.w * n, State.Base.h * n ) );
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		// �I�v�V�����^�u.
		if ( ImGui::BeginTabItem( "Option" ) ) {
			// �F�̐ݒ�.
			ImGui::Text( u8"�w�i�̐F" );
			ImGui::ColorEdit4( "##BackColor", m_BackColor );
			ImGui::Separator();
			ImGui::Text( u8"�O���b�h���̐F" );
			ImGui::ColorEdit4( "##GridColor", m_GridState.Color );
			ImGui::Separator();
			ImGui::Text( u8"�����蔻��̐F" );
			ImGui::ColorEdit4( "##HitBoxColor", m_HitBoxColor );
			ImGui::Separator();

			// �O���b�h���̊Ԋu�̐ݒ�.
			ImGui::Text( u8"�O���b�h���̊Ԋu" );
			ImGui::SliderInt( "##SliderGridSpace", &m_GridSpace, GRID_MIN, GRID_MAX );
			ImGui::InputInt( "##InputGridSpace", &m_GridSpace );
			m_GridSpace = std::clamp( m_GridSpace, GRID_MIN, GRID_MAX );
			ImGui::Separator();

			// ���O�̏���̐ݒ�.
			ImGui::Text( u8"���O�t�@�C�������" );
			ImGui::InputInt( "##LogMax", &m_LogMax );
			if ( m_LogMax < LOG_FILE_MAX_NONE ) m_LogMax = LOG_FILE_MAX_NONE;
			ImGui::SameLine();
			ImGuiManager::HelpMarker( u8"�u-1�v�̏ꍇ���O�t�@�C���̏���͖����Ƃ��Ĉ����܂��B" );
			ImGui::Separator();

			// ���Z�b�g�����^�C�~���O�Ŏ����ŕۑ����邩�̐ݒ�.
			ImGui::Text( u8"�����ŕۑ������悤�ɂ��邩" );
			ImGuiManager::CheckBox( "##IsAutoSave", &m_IsAutoSave );
			ImGui::Separator();

			// ���O�t�@�C�����쐬���邩�̐ݒ�.
			ImGui::Text( u8"���O�t�@�C�����쐬���邩" );
			ImGuiManager::CheckBox( "##IsCreaterLog", &m_IsCreaterLog );
			ImGui::Separator();

			// �����蔻���\�����邩�̐ݒ�.
			ImGui::Text( u8"�}�E�X�ŉ摜���ړ�������邩" );
			ImGuiManager::CheckBox( "##IsDrag", &m_IsDrag );
			ImGui::Separator();

			// �����蔻���\�����邩�̐ݒ�.
			ImGui::Text( u8"�����蔻���\�����邩" );
			ImGuiManager::CheckBox( "##IsDispHitBox", &m_IsDispHitBox );
			ImGui::Separator();

			// �����蔻���\�����邩�̐ݒ�.
			ImGui::Text( u8"�����Ƃ��Ƀt�@�C�����폜���邩" );
			ImGuiManager::CheckBox( "##IsFileDelete", &m_IsFileDelete );
			ImGui::Separator();

			// UI�̍X�V���s�����̐ݒ�.
			ImGui::Text( u8"UI�̍X�V���s����" );
			ImGuiManager::CheckBox( "##IsWidgetUpdate", &m_IsWidgetUpdate );
			ImGui::SameLine();
			ImGuiManager::HelpMarker( u8"�L���ɂ����ꍇ�A�o�O��\��������܂��B" );
			ImGui::Separator();


			// �ۑ��{�^��.
			if ( ImGuiManager::Button( "Save" ) ) {
				json j;
				j[".Comment"] = {
					u8"UI�G�f�B�^�̏ڍאݒ���s���܂��B",
					u8"----------------------------------------------",
					u8"[BackColor     ] : �w�i�̐F",
					u8"[GridColor     ] : �O���b�h���̐F",
					u8"[GridSpace     ] : �O���b�h���̊Ԋu",
					u8"[HitBoxColor   ] : �����蔻��̐F",
					u8"[IsAutoSave    ] : �����ŕۑ������悤�ɂ��邩",
					u8"[IsCreaterLog  ] : ���O�t�@�C�����쐬���邩",
					u8"[IsDispHitBox  ] : �����蔻���\�����邩",
					u8"[IsDrag        ] : �}�E�X�ŉ摜���ړ��������悤�ɂ��邩",
					u8"[IsFileDelete  ] : �����Ƃ��Ƀt�@�C�����폜���邩",
					u8"[IsWidgetUpdate] : UI�̍X�V���s���悤�ɂ��邩",
					u8"[LogMax        ] : ���O�t�@�C���̏��",
					u8"----------------------------------------------"
				};
				j["BackColor"]["R"]		= m_BackColor.x;
				j["BackColor"]["G"]		= m_BackColor.y;
				j["BackColor"]["B"]		= m_BackColor.z;
				j["BackColor"]["A"]		= m_BackColor.w;
				j["GridColor"]["R"]		= m_GridState.Color.x;
				j["GridColor"]["G"]		= m_GridState.Color.y;
				j["GridColor"]["B"]		= m_GridState.Color.z;
				j["GridColor"]["A"]		= m_GridState.Color.w;
				j["HitBoxColor"]["R"]	= m_HitBoxColor.x;
				j["HitBoxColor"]["G"]	= m_HitBoxColor.y;
				j["HitBoxColor"]["B"]	= m_HitBoxColor.z;
				j["HitBoxColor"]["A"]	= m_HitBoxColor.w;
				j["GridSpace"]			= m_GridSpace;
				j["IsAutoSave"]			= m_IsAutoSave;
				j["IsCreaterLog"]		= m_IsCreaterLog;
				j["IsDrag"]				= m_IsDrag;
				j["IsDispHitBox"]		= m_IsDispHitBox;
				j["IsFileDelete"]		= m_IsFileDelete;
				j["IsWidgetUpdate"]		= m_IsWidgetUpdate;
				j["LogMax"]				= m_LogMax;
				FileManager::JsonSave( TEXT_PATH, j );
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

//---------------------------.
// UI�̕`��.
//---------------------------.
void CUIEditor::UIRender()
{
	const int& DispNum_H = WND_H / m_GridSpace;
	const int& DispNum_W = WND_W / m_GridSpace;

	// �O���b�h���̕`��.
	for ( int i = 0; i <= DispNum_H; ++i ) {
		m_pGrid->RenderUI( &m_GridState );
		m_GridState.Transform.Position.y += m_GridSpace;
	}
	m_GridState.Transform.Rotation.z = Math::ToRadian( -90.0f );
	for ( int i = 0; i <= DispNum_W; ++i ) {
		m_pGrid->RenderUI( &m_GridState );
		m_GridState.Transform.Position.x += m_GridSpace;
	}
	m_GridState.Transform.Position.x = 0.0f;
	m_GridState.Transform.Position.y = 0.0f;
	m_GridState.Transform.Rotation.z = 0.0f;

	// UI�̕`��.
	m_pUI->SetIsDispHitBox( m_IsDispHitBox, m_HitBoxColor );
	m_pUI->Render();
}

//---------------------------.
// �\������V�[���̕ύX.
//---------------------------.
void CUIEditor::ChangeDispScene( const bool IsSaveStop )
{
	// �V�[������ۑ�����.
	if ( !IsSaveStop && m_IsAutoSave ) {
		if ( FAILED( m_pUI->AllSpriteStateDataSave() ) ) return;
		Log::PushLog( "������" + StringConversion::Enum( m_DispScene ) + "�V�[���̃X�v���C�g����S�ĕۑ����܂����B" );
	}

	// �X�v���C�g����ǂݍ��݂Ȃ���.
	m_pUI->AllSpriteStateDataLoad();

	// �V�����V�[���ɕύX����.
	switch ( m_DispScene ) {
	case ESceneList::GameMain:
		m_pUI = std::make_unique<CGameMainWidget>();
		break;
	default: return;
	}
	m_pUI->Init();
	m_pUI->SetIsCreaterLog( &m_IsCreaterLog );
	m_pUI->SetLogList( &m_UndoLogList );
}

//----------------------------.
// ���O�t�@�C���̓ǂݍ���.
//----------------------------.
HRESULT CUIEditor::ZLogLoad()
{
	auto SoundLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// �g���q.
		const std::string FilePath	= Entry.path().string();				// �t�@�C���p�X.
		const std::string FileName	= Entry.path().stem().string();			// �t�@�C����.
		const std::string LoadMsg	= FilePath + " �ǂݍ��� : ����";			// ���炩���߃��[�h�������b�Z�[�W��ݒ肷��.

		// �g���q�� ".json" �ł͂Ȃ��ꍇ�ǂݍ��܂Ȃ�.
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// ���O�t�@�C����ǂݍ���.
		m_UndoLogList.emplace_back( FilePath );

		Log::PushLog( LoadMsg.c_str() );
	};

	Log::PushLog( "------ ���O�t�@�C���ǂݍ��݊J�n -------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( UNDO_LOG_FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SoundLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// �t�@�C����������Ȃ��G���[�͖�������.
		if ( std::string( e.what() ).find( "�w�肳�ꂽ�p�X��������܂���B" ) != std::string::npos ) return S_OK;

		// �G���[���b�Z�[�W��\��.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ ���O�t�@�C���ǂݍ��ݏI�� -------" );
	return S_OK;
}

//----------------------------.
// ���O�t�@�C���̓ǂݍ���.
//----------------------------.
HRESULT CUIEditor::YLogLoad()
{
	auto SoundLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// �g���q.
		const std::string FilePath	= Entry.path().string();				// �t�@�C���p�X.
		const std::string FileName	= Entry.path().stem().string();			// �t�@�C����.
		const std::string LoadMsg	= FilePath + " �ǂݍ��� : ����";			// ���炩���߃��[�h�������b�Z�[�W��ݒ肷��.

		// �g���q�� ".json" �ł͂Ȃ��ꍇ�ǂݍ��܂Ȃ�.
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// ���O�t�@�C����ǂݍ���.
		m_ReduLogList.emplace_back( FilePath );

		Log::PushLog( LoadMsg.c_str() );
	};

	Log::PushLog( "------ ���O�t�@�C���ǂݍ��݊J�n -------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( REDU_LOG_FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SoundLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// �t�@�C����������Ȃ��G���[�͖�������.
		if ( std::string( e.what() ).find( "�w�肳�ꂽ�p�X��������܂���B" ) != std::string::npos ) return S_OK;

		// �G���[���b�Z�[�W��\��.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ ���O�t�@�C���ǂݍ��ݏI�� -------" );
	return S_OK;
}

//----------------------------.
// �ۑ�.
//----------------------------.
void CUIEditor::Save()
{
	if ( FAILED( m_pUI->AllSpriteStateDataSave() ) ) return;
	MessageWindow::PushMessage( u8"�S�ĕۑ����܂����B(Ctrl + S)", Color4::Magenta );
}

//----------------------------.
// ���ɖ߂�.
//----------------------------.
void CUIEditor::Undo()
{
	if ( m_UndoLogList.empty()								 ) return;
	if ( KeyInput::IsANDKeyDown( VK_CONTROL, 'Z' ) == false ) return;

	// ���O�t�@�C���̓ǂݍ���.
	std::string LogFilePath	 = m_UndoLogList.back();
	std::string MoveFilePath = FileManager::JsonLoad( LogFilePath )["FilePath"];

	// ���݂̎��Ԃ��擾.
	auto now	= std::chrono::system_clock::now();
	auto now_c	= std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

	// �t�@�C�������擾.
	std::string FileName = "";
	std::string::size_type Spos = MoveFilePath.rfind( "\\" );
	if ( Spos != std::string::npos ) {
		std::string::size_type Epos = MoveFilePath.find( ".", Spos + 1 );
		if ( Epos != std::string::npos ) {
			FileName = MoveFilePath.substr( Spos + 1, Epos - Spos - 1 );
		}
	}

	// �t�@�C���p�X�̈ړ���̃t�@�C���p�X�̍쐬.
	const std::string LogMoveFilePath = REDU_LOG_FILE_PATH + Time.str() + "_" + FileName + ".json";

	// �t�@�C���̈ړ�.
	FileManager::CreateFileDirectory( REDU_LOG_FILE_PATH );
	if ( std::rename( MoveFilePath.c_str(), LogMoveFilePath.c_str() )	) return;
	if ( std::rename( LogFilePath.c_str(), MoveFilePath.c_str() )		) return;
	m_ReduLogList.emplace_back( LogMoveFilePath );
	m_UndoLogList.emplace_back();

	// �I�[�g�Z�[�u�𖳌��ɂ��ēǂݍ��ݒ���.
	ChangeDispScene( true );
	MessageWindow::PushMessage( u8"���ɖ߂��܂����B(Ctrl + Z)", Color4::Magenta );
}

//----------------------------.
// ��蒼��.
//----------------------------.
void CUIEditor::Redu()
{
	if ( m_ReduLogList.empty()								 ) return;
	if ( KeyInput::IsANDKeyDown( VK_CONTROL, 'Y' ) == false ) return;

	// ���O�t�@�C���̓ǂݍ���.
	std::string LogFilePath	 = m_ReduLogList.back();
	std::string MoveFilePath = FileManager::JsonLoad( LogFilePath )["FilePath"];

	// ���݂̎��Ԃ��擾.
	auto now	= std::chrono::system_clock::now();
	auto now_c	= std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

	// �t�@�C�������擾.
	std::string FileName = "";
	std::string::size_type Spos = MoveFilePath.rfind( "\\" );
	if ( Spos != std::string::npos ) {
		std::string::size_type Epos = MoveFilePath.find( ".", Spos + 1 );
		if ( Epos != std::string::npos ) {
			FileName = MoveFilePath.substr( Spos + 1, Epos - Spos - 1 );
		}
	}

	// �t�@�C���p�X�̈ړ���̃t�@�C���p�X�̍쐬.
	const std::string LogMoveFilePath = UNDO_LOG_FILE_PATH + Time.str() + "_" + FileName + ".json";

	// �t�@�C���̈ړ�.
	FileManager::CreateFileDirectory( UNDO_LOG_FILE_PATH );
	if ( std::rename( MoveFilePath.c_str(), LogMoveFilePath.c_str() )	) return;
	if ( std::rename( LogFilePath.c_str(), MoveFilePath.c_str() )		) return;
	m_UndoLogList.emplace_back( LogMoveFilePath );
	m_ReduLogList.pop_back();

	// �I�[�g�Z�[�u�𖳌��ɂ��ēǂݍ��ݒ���.
	ChangeDispScene( true );
	MessageWindow::PushMessage( u8"��蒼���܂����B(Ctrl + Y)", Color4::Magenta );
}
