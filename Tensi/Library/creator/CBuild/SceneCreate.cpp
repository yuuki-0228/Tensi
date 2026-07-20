#include "SceneCreate.h"
#include "error.h"
#include "file.h"
#include <algorithm>

namespace {
#ifdef _DEBUG
	const std::string SCENE_LIST_PATH	= "..\\..\\..\\SourceCode\\Scene\\Scenes.h";
	const std::string SCENE_DIR			= "..\\..\\..\\SourceCode\\Scene";
	const std::string SCENE_WIDGET_DIR	= "..\\..\\..\\SourceCode\\Object\\GameObject\\Widget\\SceneWidget";
	const std::string UI_WIDGET_DIR		= "..\\..\\..\\SourceCode\\Object\\GameObject\\Widget\\UIWidget";
#else
	const std::string SCENE_LIST_PATH	= "SourceCode\\Scene\\Scenes.h";
	const std::string SCENE_DIR			= "SourceCode\\Scene";
	const std::string SCENE_WIDGET_DIR	= "SourceCode\\Object\\GameObject\\Widget\\SceneWidget";
	const std::string UI_WIDGET_DIR		= "SourceCode\\Object\\GameObject\\Widget\\UIWidget";
#endif

	// シーンではないため確認から除外するディレクトリ
	const std::vector<std::string> SCENE_DIR_IGNORE = { "FadeManager", "SceneManager" };

	std::vector<std::string> CreatedList;	// 作成したファイル・ディレクトリ
	std::vector<std::string> UnknownList;	// シーンの宣言に存在しないディレクトリ

	// ファイルからシーン名を取得する
	//	「None」と「Max」の間のコメントで囲まれているエリアを参照する
	std::vector<std::string> GetSceneList( const std::vector<std::string>& File ) {
		std::vector<std::string> sceneList;
		bool inArea = false;
		for ( auto& f : File ) {
			// タブ・空白を除いて「,」「/」までを名前として取得
			std::string name = "";
			for ( auto c : f ) {
				if ( c == '\t' || c == ' '	) continue;
				if ( c == ','  || c == '/'	) break;
				name += c;
			}

			if ( name == "None" )	{ inArea = true; continue; }
			if ( name == "Max" )	break;
			if ( inArea == false )	continue;
			if ( name.empty() )		continue;
			sceneList.emplace_back( name );
		}
		return sceneList;
	}

	// ディレクトリがなければ作成する
	void CreateDirectoryIfMissing( const std::string& Path ) {
		if ( std::filesystem::exists( Path ) ) return;
		std::filesystem::create_directories( Path );
		CreatedList.emplace_back( Path );
	}

	// ファイルがなければ作成する
	void CreateFileIfMissing( const std::string& Path, const std::string& Text ) {
		if ( std::filesystem::exists( Path ) ) return;
		FileManager::TextSave( Path, Text );
		CreatedList.emplace_back( Path );
	}

	// シーンの.hファイルの内容の作成
	std::string SceneH( const std::string& Name ) {
		return
			"#pragma once\n"
			"#include \"..\\SceneBase.h\"\n"
			"\n"
			"class C" + Name + "Widget;\n"
			"\n"
			"/************************************************\n"
			"*	" + Name + "シーンクラス.\n"
			"**/\n"
			"class C" + Name + " final\n"
			"	: public CSceneBase\n"
			"{\n"
			"public:\n"
			"	C" + Name + "();\n"
			"	~C" + Name + "();\n"
			"\n"
			"	// 初期化.\n"
			"	virtual bool Init() override;\n"
			"	// 初回起動時の初期化.\n"
			"	virtual bool FirstPlayInit() override;\n"
			"	// 初回ログイン時の初期化.\n"
			"	virtual bool LoginInit( std::tm lastDay ) override;\n"
			"\n"
			"	// 更新.\n"
			"	virtual void Update( const float& DeltaTime ) override;\n"
			"\n"
			"	// モデルの描画.\n"
			"	virtual void ModelRender() override;\n"
			"	// スプライト(UI)の描画.\n"
			"	virtual void SpriteUIRender() override;\n"
			"	// スプライト(3D)/Effectの描画.\n"
			"	//	_A：上に表示される / _B：下に表示される.\n"
			"	virtual void Sprite3DRender_A() override;\n"
			"	virtual void Sprite3DRender_B() override;\n"
			"\n"
			"private:\n"
			"	std::unique_ptr<C" + Name + "Widget>	m_p" + Name + "Widget;	// UI.\n"
			"};\n";
	}

