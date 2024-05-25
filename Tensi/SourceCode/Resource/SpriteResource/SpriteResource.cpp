#include "SpriteResource.h"
#include "..\..\Utility\FileManager\FileManager.h"

namespace {
	constexpr char FILE_PATH[] = "Data\\Sprite"; // スプライトファイルパス.
}

SpriteResource::SpriteResource()
	: m_SpriteList		()
	, m_SpriteNames		()
	, m_Mutex			()
	, m_IsLoadEnd		( false )
{
}

SpriteResource::~SpriteResource()
{
	m_SpriteList.clear();
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
SpriteResource* SpriteResource::GetInstance()
{
	static std::unique_ptr<SpriteResource> pInstance = std::make_unique<SpriteResource>();
	return pInstance.get();
}

//---------------------------.
// スプライトの読み込み.
//---------------------------.
HRESULT SpriteResource::SpriteLoad()
{
	SpriteResource* pI = GetInstance();
	std::unique_lock<std::mutex> Lock( pI->m_Mutex );
	
	auto SpriteLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		std::string		  Extension = Entry.path().extension().string();	// 拡張子.
		std::string		  FileName	= Entry.path().stem().string();			// ファイル名.

#ifndef _DEBUG
		// 暗号化前の拡張しを取得.
		Extension = encrypt::GetExtension( FilePath );
#endif
		// 拡張子が ".png" / ".bmp" / ".jpg" ではない場合読み込まない.
		if ( Extension != ".png" && Extension != ".PNG" &&
			 Extension != ".bmp" && Extension != ".BMP" &&
			 Extension != ".jpg" && Extension != ".JPG" ) return;
		
#ifndef _DEBUG
		// 不要な文字を削除
		FileName.erase( 0, 1 );
		FileName.erase( FileName.length() - 2 );
#endif

		// スプライトファイルの読み込み.
		pI->m_SpriteList[FileName] = std::make_unique<CSprite>();
		if ( FAILED( pI->m_SpriteList[FileName]->Init( FilePath ) ) ) throw E_FAIL;
		pI->m_SpriteNames.emplace_back( FileName );
	};

	Log::PushLog( "------ スプライト読み込み開始 ------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SpriteLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// ファイルが見つからないエラーは無視する.
		if ( std::string( e.what() ).find( "指定されたパスが見つかりません。" ) != std::string::npos ) return S_OK;

		// エラーメッセージを表示.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ スプライト読み込み終了 ------" );

	// 読み込み終了.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//---------------------------.
// スプライト取得関数.
//---------------------------.
CSprite* SpriteResource::GetSprite( const std::string& FileName, SSpriteRenderState* pState )
{
	// 読み込みが終わっていなかったら null を返す.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// 指定したエフェクトを取得.
	for ( auto& s : GetInstance()->m_SpriteList ) {
		if ( s.first != FileName	) continue;
		if ( pState  != nullptr		) *pState = s.second->GetRenderState();
		return s.second.get();
	}

	ErrorMessage( FileName + " sounds not found" );
	return nullptr;
}
CSprite* SpriteResource::GetSprite( const std::string& FileName, SSpriteRenderStateList* pState )
{
	CSprite*			Out;
	SSpriteRenderState	State;
	Out = GetSprite( FileName, &State );
	for ( auto& s : *pState ) s = State;
	return Out;
}