#include "SoundManeger.h"
#include "Sound\Sound.h"
#include "..\..\Object\Object.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"

namespace {
	// サウンドファイルパス.
	constexpr char FILE_PATH[] = "Data\\Sound";

	// 3Dサウンドで使用する音量を下げていく距離.
	constexpr float VOLUME_DOWN_LENGTH = 0.1f;

	// 通常時の音量.
	constexpr int NORMAL_VOLUME = 1000;
}

SoundManager::SoundManager()
	: m_pEnumerator		( NULL )
	, m_pDevice			( NULL )
	, m_pMeterInfo		( NULL )
	, m_pListener		( nullptr )
	, m_BGMList			()
	, m_SEList			()
	, m_BGMNames		()
	, m_SENames			()
	, m_NowBGMName		( "" )
	, m_Mutex			()
	, m_IsLoadEnd		( false )
	, m_LoopStopFlag	( false )
	, m_SelectNo		( -1 )
{
}

SoundManager::~SoundManager()
{
	m_BGMList.clear();
	m_SEList.clear();

	SAFE_RELEASE( m_pEnumerator );
	SAFE_RELEASE( m_pDevice		);
	SAFE_RELEASE( m_pMeterInfo	);
	CoUninitialize();
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
SoundManager* SoundManager::GetInstance()
{
	static std::unique_ptr<SoundManager> pInstance = std::make_unique<SoundManager>();
	return pInstance.get();
}

//----------------------------.
// オーディオインターフェースの構築.
//----------------------------.
HRESULT SoundManager::Create()
{
	SoundManager* pI = GetInstance();

	if ( FAILED( CoInitialize( NULL ) ) ) return E_FAIL;
	if ( FAILED( CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, __uuidof( IMMDeviceEnumerator ), ( void** )&pI->m_pEnumerator ) ) ) return E_FAIL;
	pI->m_pEnumerator->GetDefaultAudioEndpoint( eRender, eConsole, &pI->m_pDevice );
	pI->m_pDevice->Activate( __uuidof( IAudioMeterInformation ), CLSCTX_ALL, NULL, ( void** )&pI->m_pMeterInfo );
	return S_OK;
}

//----------------------------.
// サウンドの読み込み.
//----------------------------.
HRESULT SoundManager::SoundLoad( HWND hWnd )
{
	SoundManager* pI = GetInstance();
	std::unique_lock<std::mutex> Lock( pI->m_Mutex );

	auto SoundLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		const std::string FileName	= Entry.path().stem().string();			// ファイル名.
		const std::string LoadMsg	= FilePath + " 読み込み : 成功";			// あらかじめロード完了メッセージを設定する.

		// 拡張子が ".midi" / ".mp3" / ".wav" ではない場合読み込まない.
		if ( Extension != ".midi" && Extension != ".MIDI" &&
			 Extension != ".mp3"  && Extension != ".MP3"  &&
			 Extension != ".wav"  && Extension != ".WAV" ) return;

		// 同じ名前のjsonファイルの読み込み.
		std::string TextPath = FilePath;
		TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

#ifndef _DEBUG
		// 暗号化されているため元のパスに戻す
		auto dp = TextPath.rfind( "." );
		auto fp = TextPath.rfind( "\\" ) + 1;
		if ( dp != std::string::npos && fp != std::string::npos ) {
			std::string n = TextPath.substr( fp, dp - fp );
			n.erase( 0, 1 );
			n.erase( n.length() - 2 );
			TextPath.erase( fp, dp - fp );
			TextPath.insert( fp, n );
		}
#endif
		json j = FileManager::JsonLoad( TextPath );
		// jsonファイルの作成.
		if ( j.empty() ){
			Log::PushLog( TextPath + " が無いため作成します。" );
			j[".Comment"]	= {
				"----------------------------------------------",
				u8"[MaxVolume] : 最大の音量( 0 ~ 1000 )",
				u8"[PlayMax]   : 多重再生可能な数",
				u8"[SiteName]  : ダウンロードしてきたサイト名",
				u8"[SiteURL]   : ダウンロードしてきたサイトのURL",
				"----------------------------------------------"
			};
			j["MaxVolume"]	= 1000;
			j["PlayMax"]	= 1;
			j["SiteName"]	= "";
			j["SiteURL"]	= "";
			FileManager::JsonSave( TextPath, j );
		}

		// ファイルパスに "BGM" がないためSEとして保存.
		const std::wstring wFilePath = StringConversion::to_wString( FilePath );
		if ( FilePath.find( "BGM" ) == std::string::npos ) {
			// サウンドファイルの読み込み.
			OpenSound( &pI->m_SEList[FileName], hWnd, j["PlayMax"], j["MaxVolume"], FileName, wFilePath);
			pI->m_SENames.emplace_back( FileName );
		}
		// ファイルパスに "BGM" があるためBGMとして保存.
		else{
			// サウンドファイルの読み込み.
			OpenSound( &pI->m_BGMList[FileName], hWnd, j["PlayMax"], j["MaxVolume"], FileName, wFilePath );
			pI->m_BGMNames.emplace_back( FileName );
		}
		Log::PushLog( LoadMsg.c_str() );
	};

	Log::PushLog( "------ サウンド読み込み開始 -------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SoundLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ サウンド読み込み終了 -------" );

	// 読み込み終了.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//----------------------------.