	// シーンの.cppファイルの内容の作成
	std::string SceneCpp( const std::string& Name ) {
		return
			"#include \"" + Name + ".h\"\n"
			"#include \"..\\..\\Object\\GameObject\\Widget\\SceneWidget\\" + Name + "Widget\\" + Name + "Widget.h\"\n"
			"\n"
			"C" + Name + "::C" + Name + "()\n"
			"	: m_p" + Name + "Widget	( nullptr )\n"
			"{\n"
			"}\n"
			"\n"
			"C" + Name + "::~C" + Name + "()\n"
			"{\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// 初期化.\n"
			"//---------------------------.\n"
			"bool C" + Name + "::Init()\n"
			"{\n"
			"	m_p" + Name + "Widget = std::make_unique<C" + Name + "Widget>();\n"
			"	m_p" + Name + "Widget->Init();\n"
			"	return true;\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// 初回起動時の初期化.\n"
			"//---------------------------.\n"
			"bool C" + Name + "::FirstPlayInit()\n"
			"{\n"
			"	return true;\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// 初回ログイン時の初期化.\n"
			"//---------------------------.\n"
			"bool C" + Name + "::LoginInit( std::tm lastDay )\n"
			"{\n"
			"	return false;\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// 更新.\n"
			"//---------------------------.\n"
			"void C" + Name + "::Update( const float& DeltaTime )\n"
			"{\n"
			"	// UIの更新.\n"
			"	m_p" + Name + "Widget->Update( DeltaTime );\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// モデルの描画.\n"
			"//---------------------------.\n"
			"void C" + Name + "::ModelRender()\n"
			"{\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// スプライト(UI)の描画.\n"
			"//---------------------------.\n"
			"void C" + Name + "::SpriteUIRender()\n"
			"{\n"
			"	m_p" + Name + "Widget->Render();\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// スプライト(3D)/Effectの描画.\n"
			"//	_A：上に表示される / _B：下に表示される.\n"
			"//---------------------------.\n"
			"void C" + Name + "::Sprite3DRender_A()\n"
			"{\n"
			"}\n"
			"void C" + Name + "::Sprite3DRender_B()\n"
			"{\n"
			"}\n";
	}

	// シーンUIの.hファイルの内容の作成
	std::string WidgetH( const std::string& Name ) {
		return
			"#pragma once\n"
			"#include \"..\\SceneWidget.h\"\n"
			"\n"
			"/************************************************\n"
			"*	" + Name + "のUIクラス.\n"
			"**/\n"
			"class C" + Name + "Widget final\n"
			"	: public CSceneWidget\n"
			"{\n"
			"public:\n"
			"	C" + Name + "Widget();\n"
			"	virtual ~C" + Name + "Widget();\n"
			"\n"
			"	// 初期化.\n"
			"	virtual bool Init() override;\n"
			"\n"
			"	// 更新.\n"
			"	virtual void Update( const float& DeltaTime ) override;\n"
			"\n"
			"	// 描画.\n"
			"	virtual void Render() override;\n"
			"\n"
			"private:\n"
			"};\n";
	}

	// シーンUIの.cppファイルの内容の作成
	std::string WidgetCpp( const std::string& Name ) {
		return
			"#include \"" + Name + "Widget.h\"\n"
			"\n"
			"C" + Name + "Widget::C" + Name + "Widget()\n"
			"{\n"
			"}\n"
			"\n"
			"C" + Name + "Widget::~C" + Name + "Widget()\n"
			"{\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// 初期化.\n"
			"//---------------------------.\n"
			"bool C" + Name + "Widget::Init()\n"
			"{\n"
			"	m_SceneType = ESceneList::" + Name + ";\n"
			"	return true;\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// 更新.\n"
			"//---------------------------.\n"
			"void C" + Name + "Widget::Update( const float& DeltaTime )\n"
			"{\n"
			"	m_DeltaTime = DeltaTime;\n"
			"}\n"
			"\n"
			"//---------------------------.\n"
			"// 描画.\n"
			"//---------------------------.\n"
			"void C" + Name + "Widget::Render()\n"
			"{\n"
			"}\n";
	}

