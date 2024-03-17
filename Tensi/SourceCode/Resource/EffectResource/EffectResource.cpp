#include "EffectResource.h"
#include "..\..\Common\EffectManager\Effect\Effect.h"
#include <encrypt/file.h>

namespace {
	constexpr char FILE_PATH[] = "Data\\Effect"; // エフェクトファイルパス.
}

EffectResource::EffectResource()
	: m_EffectList		()
	, m_EffectNames		()
	, m_Mutex			()
	, m_IsLoadEnd		( false )
{
}

EffectResource::~EffectResource()
{
	m_EffectList.clear();
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
EffectResource* EffectResource::GetInstance()
{
	static std::unique_ptr<EffectResource> pInstance = std::make_unique<EffectResource>();
	return pInstance.get();
}

//---------------------------.
// エフェクトの読み込み.
//---------------------------.
HRESULT EffectResource::EffectLoad()
{
	EffectResource* pI = GetInstance();
	std::unique_lock<std::mutex> Lock( pI->m_Mutex );

	auto EffectLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		std::string		  Extension = Entry.path().extension().string();	// 拡張子.
		std::string		  FileName	= Entry.path().stem().string();			// ファイル名.

#ifndef _DEBUG
// 暗号化前の拡張しを取得.
		Extension = encrypt::GetExtension( FilePath );
#endif

		// 拡張子が ".efk" ではない場合読み込まない.
		if ( Extension != ".efk" && Extension != ".EFK" ) return;

#ifndef _DEBUG
		// 不要な文字を削除s
		FileName.erase( 0, 1 );
		FileName.erase( FileName.length() - 2 );
#endif

		// エフェクトを読み込みリストに格納する.
		pI->m_EffectList[FileName] = std::make_unique<CEffect>( FilePath );
		pI->m_EffectNames.emplace_back( FileName );
	};

	Log::PushLog( "------ エフェクト読み込み開始 ------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, EffectLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ エフェクト読み込み終了 ------" );

	// 読み込み終了.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//---------------------------.
// エフェクト取得関数.
//---------------------------.
CEffect* EffectResource::GetEffect( const std::string& FileName )
{
	// 読み込みが終わっていなかったら null を返す.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// 指定したエフェクトを取得.
	for ( auto& m : GetInstance()->m_EffectList ) {
		if ( m.first == FileName ) return m.second.get();
	}

	ErrorMessage( FileName + " effects not found" );
	return nullptr;
}