// ピーク地点の取得.
//----------------------------.
float SoundManager::GetPeakValue()
{
	float Value;
	GetInstance()->m_pMeterInfo->GetPeakValue( &Value );
	return Value;
}

//----------------------------.
// SEを再生する.
//	指定した間隔より再生間隔が短い場合は処理は行わない.
//----------------------------.
void SoundManager::PlaySE( std::string Name, float Interval, bool IsStopPlay, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	// 指定した間隔(ミリ秒に変換)と、現在の時刻と前に再生した時刻の間隔を比較する.
	if ( Interval * 1000 > static_cast<float>( GetTickCount64() - pI->m_PlayTimeList[Name] ) ) return;

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		// SEの再生.
		pI->m_SEList[Name][No]->ResetVolume();
		pI->m_SEList[Name][No]->PlaySE( IsStopPlay );
		pI->m_PlayTimeList[Name] = GetTickCount64();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を再生しました", Color4::Cyan );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		if ( IsStopPlay == false && s->IsStopped() == false ) continue;

		// SEの再生.
		s->ResetVolume();
		s->PlaySE( IsStopPlay );
		pI->m_PlayTimeList[Name] = GetTickCount64();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を再生しました", Color4::Cyan );
		return;
	}

	if ( IsStopPlay ) return;
	pI->m_SEList[Name][0]->ResetVolume();
	pI->m_SEList[Name][0]->PlaySE( IsStopPlay );
	pI->m_PlayTimeList[Name] = GetTickCount64();
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を再生しました", Color4::Cyan );
}

//----------------------------.
// SEを再生する.
//	ループして再生する.
//----------------------------.
void SoundManager::PlaySELoop( std::string Name, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		// SEを再生する.
		pI->m_SEList[Name][No]->ResetVolume();
		if ( pI->m_SEList[Name][No]->PlayLoop() == false ) return;
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」をループさせました", Color4::Green );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ) {
		if ( s->IsStopped() == false ) continue;

		// SEを再生する.
		s->ResetVolume();
		if ( s->PlayLoop()	== false ) return;
		if ( IsMsgStop		== false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」をループさせました", Color4::Green );
		break;
	}
}