	// シーンの宣言に存在しないディレクトリの確認
	void CheckUnknownDirectory( const std::vector<std::string>& SceneList ) {
		// シーンの宣言に存在するか
		auto isScene = [&]( const std::string& name ) {
			return std::find( SceneList.begin(), SceneList.end(), name ) != SceneList.end();
		};

		// シーンのディレクトリの確認
		for ( auto& e : std::filesystem::directory_iterator( SCENE_DIR ) ) {
			if ( e.is_directory() == false ) continue;
			const std::string name = e.path().filename().string();
			if ( std::find( SCENE_DIR_IGNORE.begin(), SCENE_DIR_IGNORE.end(), name ) != SCENE_DIR_IGNORE.end() ) continue;
			if ( isScene( name ) ) continue;
			UnknownList.emplace_back( e.path().string() );
		}

		// シーンUIのディレクトリの確認
		//	「シーン名 + Widget」以外を報告する
		for ( auto& e : std::filesystem::directory_iterator( SCENE_WIDGET_DIR ) ) {
			if ( e.is_directory() == false ) continue;
			const std::string name = e.path().filename().string();
			const std::string suffix = "Widget";
			const bool isSceneWidget =
				name.size() > suffix.size() &&
				name.compare( name.size() - suffix.size(), suffix.size(), suffix ) == 0 &&
				isScene( name.substr( 0, name.size() - suffix.size() ) );
			if ( isSceneWidget ) continue;
			UnknownList.emplace_back( e.path().string() );
		}

		// UIWidgetのディレクトリの確認
		for ( auto& e : std::filesystem::directory_iterator( UI_WIDGET_DIR ) ) {
			if ( e.is_directory() == false ) continue;
			const std::string name = e.path().filename().string();
			if ( isScene( name ) ) continue;
			UnknownList.emplace_back( e.path().string() );
		}
	}
}

//-----------------.
// メイン
//-----------------.
int SceneCreate::main()
{
	PrintOutput( "1>Scenes.h\n" );

	auto file = FileManager::TextLoad( SCENE_LIST_PATH );
	if ( file.size() == 0 ) {
		return ErrorMessage( "1>Error : ファイルの読み込み失敗", "Scenes.h" );
	}

	// シーン名の取得
	auto sceneList = GetSceneList( file );

	for ( auto& scene : sceneList ) {
		// シーンのディレクトリ・ファイルの作成
		const std::string sceneDir = SCENE_DIR + "\\" + scene;
		CreateDirectoryIfMissing( sceneDir );
		CreateFileIfMissing( sceneDir + "\\" + scene + ".h",	SceneH( scene )		);
		CreateFileIfMissing( sceneDir + "\\" + scene + ".cpp",	SceneCpp( scene )	);

		// シーンUIのディレクトリ・ファイルの作成
		const std::string widgetDir = SCENE_WIDGET_DIR + "\\" + scene + "Widget";
		CreateDirectoryIfMissing( widgetDir );
		CreateFileIfMissing( widgetDir + "\\" + scene + "Widget.h",		WidgetH( scene )	);
		CreateFileIfMissing( widgetDir + "\\" + scene + "Widget.cpp",	WidgetCpp( scene )	);

		// UIWidgetのディレクトリの作成
		CreateDirectoryIfMissing( UI_WIDGET_DIR + "\\" + scene );
	}

	// シーンの宣言に存在しないディレクトリの確認
	CheckUnknownDirectory( sceneList );
	return S_OK;
}

//-----------------.
// リザルトの表示
//-----------------.
void SceneCreate::result()
{
	// 作成したファイル・ディレクトリの表示
	for ( auto& c : CreatedList ) {
		PrintOutput( "1>Creator -> 作成 : " + std::filesystem::absolute( c ).string() + "\n" );
	}

	// シーンの宣言に存在しないディレクトリの報告
	//	削除などの処理は行わないため対応はユーザーに任せる
	if ( UnknownList.empty() ) return;
	PrintOutput( "1>Creator -> 警告 : シーンの宣言に存在しないディレクトリがあります（削除などはしていません）\n" );
	for ( auto& u : UnknownList ) {
		PrintOutput( "1>Creator ->        " + std::filesystem::absolute( u ).string() + "\n" );
	}
}
