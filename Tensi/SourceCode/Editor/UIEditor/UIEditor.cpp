#include "UIEditor.h"
#include "..\..\Common\DirectX\DirectX11.h"
#include "..\..\Object\GameObject\Widget\Widget.h"
#include "..\..\Object\GameObject\Widget\SceneWidget\GameMainWidget\GameMainWidget.h"
#include "..\..\Utility\Input\Input.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"

namespace {
	// ファイルパス.
	constexpr char	TEXT_PATH[]				= "Data\\Parameter\\Config\\UIEdtor.json";
	constexpr char	UNDO_LOG_FILE_PATH[]	= "Data\\UIEditor\\UndoLog\\";
	constexpr char	REDU_LOG_FILE_PATH[]	= "Data\\UIEditor\\ReduLog\\";

	constexpr int	LOG_FILE_MAX_NONE	= -1;		// ログファイルの上限を設けない.
	constexpr int	GRID_MIN			= 1;		// グリッド線の下限.
	constexpr int	GRID_MAX			= WND_W;	// グリッド線の上限.
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
	// 背景の色を元に戻す.
	DirectX11::SetBackColor( m_OldBackColor );

	// シーン情報を保存する.
	m_IsCreaterLog = false;
	if ( m_IsAutoSave ) {
		if ( FAILED( m_pUI->AllSpriteStateDataSave() ) ) return;
		Log::PushLog( "自動で" + StringConversion::Enum( m_DispScene ) + "シーンのスプライト情報を全て保存しました。" );
	}