//----------------------------.
// 3DでSEを再生する.
//	指定した間隔より再生間隔が短い場合は処理は行わない.
//----------------------------.
void SoundManager::PlaySE3D( std::string Name, D3DXPOSITION3 PlayPos, float Interval, bool IsStopPlay, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	// 指定した間隔(ミリ秒に変換)と、現在の時刻と前に再生した時刻の間隔を比較する.
	if ( Interval * 1000 > static_cast<float>( GetTickCount64() - pI->m_PlayTimeList[Name] ) ) return;

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		// 再生位置とリスナーの距離を調べる.
		const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
		const float Length = D3DXVec3Length( &ListenerVec );

		// 離れるごとに音量を下げていく.
		const int MaxVolume = pI->m_SEList[Name][No]->GetMaxVolume();
		pI->m_SEList[Name][No]->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

		// SEの再生.
		pI->m_SEList[Name][No]->PlaySE( IsStopPlay );
		pI->m_PlayTimeList[Name] = GetTickCount64();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を3Dで再生しました", Color4::Cyan );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		if ( IsStopPlay == false && s->IsStopped() == false ) continue;

		// 再生位置とリスナーの距離を調べる.
		const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
		const float Length = D3DXVec3Length( &ListenerVec );

		// 離れるごとに音量を下げていく.
		const int MaxVolume = s->GetMaxVolume();
		s->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

		// SEの再生.
		s->PlaySE( IsStopPlay );
		pI->m_PlayTimeList[Name] = GetTickCount64();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を3Dで再生しました", Color4::Cyan );
		break;
	}

	if ( IsStopPlay ) return;
	const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
	const float Length = D3DXVec3Length( &ListenerVec );

	// 離れるごとに音量を下げていく.
	const int MaxVolume = pI->m_SEList[Name][0]->GetMaxVolume();
	pI->m_SEList[Name][0]->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

	// SEの再生.
	pI->m_SEList[Name][0]->PlaySE( IsStopPlay );
	pI->m_PlayTimeList[Name] = GetTickCount64();
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を3Dで再生しました", Color4::Cyan );
}

//----------------------------.
// 3DでSEを再生する.
//	ループして指定した間隔より再生間隔が短い場合は処理は行わない.
//----------------------------.
void SoundManager::PlaySELoop3D( std::string Name, D3DXPOSITION3 PlayPos, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 再生位置とリスナーの距離を調べる.
	const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
	const float Length = D3DXVec3Length( &ListenerVec );

	// 離れるごとに音量を下げていく.
	const int MaxVolume = pI->m_SEList[Name][0]->GetMaxVolume();
	pI->m_SEList[Name][0]->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

	// サウンドの再生.
	if ( pI->m_SEList[Name][0]->PlayLoop() == false ) return;
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」をループさせました", Color4::Green );
}

//----------------------------.
// BGMを再生する.
//	ループ停止中の場合処理は行わない.
//----------------------------.
void SoundManager::PlayBGM( std::string Name, bool OldBGMStop, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 今鳴っているBGMを止める.
	if ( OldBGMStop && pI->m_NowBGMName != "" && pI->m_NowBGMName != Name ) {
		pI->m_BGMList[pI->m_NowBGMName][0]->Stop();
	}

	// BGMを再生する.
	pI->m_BGMList[Name][0]->ResetVolume();
	if ( pI->m_BGMList[Name][0]->PlayLoop() == false ) return;
	if ( OldBGMStop ) pI->m_NowBGMName = Name;
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」をループさせました", Color4::Green );
}

//----------------------------.
// BGMをフェードで切り替えて再生する.
//	ループ停止中の場合処理は行わない.
//----------------------------.
void SoundManager::PlayFadeBGM( std::string Name, int FadeSpeed, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 今鳴っているBGMを徐々に止める.
	if ( pI->m_NowBGMName != "" && pI->m_NowBGMName != Name ) {
		pI->m_BGMList[pI->m_NowBGMName][0]->AddVolume( -FadeSpeed );
		pI->m_BGMList[Name][0]->SetVolume( 
			pI->m_BGMList[Name][0]->GetMaxVolume() - pI->m_BGMList[Name][0]->GetMaxVolume() * (
			pI->m_BGMList[pI->m_NowBGMName][0]->GetVolume() / pI->m_BGMList[pI->m_NowBGMName][0]->GetMaxVolume() 
		) );

		// BGMを止める.
		if ( pI->m_BGMList[pI->m_NowBGMName][0]->GetVolume() <= 0 ) {
			pI->m_BGMList[pI->m_NowBGMName][0]->ResetVolume();
			pI->m_BGMList[pI->m_NowBGMName][0]->Stop();
			pI->m_BGMList[Name][0]->ResetVolume();
			pI->m_NowBGMName = Name;
		}
	}

	// BGMを再生する.
	if ( pI->m_BGMList[Name][0]->PlayLoop() == false ) return;
	if ( pI->m_NowBGMName	== ""		) pI->m_NowBGMName = Name;
	if ( IsMsgStop			== false	) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」をループさせました", Color4::Green );
}

//----------------------------.
// 3DでBGMを再生する.
//	ループ停止中の場合処理は行わない.
//----------------------------.
void SoundManager::PlayBGM3D( std::string Name, D3DXPOSITION3 PlayPos, bool OldBGMStop, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 今鳴っているBGMを止める.
	if ( OldBGMStop && pI->m_NowBGMName != "" && pI->m_NowBGMName != Name ) pI->m_BGMList[pI->m_NowBGMName][0]->Stop();

	// 再生位置とリスナーの距離を調べる.
	const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
	const float Length = D3DXVec3Length( &ListenerVec );

	// 離れるごとに音量を下げていく.
	const int MaxVolume = pI->m_BGMList[Name][0]->GetMaxVolume();
	pI->m_BGMList[Name][0]->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

	// サウンドの再生.
	if ( pI->m_BGMList[Name][0]->PlayLoop() == false ) return;
	if ( OldBGMStop ) pI->m_NowBGMName = Name;
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」をループさせました", Color4::Green );
}

//----------------------------.
// SEを逆再生する.
//	指定した間隔より再生間隔が短い場合は処理は行わない.
//----------------------------.
void SoundManager::ReversePlaySE( std::string Name, float Interval, bool IsStopPlay, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	// 指定した間隔(ミリ秒に変換)と、現在の時刻と前に再生した時刻の間隔を比較する.
	if ( Interval * 1000 > static_cast<float>( GetTickCount64() - pI->m_PlayTimeList[Name] ) ) return;

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		// SEの再生.
		pI->m_SEList[Name][No]->ResetVolume();
		pI->m_SEList[Name][No]->PlaySE( IsStopPlay, true );
		pI->m_PlayTimeList[Name] = GetTickCount64();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を逆再生しました", Color4::Cyan );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		if ( IsStopPlay == false && s->IsStopped() == false ) continue;

		// SEの再生.
		s->ResetVolume();
		s->PlaySE( IsStopPlay, true );
		pI->m_PlayTimeList[Name] = GetTickCount64();

		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を逆再生しました", Color4::Cyan );
		break;
	}

	if ( IsStopPlay ) return;
	pI->m_SEList[Name][0]->ResetVolume();
	pI->m_SEList[Name][0]->PlaySE( IsStopPlay, true );
	pI->m_PlayTimeList[Name] = GetTickCount64();

	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を逆再生しました", Color4::Cyan );
}