	// ファイルの削除.
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
// 初期化.
//---------------------------.
bool CUIEditor::Init()
{
	// UIの作成.
	m_pUI = std::make_unique<CGameMainWidget>();
	m_pUI->Init();
	m_pUI->SetIsCreaterLog( &m_IsCreaterLog );
	m_pUI->SetLogList( &m_UndoLogList );

	// 背景のグリッド線の取得.
	m_pGrid = SpriteResource::GetSprite( "EdtorGrid" );
	m_GridState = m_pGrid->GetRenderState();

	// テキストの読み込み.
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

	// ログファイルの読み込み.
	ZLogLoad();
	YLogLoad();
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CUIEditor::Update( const float& DeltaTime )
{
	if ( m_IsWidgetUpdate ) m_pUI->Update( DeltaTime );
	m_pUI->UIEdtorUpdate( m_IsDrag, m_IsAutoSave );

	// ログファイルの上限処理.
	const int Size = static_cast<int>( m_UndoLogList.size() );
	if ( m_LogMax != LOG_FILE_MAX_NONE && Size > m_LogMax ) {
		// 上限を超えた個数古いログを削除する.
		const int OverSize = Size - m_LogMax;
		for ( int i = 0; i < OverSize; ++i ) {
			std::remove( m_UndoLogList[0].c_str() );
			m_UndoLogList.erase( m_UndoLogList.begin() );
		}
	}

	// 保存.
	if ( KeyInput::IsANDKeyDown( VK_CONTROL, 'S' ) ) Save();

	// 元に戻す.
	Undo();

	// やり直し.
	Redu();

	// 背景色の設定.
	DirectX11::SetBackColor( m_BackColor );
}

//---------------------------.
// ImGui描画.
//---------------------------.
void CUIEditor::ImGuiRender()
{
	// ImGuiのウィンドウサイズと座標を設定.
	ImGui::Begin( "UIEdtor" );
	if ( ImGui::BeginTabBar( "TabBar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_TabListPopupButton ) ) {
		// シーンタブ.
		if ( ImGui::BeginTabItem( "Scene" ) ) {
			// 表示シーンを変更できるコンボボックス.
			const ESceneList OldScene = m_DispScene;
			ImGuiManager::Combo( u8"表示シーン設定", &m_DispScene, { ESceneList::None, ESceneList::Max, ESceneList::UIEdit } );
			ImGui::SameLine();
			// 現在のシーンのスプライトの全て保存するボタン.
			if ( ImGuiManager::Button( "AllSave" ) ) Save();
			ImGui::SameLine();
			// リセットボタン.
			if ( ImGuiManager::Button( "Reset" ) ) {
				// 一時的にオートセーブを無効にする.
				const bool IsAutoSave = m_IsAutoSave;
				if ( IsAutoSave ) m_IsAutoSave = false;

				// 再読み込み.
				ChangeDispScene();
				if ( IsAutoSave ) m_IsAutoSave = true;
			}

			// 表示するシーンが変更されている場合更新する.
			if ( OldScene != m_DispScene ) 
				ChangeDispScene();
			ImGui::Separator();

			// 表示しているUIの設定の表示.
			ImGui::BeginChild( ImGui::GetID( (void*) 0 ), ImVec2( 0.0f, 0.0f ), ImGuiWindowFlags_NoTitleBar );
			m_pUI->UIEdtorImGuiRender( m_IsAutoSave );
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		// UI追加タブ.
		if ( ImGui::BeginTabItem( "NewUI" ) ) {
			// 読み込んでる全てのUIのコンボボックス.
			ImGuiManager::Combo( u8"UIリスト", &m_SelectUI, SpriteResource::GetSpriteNames() );
			ImGui::SameLine();
			// スプライトの追加ボタン.
			if ( ImGuiManager::Button( "Add" ) ) {
				CSprite* pUI = SpriteResource::GetSprite( m_SelectUI );
				pUI->AddDispSprite( m_IsAutoSave, StringConversion::Enum( m_DispScene ) );
				m_pUI->AddUISpreite( m_SelectUI, pUI );
			}
			ImGui::Separator();
			const SSpriteState& State = SpriteResource::GetSprite( m_SelectUI )->GetSpriteState();
			// 画像情報.
			ImGui::Text( u8"ファイルパス : %s", State.FilePath.c_str() );
			ImGui::Text( u8"ローカル座標 : %d ( %s )", static_cast<int>( State.LocalPosNo ), StringConversion::Enum( State.LocalPosNo ).c_str() );
			ImGui::Text( u8"画像サイズ   : w = %4.3f, h = %4.3f", State.Base.w, State.Base.h );
			ImGui::Text( u8"表示サイズ   : w = %4.3f, h = %4.3f", State.Disp.w, State.Disp.h );
			ImGui::Text( u8"1コマサイズ  : w = %4.3f, h = %4.3f", State.Stride.w, State.Stride.h );
			ImGui::Separator();
			
			// サンプル画像.
			ImGui::BeginChild( ImGui::GetID( (void*) 0 ), ImVec2( 0.0f, 0.0f ), ImGuiWindowFlags_NoTitleBar );
			const float& n = State.Base.w > 520 ? 520.0f / State.Base.w : 1.0f;
			ImGui::Image( SpriteResource::GetSprite( m_SelectUI )->GetTexture(), ImVec2( State.Base.w * n, State.Base.h * n ) );
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		// オプションタブ.
		if ( ImGui::BeginTabItem( "Option" ) ) {
			// 色の設定.
			ImGui::Text( u8"背景の色" );
			ImGui::ColorEdit4( "##BackColor", m_BackColor );
			ImGui::Separator();
			ImGui::Text( u8"グリッド線の色" );
			ImGui::ColorEdit4( "##GridColor", m_GridState.Color );
			ImGui::Separator();
			ImGui::Text( u8"当たり判定の色" );
			ImGui::ColorEdit4( "##HitBoxColor", m_HitBoxColor );
			ImGui::Separator();

			// グリッド線の間隔の設定.
			ImGui::Text( u8"グリッド線の間隔" );
			ImGui::SliderInt( "##SliderGridSpace", &m_GridSpace, GRID_MIN, GRID_MAX );
			ImGui::InputInt( "##InputGridSpace", &m_GridSpace );
			m_GridSpace = std::clamp( m_GridSpace, GRID_MIN, GRID_MAX );
			ImGui::Separator();

			// ログの上限の設定.
			ImGui::Text( u8"ログファイルを上限" );
			ImGui::InputInt( "##LogMax", &m_LogMax );
			if ( m_LogMax < LOG_FILE_MAX_NONE ) m_LogMax = LOG_FILE_MAX_NONE;
			ImGui::SameLine();
			ImGuiManager::HelpMarker( u8"「-1」の場合ログファイルの上限は無しとして扱います。" );
			ImGui::Separator();

			// リセットされるタイミングで自動で保存するかの設定.
			ImGui::Text( u8"自動で保存されるようにするか" );
			ImGuiManager::CheckBox( "##IsAutoSave", &m_IsAutoSave );
			ImGui::Separator();

			// ログファイルを作成するかの設定.
			ImGui::Text( u8"ログファイルを作成するか" );
			ImGuiManager::CheckBox( "##IsCreaterLog", &m_IsCreaterLog );
			ImGui::Separator();

			// 当たり判定を表示するかの設定.
			ImGui::Text( u8"マウスで画像を移動させれるか" );
			ImGuiManager::CheckBox( "##IsDrag", &m_IsDrag );
			ImGui::Separator();

			// 当たり判定を表示するかの設定.
			ImGui::Text( u8"当たり判定を表示するか" );
			ImGuiManager::CheckBox( "##IsDispHitBox", &m_IsDispHitBox );
			ImGui::Separator();

			// 当たり判定を表示するかの設定.
			ImGui::Text( u8"閉じたときにファイルを削除するか" );
			ImGuiManager::CheckBox( "##IsFileDelete", &m_IsFileDelete );
			ImGui::Separator();

			// UIの更新を行うかの設定.
			ImGui::Text( u8"UIの更新を行うか" );
			ImGuiManager::CheckBox( "##IsWidgetUpdate", &m_IsWidgetUpdate );
			ImGui::SameLine();
			ImGuiManager::HelpMarker( u8"有効にした場合、バグる可能性があります。" );
			ImGui::Separator();


			// 保存ボタン.
			if ( ImGuiManager::Button( "Save" ) ) {
				json j;
				j[".Comment"] = {
					u8"UIエディタの詳細設定を行えます。",
					u8"----------------------------------------------",
					u8"[BackColor     ] : 背景の色",
					u8"[GridColor     ] : グリッド線の色",
					u8"[GridSpace     ] : グリッド線の間隔",
					u8"[HitBoxColor   ] : 当たり判定の色",
					u8"[IsAutoSave    ] : 自動で保存されるようにするか",
					u8"[IsCreaterLog  ] : ログファイルを作成するか",
					u8"[IsDispHitBox  ] : 当たり判定を表示するか",
					u8"[IsDrag        ] : マウスで画像を移動させれるようにするか",
					u8"[IsFileDelete  ] : 閉じたときにファイルを削除するか",
					u8"[IsWidgetUpdate] : UIの更新を行うようにするか",
					u8"[LogMax        ] : ログファイルの上限",
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
// UIの描画.
//---------------------------.
void CUIEditor::UIRender()
{
	const int& DispNum_H = WND_H / m_GridSpace;
	const int& DispNum_W = WND_W / m_GridSpace;

	// グリッド線の描画.
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

	// UIの描画.
	m_pUI->SetIsDispHitBox( m_IsDispHitBox, m_HitBoxColor );
	m_pUI->Render();
}

//---------------------------.
// 表示するシーンの変更.
//---------------------------.
void CUIEditor::ChangeDispScene( const bool IsSaveStop )
{
	// シーン情報を保存する.
	if ( !IsSaveStop && m_IsAutoSave ) {
		if ( FAILED( m_pUI->AllSpriteStateDataSave() ) ) return;
		Log::PushLog( "自動で" + StringConversion::Enum( m_DispScene ) + "シーンのスプライト情報を全て保存しました。" );
	}

	// スプライト情報を読み込みなおす.
	m_pUI->AllSpriteStateDataLoad();

	// 新しいシーンに変更する.
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
// ログファイルの読み込み.
//----------------------------.
HRESULT CUIEditor::ZLogLoad()
{
	auto SoundLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		const std::string FileName	= Entry.path().stem().string();			// ファイル名.
		const std::string LoadMsg	= FilePath + " 読み込み : 成功";			// あらかじめロード完了メッセージを設定する.

		// 拡張子が ".json" ではない場合読み込まない.
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// ログファイルを読み込む.
		m_UndoLogList.emplace_back( FilePath );

		Log::PushLog( LoadMsg.c_str() );
	};

	Log::PushLog( "------ ログファイル読み込み開始 -------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( UNDO_LOG_FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SoundLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// ファイルが見つからないエラーは無視する.
		if ( std::string( e.what() ).find( "指定されたパスが見つかりません。" ) != std::string::npos ) return S_OK;

		// エラーメッセージを表示.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ ログファイル読み込み終了 -------" );
	return S_OK;
}

//----------------------------.
// ログファイルの読み込み.
//----------------------------.
HRESULT CUIEditor::YLogLoad()
{
	auto SoundLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		const std::string FileName	= Entry.path().stem().string();			// ファイル名.
		const std::string LoadMsg	= FilePath + " 読み込み : 成功";			// あらかじめロード完了メッセージを設定する.

		// 拡張子が ".json" ではない場合読み込まない.
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// ログファイルを読み込む.
		m_ReduLogList.emplace_back( FilePath );

		Log::PushLog( LoadMsg.c_str() );
	};

	Log::PushLog( "------ ログファイル読み込み開始 -------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( REDU_LOG_FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SoundLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// ファイルが見つからないエラーは無視する.
		if ( std::string( e.what() ).find( "指定されたパスが見つかりません。" ) != std::string::npos ) return S_OK;

		// エラーメッセージを表示.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ ログファイル読み込み終了 -------" );
	return S_OK;
}

//----------------------------.
// 保存.
//----------------------------.
void CUIEditor::Save()
{
	if ( FAILED( m_pUI->AllSpriteStateDataSave() ) ) return;
	MessageWindow::PushMessage( u8"全て保存しました。(Ctrl + S)", Color4::Magenta );
}

//----------------------------.
// 元に戻す.
//----------------------------.
void CUIEditor::Undo()
{
	if ( m_UndoLogList.empty()								 ) return;
	if ( KeyInput::IsANDKeyDown( VK_CONTROL, 'Z' ) == false ) return;

	// ログファイルの読み込み.
	std::string LogFilePath	 = m_UndoLogList.back();
	std::string MoveFilePath = FileManager::JsonLoad( LogFilePath )["FilePath"];

	// 現在の時間を取得.
	auto now	= std::chrono::system_clock::now();
	auto now_c	= std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

	// ファイル名を取得.
	std::string FileName = "";
	std::string::size_type Spos = MoveFilePath.rfind( "\\" );
	if ( Spos != std::string::npos ) {
		std::string::size_type Epos = MoveFilePath.find( ".", Spos + 1 );
		if ( Epos != std::string::npos ) {
			FileName = MoveFilePath.substr( Spos + 1, Epos - Spos - 1 );
		}
	}

	// ファイルパスの移動先のファイルパスの作成.
	const std::string LogMoveFilePath = REDU_LOG_FILE_PATH + Time.str() + "_" + FileName + ".json";

	// ファイルの移動.
	FileManager::CreateFileDirectory( REDU_LOG_FILE_PATH );
	if ( std::rename( MoveFilePath.c_str(), LogMoveFilePath.c_str() )	) return;
	if ( std::rename( LogFilePath.c_str(), MoveFilePath.c_str() )		) return;
	m_ReduLogList.emplace_back( LogMoveFilePath );
	m_UndoLogList.emplace_back();

	// オートセーブを無効にして読み込み直し.
	ChangeDispScene( true );
	MessageWindow::PushMessage( u8"元に戻しました。(Ctrl + Z)", Color4::Magenta );
}

//----------------------------.
// やり直す.
//----------------------------.
void CUIEditor::Redu()
{
	if ( m_ReduLogList.empty()								 ) return;
	if ( KeyInput::IsANDKeyDown( VK_CONTROL, 'Y' ) == false ) return;

	// ログファイルの読み込み.
	std::string LogFilePath	 = m_ReduLogList.back();
	std::string MoveFilePath = FileManager::JsonLoad( LogFilePath )["FilePath"];

	// 現在の時間を取得.
	auto now	= std::chrono::system_clock::now();
	auto now_c	= std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

	// ファイル名を取得.
	std::string FileName = "";
	std::string::size_type Spos = MoveFilePath.rfind( "\\" );
	if ( Spos != std::string::npos ) {
		std::string::size_type Epos = MoveFilePath.find( ".", Spos + 1 );
		if ( Epos != std::string::npos ) {
			FileName = MoveFilePath.substr( Spos + 1, Epos - Spos - 1 );
		}
	}

	// ファイルパスの移動先のファイルパスの作成.
	const std::string LogMoveFilePath = UNDO_LOG_FILE_PATH + Time.str() + "_" + FileName + ".json";

	// ファイルの移動.
	FileManager::CreateFileDirectory( UNDO_LOG_FILE_PATH );
	if ( std::rename( MoveFilePath.c_str(), LogMoveFilePath.c_str() )	) return;
	if ( std::rename( LogFilePath.c_str(), MoveFilePath.c_str() )		) return;
	m_UndoLogList.emplace_back( LogMoveFilePath );
	m_ReduLogList.pop_back();

	// オートセーブを無効にして読み込み直し.
	ChangeDispScene( true );
	MessageWindow::PushMessage( u8"やり直しました。(Ctrl + Y)", Color4::Magenta );
}