//----------------------------.
// 3DでSEを逆再生する.
//	指定した間隔より再生間隔が短い場合は処理は行わない.
//----------------------------.
void SoundManager::ReversePlaySE3D( std::string Name, D3DXPOSITION3 PlayPos, float Interval, bool IsStopPlay, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	// 指定した間隔(ミリ秒に変換)と、現在の時刻と前に再生した時刻の間隔を比較する.
	if ( Interval * 1000 > static_cast<float>( GetTickCount64() - pI->m_PlayTimeList[Name] ) ) return;

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		// 再生位置とリスナーの距離を調べる.
		const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
		const float Length = D3DXVec3Length( &ListenerVec );

		// 離れるごとに音量を下げていく.
		const int MaxVolume = pI->m_SEList[Name][No]->GetMaxVolume();
		pI->m_SEList[Name][No]->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

		// SEの再生.
		pI->m_SEList[Name][No]->PlaySE( IsStopPlay, true );
		pI->m_PlayTimeList[Name] = GetTickCount64();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を3Dで逆再生しました", Color4::Cyan);
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		if ( IsStopPlay == false && s->IsStopped() == false ) continue;

		// 再生位置とリスナーの距離を調べる.
		const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
		const float Length = D3DXVec3Length( &ListenerVec );

		// 離れるごとに音量を下げていく.
		const int MaxVolume = s->GetMaxVolume();
		s->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

		// SEの再生.
		s->PlaySE( IsStopPlay, true );
		pI->m_PlayTimeList[Name] = GetTickCount64();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を3Dで逆再生しました", Color4::Cyan);
		break;
	}

	if ( IsStopPlay ) return;
	const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
	const float Length = D3DXVec3Length( &ListenerVec );

	// 離れるごとに音量を下げていく.
	const int MaxVolume = pI->m_SEList[Name][0]->GetMaxVolume();
	pI->m_SEList[Name][0]->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

	// SEの再生.
	pI->m_SEList[Name][0]->PlaySE( IsStopPlay, true );
	pI->m_PlayTimeList[Name] = GetTickCount64();
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を3Dで逆再生しました", Color4::Cyan);
}

//----------------------------.
// BGMを逆再生する.
//	ループ停止中の場合処理は行わない.
//----------------------------.
void SoundManager::ReversePlayBGM( std::string Name, bool OldBGMStop, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 今鳴っているBGMを止める.
	if ( OldBGMStop && pI->m_NowBGMName != "" && pI->m_NowBGMName != Name ) {
		pI->m_BGMList[pI->m_NowBGMName][0]->Stop();
	}

	// BGMを再生する.
	pI->m_BGMList[Name][0]->ResetVolume();
	if ( pI->m_BGMList[Name][0]->PlayLoop( true ) == false ) return;
	if ( OldBGMStop ) pI->m_NowBGMName = Name;
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」をループさせました", Color4::Green );
}

//----------------------------.
// BGMをフェードで切り替えて逆再生する.
//	ループ停止中の場合処理は行わない.
//----------------------------.
void SoundManager::ReversePlayFadeBGM( std::string Name, int FadeSpeed, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 今鳴っているBGMを徐々に止める.
	if ( pI->m_NowBGMName != "" && pI->m_NowBGMName != Name ) {
		pI->m_BGMList[pI->m_NowBGMName][0]->AddVolume( -FadeSpeed );
		pI->m_BGMList[Name][0]->SetVolume( 
			pI->m_BGMList[Name][0]->GetMaxVolume() - pI->m_BGMList[Name][0]->GetMaxVolume() * (
			pI->m_BGMList[pI->m_NowBGMName][0]->GetVolume() / pI->m_BGMList[pI->m_NowBGMName][0]->GetMaxVolume() 
		) );

		// BGMを止める.
		if ( pI->m_BGMList[pI->m_NowBGMName][0]->GetVolume() <= 0 ) {
			pI->m_BGMList[pI->m_NowBGMName][0]->ResetVolume();
			pI->m_BGMList[pI->m_NowBGMName][0]->Stop();
			pI->m_BGMList[Name][0]->ResetVolume();
			pI->m_NowBGMName = Name;
		}
	}

	// BGMを再生する.
	if ( pI->m_BGMList[Name][0]->PlayLoop( true ) == false ) return;
	if ( pI->m_NowBGMName	== ""		) pI->m_NowBGMName = Name;
	if ( IsMsgStop			== false	) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」をループさせました", Color4::Green );
}

//----------------------------.
// 3DでBGMを逆再生する.
//	ループ停止中の場合処理は行わない.
//----------------------------.
void SoundManager::ReversePlayBGM3D( std::string Name, D3DXPOSITION3 PlayPos, bool OldBGMStop, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_LoopStopFlag ) return;

	// 今鳴っているBGMを止める.
	if ( OldBGMStop && pI->m_NowBGMName != "" && pI->m_NowBGMName != Name ) pI->m_BGMList[pI->m_NowBGMName][0]->Stop();

	// 再生位置とリスナーの距離を調べる.
	const D3DXVECTOR3& ListenerVec = PlayPos - pI->m_pListener->GetPosition();
	const float Length = D3DXVec3Length( &ListenerVec );

	// 離れるごとに音量を下げていく.
	const int MaxVolume = pI->m_BGMList[Name][0]->GetMaxVolume();
	pI->m_BGMList[Name][0]->SetVolume( MaxVolume - static_cast<int>( Length / VOLUME_DOWN_LENGTH ) );

	// サウンドの再生.
	if ( pI->m_BGMList[Name][0]->PlayLoop( true ) == false ) return;
	if ( OldBGMStop ) pI->m_NowBGMName = Name;
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」をループさせました", Color4::Green );
}

//----------------------------.
// SEを停止させる.
//----------------------------.
void SoundManager::StopSE( std::string Name, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		if ( pI->m_SEList[Name][No]->Stop() ) {
			if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を停止させました", Color4::Green );
		}
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		if ( s->IsStopped() ) continue;

		if ( s->Stop() ) {
			if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を停止させました", Color4::Green );
		}
	}
}

//----------------------------.
// SEを一時停止させる.
//----------------------------.
void SoundManager::PauseSE( std::string Name, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No = pI->m_SelectNo;
		pI->m_SelectNo = -1;

		pI->m_SEList[Name][No]->Pause();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を一時停止させました", Color4::Green );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		if ( s->IsStopped() ) continue;

		s->Pause();
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"SE「" + Name + u8"」を一時停止させました", Color4::Green );
	}
}

//---------------------------.
// BGMを停止させる.
//---------------------------.
void SoundManager::StopBGM( std::string Name, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_BGMList[Name][0]->IsStopped() ) return;
	pI->m_NowBGMName = "";
	if ( pI->m_BGMList[Name][0]->Stop() ) {
		if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」を停止させました", Color4::Green );
	}
}

//---------------------------.
// BGMを一時停止させる.
//---------------------------.
void SoundManager::PauseBGM( std::string Name, bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_BGMList[Name][0]->IsStopped() ) return;
	pI->m_NowBGMName = "";
	pI->m_BGMList[Name][0]->Pause();
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGM「" + Name + u8"」を一時停止させました", Color4::Green );
}

//----------------------------.
// BGMを全て停止させる.
//----------------------------.
void SoundManager::BGMAllStop( bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	pI->m_NowBGMName = "";
	for ( auto& vb : pI->m_BGMList ){
		for ( auto& b : vb.second ) b->Stop();
	}
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGMを全て停止させました", Color4::Green );
}

//----------------------------.
// BGMを全て一時停止させる.
//----------------------------.
void SoundManager::BGMAllPause( bool IsMsgStop )
{
	SoundManager* pI = GetInstance();

	pI->m_NowBGMName = "";
	for ( auto& vb : pI->m_BGMList ){
		for ( auto& b : vb.second ) b->Pause();
	}
	if ( IsMsgStop == false ) MessageWindow::PushMessage( u8"BGMを全て一時停止させました", Color4::Green );
}

//----------------------------.
// 左右のオーディオのON/OFF.
//----------------------------.
void SoundManager::SetIsBGMAllON( std::string Name, bool Flag )
{
	SoundManager* pI = GetInstance();

	for ( auto& b : pI->m_BGMList[Name] ) {
		b->SetIsAudioAllON( Flag );
	}
}
void SoundManager::SetIsSEAllON( std::string Name, bool Flag )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		pI->m_SEList[Name][No]->SetIsAudioAllON( Flag );
		return;
	}

	for ( auto& b : pI->m_SEList[Name] ) {
		b->SetIsAudioAllON( Flag );
	}
}

//----------------------------.
// 左オーディオのON/OFF.
//----------------------------.
void SoundManager::SetIsBGMLeftON( std::string Name, bool Flag )
{
	SoundManager* pI = GetInstance();

	for ( auto& b : pI->m_BGMList[Name] ) {
		b->SetIsAudioLeftON( Flag );
	}
}
void SoundManager::SetIsSELeftON( std::string Name, bool Flag )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		pI->m_SEList[Name][No]->SetIsAudioLeftON( Flag );
		return;
	}

	for ( auto& b : pI->m_SEList[Name] ) {
		b->SetIsAudioLeftON( Flag );
	}
}

//----------------------------.
// 右オーディオのON/OFF.
//----------------------------.
void SoundManager::SetIsBGMRightON( std::string Name, bool Flag )
{
	SoundManager* pI = GetInstance();

	for ( auto& b : pI->m_BGMList[Name] ) {
		b->SetIsAudioRightON( Flag );
	}
}
void SoundManager::SetIsSERightON( std::string Name, bool Flag )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		pI->m_SEList[Name][No]->SetIsAudioRightON( Flag );
		return;
	}

	for ( auto& b : pI->m_SEList[Name] ) {
		b->SetIsAudioRightON( Flag );
	}
}

//----------------------------.
// ループを停止させるか.
//----------------------------.
void SoundManager::SetIssLoopStop( bool Flag, bool IsMsgStop )
{
	GetInstance()->m_LoopStopFlag = Flag;
	if ( IsMsgStop ) return;
	if( Flag )	MessageWindow::PushMessage( u8"ループ再生を停止させました", Color4::Green );
	else		MessageWindow::PushMessage( u8"ループ再生を元に戻しました", Color4::Green );
}

//----------------------------.
// 音量を元に戻す.
//----------------------------.
void SoundManager::ResetSEVolume( std::string Name )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		pI->m_SEList[Name][No]->ResetVolume();
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ) {
		// 音量を元に戻す.
		s->ResetVolume();
	}
}

//----------------------------.
// 音量の設定.
//----------------------------.
void SoundManager::SetSEVolume( std::string Name, int Volume )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		pI->m_SEList[Name][No]->SetVolume( Volume );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		// 音量の設定.
		s->SetVolume( Volume );
	}
}
void SoundManager::SetSEVolume( std::string Name, float Volume )
{
	SetSEVolume( Name, static_cast<int>( NORMAL_VOLUME * Volume ) );
}

//----------------------------.
// 音量の追加.
//----------------------------.
void SoundManager::AddSEVolume( std::string Name, int Volume )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		pI->m_SEList[Name][No]->AddVolume( Volume );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ) {
		// 音量の設定.
		s->AddVolume( Volume );
	}
}
void SoundManager::AddSEVolume( std::string Name, float Volume )
{
	AddSEVolume( Name, static_cast<int>( NORMAL_VOLUME * Volume ) );
}

//----------------------------.
// 音量を元に戻す.
//----------------------------.
void SoundManager::ResetBGMVolume( std::string Name )
{
	SoundManager* pI = GetInstance();

	for ( auto& s : pI->m_BGMList[Name] ) {
		// 音量を元に戻す.
		s->ResetVolume();
	}
}

//----------------------------.
// 音量の設定.
//----------------------------.
void SoundManager::SetBGMVolume( std::string Name, int Volume )
{
	SoundManager* pI = GetInstance();

	for ( auto& s : pI->m_BGMList[Name] ) {
		// 音量の設定.
		s->SetVolume( Volume );
	}
}
void SoundManager::SetBGMVolume( std::string Name, float Volume )
{
	SetBGMVolume( Name, static_cast< int >( NORMAL_VOLUME * Volume ) );
}

//----------------------------.
// 音量の追加.
//----------------------------.
void SoundManager::AddBGMVolume( std::string Name, int Volume )
{
	SoundManager* pI = GetInstance();

	for ( auto& s : pI->m_BGMList[Name] ) {
		// 音量の設定.
		s->AddVolume( Volume );
	}
}
void SoundManager::AddBGMVolume( std::string Name, float Volume )
{
	AddBGMVolume( Name, static_cast< int >( NORMAL_VOLUME * Volume ) );
}

//----------------------------.
// 再生速度を元に戻す.
//----------------------------.
void SoundManager::ResetSEPlaySpeed( std::string Name )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No	= pI->m_SelectNo;

		pI->m_SEList[Name][No]->ResetPlaySpeed();
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ) {
		// 再生速度の設定.
		s->ResetPlaySpeed();
	}
}

//----------------------------.
// 再生速度の設定.
//----------------------------.
void SoundManager::SetSEPlaySpeed( std::string Name, int Speed )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No = pI->m_SelectNo;
		pI->m_SelectNo = -1;

		pI->m_SEList[Name][No]->SetPlaySpeed( Speed );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ){
		// 再生速度の設定.
		s->SetPlaySpeed( Speed );
	}
}
void SoundManager::AddSEPlaySpeed( std::string Name, int Speed )
{
	SoundManager* pI = GetInstance();

	// 制御する番号が決まっているか.
	if ( pI->m_SelectNo >= 0 ) {
		const int No = pI->m_SelectNo;
		pI->m_SelectNo = -1;

		pI->m_SEList[Name][No]->AddPlaySpeed( Speed );
		return;
	}

	for ( auto& s : pI->m_SEList[Name] ) {
		// 再生速度の設定.
		s->AddPlaySpeed( Speed );
	}
}

//----------------------------.
// 再生速度を元に戻す.
//----------------------------.
void SoundManager::ResetBGMPlaySpeed( std::string Name )
{
	SoundManager* pI = GetInstance();

	for ( auto& s : pI->m_BGMList[Name] ) {
		// 再生速度の設定.
		s->ResetPlaySpeed();
	}
}

//----------------------------.
// 再生速度の設定.
//----------------------------.
void SoundManager::SetBGMPlaySpeed( std::string Name, int Speed )
{
	SoundManager* pI = GetInstance();

	for ( auto& s : pI->m_BGMList[Name] ){
		// 再生速度の設定.
		s->SetPlaySpeed( Speed );
	}
}
void SoundManager::AddBGMPlaySpeed( std::string Name, int Speed )
{
	SoundManager* pI = GetInstance();

	for ( auto& s : pI->m_BGMList[Name] ) {
		// 再生速度の設定.
		s->AddPlaySpeed( Speed );
	}
}

//----------------------------.
// サウンドファイルを開く.
//----------------------------.
void SoundManager::OpenSound(
	std::vector<std::shared_ptr<CSound>>* List,	const HWND&			hWnd,
	const int			PlayMax,				const int			MaxVolume,
	const std::string&	FileName,				const std::wstring& wFilePath )
{
	// 多重再生可能個数分音声を作成する.
	List->resize( PlayMax );
	for ( auto& s : *List ) s = std::make_shared<CSound>();

	// サウンドの設定.
	for ( int i = 0; i < PlayMax; ++i ) {
		// エイリアスを w_char に変換する.
		const std::string&	Alias	= FileName + "_" + std::to_string( i );
		const std::wstring&	wAlias	= StringConversion::to_wString( Alias );

		// サウンドファイルを開く.
		( *List )[i]->Open( wFilePath.c_str(), wAlias.c_str(), hWnd, MaxVolume );
	}
}