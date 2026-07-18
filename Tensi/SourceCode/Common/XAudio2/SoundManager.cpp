#include "SoundManager.h"
#ifdef ENABLE_SOUND
#include <crtdbg.h>
#include <filesystem>
#include "..\..\Global.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"
#include "..\..\Utility\ImGuiManager\DebugWindow\DebugWindow.h"
#include <encrypt/file.h>

namespace fs = std::filesystem;

namespace {
	constexpr char	FILE_PATH[]							= "Data\\Sound";				// Soundデータが入っているディレクトリパス.
	constexpr char	BINARY_FILE_PATH[]					= "Data\\DataCache\\adv.bin";	// バイナリデータが入ってるパス.
	constexpr SoundManager::SSoundVolume INIT_VOLUME	= { 0.5f, 0.5f, 0.5f, 0.5f };	// バイナリデータが存在しない際に設定する初期音量.
	constexpr float	DEFAULT_SOUND_VOLUME				= 1.0f;							// サウンドデータのデフォルトの音量
	constexpr float	DEFAULT_MAX_PITCH					= 2.0f;							// サウンドデータのデフォルトの最大ピッチ数
	constexpr float	DEFAULT_START_PITCH					= 1.0f;							// サウンドデータのデフォルトの初期のピッチ数
}

SoundManager::SoundManager()
	: m_EndGameFlag					( false )
	, m_EndCreateFlag				( false )
	, m_MoveSoundVolumeThreadFlag	( true )
	, m_isCreateThread				( false )
	, m_ResumeSoundVolumeThreadFlag	( true )
	, m_LogStopFlag					( false )
#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
	, m_pEnumerator					( NULL )
	, m_pDevice						( NULL )
	, m_pMeterInfo					( NULL )
#endif
#ifdef _DEBUG
	, m_DebugSoundVolumeChange		( false )
	, m_DebugTestBGMName			( "" )
	, m_DebugTestVoiceName			( "" )
	, m_DebugTestSEName				( "" )
#endif
{
	// Mapコンテナのクリア.
	m_pBgmSource.clear();
	m_pSeSource.clear();
	m_pVoiceSource.clear();
	m_BGMNameList.clear();
	m_SENameList.clear();
	m_VoiceNameList.clear();
	m_pBGMThread.clear();
	m_bisEndThread.clear();
	m_bisThreadRelease.clear();
	m_InThreadID.clear();

#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
	SAFE_RELEASE( m_pEnumerator );
	SAFE_RELEASE( m_pDevice );
	SAFE_RELEASE( m_pMeterInfo );
	CoUninitialize();
#endif
}

SoundManager::~SoundManager()
{
	// 終了フラグを立て, 各再生ループを抜けさせる.
	m_EndGameFlag = true;

	// 音量変更スレッドを停止して回収する.
	m_MoveSoundVolumeThreadFlag = false;
	{
		// cv の待機を解除して while ループを抜けさせる.
		std::lock_guard<std::mutex> lk( m_SoundVolumemtx );
		m_ResumeSoundVolumeThreadFlag = true;
	}
	m_SoundVolumeCv.notify_all();
	// joinable なまま破棄すると terminate するため join する.
	if ( m_SoundSourceVolume.joinable() ) m_SoundSourceVolume.join();

	// BGM 再生スレッドを停止して回収する.
	for ( auto& [Name, Thread] : m_pBGMThread ) {
		// 再生ループを抜けさせる.
		m_bisEndThread[Name] = true;
		// joinable なまま破棄すると terminate するため detach する.
		if ( Thread.joinable() ) Thread.detach();
	}
}

//----------------------------.
// インスタンスの作成.
//----------------------------.
SoundManager* SoundManager::GetInstance()
{
	// インスタンスの作成.
	static std::unique_ptr<SoundManager> pInstance = std::make_unique<SoundManager>();
	return pInstance.get();
}

//----------------------------.
// サウンドデータ作成.
//----------------------------.
void SoundManager::CreateSoundData()
{
	SoundManager* pI = GetInstance();

	std::mutex	m_Mutex;
	m_Mutex.lock();
	if ( pI->m_EndCreateFlag == true ) {
		m_Mutex.unlock();
		return;
	}

	auto eachLoad = [&]( const std::string& entryPath )
	{
		std::string		  extension = fs::path( entryPath ).extension().string();	// 拡張子.
		const std::string filePath = entryPath;					// ファイルパス.
		std::string		  fileName = fs::path( entryPath ).stem().string();			// ファイル名.

#ifndef _DEBUG
		// 暗号化ファイルの場合暗号化前の拡張子に戻す.
		if ( encrypt::GetIsEncryption( filePath ) ) {
			extension = encrypt::GetExtension( filePath );

			// 不要な文字を削除.
			fileName.erase( 0, 1 );
			fileName.erase( fileName.length() - 2 );
		}
#endif

		// 拡張子がOggでなければ終了.
		if ( extension != ".ogg" && extension != ".OGG" ) return;

		// ファイルパス内にBGMがあればBGMを作成をする.
		if ( filePath.find( "BGM" ) != std::string::npos ) {
			// WavLoadクラス作成.
			pI->m_pOggWavData[fileName] = std::make_shared<COggLoad>();
			// Wavフォーマット取得.
			pI->m_pOggWavData[fileName]->CreateOggData( filePath, fileName );
			// PlaySoundクラス作成.
			pI->m_pBgmSource[fileName] = std::make_shared<CXAudio2PlayBGM>();
			// BGMのSoundSource作成.
			pI->m_pBgmSource[fileName]->CreateOggSound( pI->m_pOggWavData[fileName], LoadSoundDataFile( filePath ), pI->m_pOggWavData[fileName]->GetFileName() );
			// フォルダの名前をBGM名前リストに入れる.
			pI->m_BGMNameList.emplace_back( fileName );

			Log::PushLogInfo( filePath + "(BGM) 読み込み : 成功" );
		}
		// ファイルパス内にVoiceがあればVoiceを作成をする.
		else if ( filePath.find( "Voice" ) != std::string::npos ) {
			// WavLoadクラス作成.
			pI->m_pOggWavData[fileName] = std::make_shared<COggLoad>();
			// Wavフォーマット取得.
			pI->m_pOggWavData[fileName]->CreateOggData( filePath, fileName, true );
			// PlaySoundクラス作成.
			pI->m_pVoiceSource[fileName] = std::make_shared<CXAudio2PlayVoice>();
			// VoiceのSoundSource作成.
			pI->m_pVoiceSource[fileName]->CreateOggSound( pI->m_pOggWavData[fileName], LoadSoundDataFile( filePath ), pI->m_pOggWavData[fileName]->GetFileName() );
			// フォルダの名前をVoice名前リストに入れる.
			pI->m_VoiceNameList.emplace_back( fileName );

			Log::PushLogInfo( filePath + "(Voice) 読み込み : 成功" );
		} 
		// SEを作成する
		else {
			// WavLoadクラス作成.
			pI->m_pOggWavData[fileName] = std::make_shared<COggLoad>();
			// Wavフォーマット取得.
			pI->m_pOggWavData[fileName]->CreateOggData( filePath, fileName, true );
			// PlaySoundクラス作成.
			pI->m_pSeSource[fileName] = std::make_shared<CXAudio2PlaySE>();
			// SEのSoundSource作成.
			pI->m_pSeSource[fileName]->CreateOggSound( pI->m_pOggWavData[fileName], LoadSoundDataFile( filePath ), pI->m_pOggWavData[fileName]->GetFileName() );
			// フォルダの名前をSE名前リストに入れる.
			pI->m_SENameList.emplace_back( fileName );

			Log::PushLogInfo( filePath + "(SE) 読み込み : 成功" );
		}
	};

	Log::PushLogInfo( "------ サウンド読み込み開始 ------" );
	try {
		const std::vector<std::string> Files = encrypt::EnumerateDataFiles( FILE_PATH );
		std::for_each( Files.begin(), Files.end(), eachLoad );
	}
	catch ( const fs::filesystem_error& e ) {
		// ファイルが見つからないエラーは無視する.
		if ( std::string( e.what() ).find( "The system cannot find the path specified" ) == std::string::npos ) {
			ErrorMessage( std::string( "サウンドデータ作成失敗(" + e.path1().string() + ")" ) );
		}
	}
	Log::PushLogInfo( "------ サウンド読み込み終了 ------" );

	// 音量設定.
	VolumeInit();
#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
	// オーディオインターフェースの初期化
	AudioInterfaceInit();
#endif

	// BGM,SEの音量を変更するためのスレッドを立ち上げる.
	CreateChangeSoundVolumeThread();

	// サウンド作成終了.
	pI->m_EndCreateFlag = true;
	m_Mutex.unlock();
}

//----------------------------.
// デバッグの更新
//----------------------------.
void SoundManager::DebugUpdate()
{
#ifdef _DEBUG
	DebugWindow::PushProc( u8"SoundManager", [&] {
		SoundManager* pI = GetInstance();

		ImGuiManager::CheckBox( u8"音量の変更", &pI->m_DebugSoundVolumeChange );
		if ( pI->m_DebugSoundVolumeChange ) {
			// 音量変更できるようにしておく
			pI->m_SoundVolumeCv.notify_one();

			ImGui::SetNextItemOpen( true, ImGuiCond_Once );
			if ( ImGui::TreeNode( "SoundVolume" ) ) {
				// マスター音量
				ImGui::Text( "Master" );
				ImGui::SliderFloat( "##Sl_Master", &pI->m_SoundVolume.Master, 0.0f, 1.0f );
				ImGui::SameLine();
				if ( ImGuiManager::Button( "Reset##Master" ) ) pI->m_SoundVolume.Master = INIT_VOLUME.Master;
				ImGui::SameLine();
				ImGuiManager::HelpMarker( u8"初期状態に戻します" );

				// BGM音量
				ImGui::Text( "BGM" );
				ImGui::SliderFloat( "##Sl_BGM", &pI->m_SoundVolume.BGM, 0.0f, 1.0f );
				ImGui::SameLine();
				if ( ImGuiManager::Button( "Reset##BGM" ) ) pI->m_SoundVolume.BGM = INIT_VOLUME.BGM;
				ImGui::SameLine();
				ImGuiManager::HelpMarker( u8"初期状態に戻します" );

				// Voice音量
				ImGui::Text( "Voice" );
				ImGui::SliderFloat( "##Sl_Voice", &pI->m_SoundVolume.Voice, 0.0f, 1.0f );
				ImGui::SameLine();
				if ( ImGuiManager::Button( "Reset##Voice" ) ) pI->m_SoundVolume.Voice = INIT_VOLUME.Voice;
				ImGui::SameLine();
				ImGuiManager::HelpMarker( u8"初期状態に戻します" );

				// SE音量
				ImGui::Text( "SE" );
				ImGui::SliderFloat( "##Sl_SE", &pI->m_SoundVolume.SE, 0.0f, 1.0f );
				ImGui::SameLine();
				if ( ImGuiManager::Button( "Reset##SE" ) ) pI->m_SoundVolume.SE = INIT_VOLUME.SE;
				ImGui::SameLine();
				ImGuiManager::HelpMarker( u8"初期状態に戻します" );

				// 保存
				if ( ImGuiManager::Button( u8"保存" ) ) {
					SaveSoundVolumeData();
					pI->m_DebugSoundVolumeChange = false;
				}
				ImGui::TreePop();
			}
		}

		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "SoundTest" ) ) {
			// BGM音量
			ImGui::Text( "BGM" );
			const auto oldBgm = pI->m_DebugTestBGMName;
			ImGuiManager::Combo( "##NoName_Co_BGM", &pI->m_DebugTestBGMName, pI->m_BGMNameList );
			if ( oldBgm != pI->m_DebugTestBGMName ) StopBGM( oldBgm );
			ImGui::SameLine();
			if ( ImGui::Button( "Play##BGM" ) ) PlayBGM( pI->m_DebugTestBGMName );
			ImGui::SameLine();
			if ( ImGui::Button( "Stop##BGM" ) ) StopBGM( pI->m_DebugTestBGMName );

			// Voice音量
			ImGui::Text( "Voice" );
			const auto oldVoice = pI->m_DebugTestVoiceName;
			ImGuiManager::Combo( "##NoName_Co_Voice", &pI->m_DebugTestVoiceName, pI->m_VoiceNameList );
			if ( oldVoice != pI->m_DebugTestVoiceName ) StopAllVoice( pI->m_DebugTestVoiceName );
			ImGui::SameLine();
			if ( ImGui::Button( "Play##Voice" ) ) NoMultipleVoicePlay( pI->m_DebugTestVoiceName );
			ImGui::SameLine();
			if ( ImGui::Button( "Stop##Voice" ) ) StopAllVoice( pI->m_DebugTestVoiceName );

			// SE音量
			ImGui::Text( "SE" );
			const auto oldSe = pI->m_DebugTestSEName;
			ImGuiManager::Combo( "##NoName_Co_SE", &pI->m_DebugTestSEName, pI->m_SENameList );
			if ( oldSe != pI->m_DebugTestSEName ) StopAllSE( oldBgm );
			ImGui::SameLine();
			if ( ImGui::Button( "Play##SE" ) ) NoMultipleSEPlay( pI->m_DebugTestSEName );
			ImGui::SameLine();
			if ( ImGui::Button( "Stop##SE" ) ) StopAllSE( pI->m_DebugTestSEName );

			ImGui::TreePop();
		}
	} );
#endif
}

//----------------------------.
// スレッドを作ってBGMを再生する関数(外部呼出).
//----------------------------.
void SoundManager::PlayBGM( const std::string& Name, const bool& LoopFlag )
{
	if ( Name.size()				== 0	 ) return;
	if ( CheckBGMDataIsTrue( Name ) == false ) return;

	SoundManager* pI = GetInstance();

	// スレッドに入れるラムダ関数.
	pI->m_bisEndThread[Name] = !LoopFlag;
	auto BGM = [&]( const std::string& Name )
	{
		if ( CheckBGMDataIsTrue( Name ) == false ) return;

		// 再生.
		SoundManager* pI = GetInstance();
		if ( !pI->m_LogStopFlag ) {
			MessageWindow::PushMessage( std::string( u8"BGM「" ) + Name + u8"」を再生しました", Color4::Cyan );
		}
		pI->m_pBgmSource[Name]->Play( pI->m_pOggWavData[Name], pI->m_pOggWavData[Name]->GetFileName(), pI->m_bisEndThread[Name] );
	};

	// 指定したBGMの名前のスレッドからハンドルID取得.
	DWORD ThreadExitCode = -1;
	GetExitCodeThread( pI->m_pBGMThread[Name].native_handle(), &ThreadExitCode );
	// 0xFFFFFFFFが帰ってきたらスレッドが動いていないので動かす.
	if ( ThreadExitCode == 0xFFFFFFFF ) pI->m_pBGMThread[Name] = std::thread( BGM, Name );

	// スレッドのハンドルIDを保持.
	pI->m_InThreadID[Name] = pI->m_pBGMThread[Name].get_id();

	// 鳴らしたBGMのスレッドの解放フラグを下す.
	pI->m_bisThreadRelease[Name] = false;
}

//----------------------------.
// BGM一時停止.
//----------------------------.
void SoundManager::PauseBGM( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_pBgmSource.find( Name ) == pI->m_pBgmSource.end() ) return;
	if ( pI->m_pBgmSource[Name]	== nullptr	) return;
	if ( pI->m_EndGameFlag		== true		) return;

	// 指定したBGM一時停止.
	pI->m_pBgmSource[Name]->Pause();
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"BGM「" ) + Name + u8"」を一時停止しました", Color4::Cyan );
}

//----------------------------.
// BGM再開関数.
//----------------------------.
void SoundManager::AgainPlayBGM( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_pBgmSource.find( Name ) == pI->m_pBgmSource.end() ) return;
	if ( pI->m_pBgmSource[Name]	== nullptr	) return;
	if ( pI->m_EndGameFlag		== true		) return;

	// 指定したBGM再開.
	pI->m_pBgmSource[Name]->PlayStart();
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"BGM「" ) + Name + u8"」を再開しました", Color4::Cyan );
}

//----------------------------.
// 音量を変更できるスレッドのステート変更.
//----------------------------.
void SoundManager::StateChangeVolumeThread( const bool& bFlag )
{
	SoundManager* pI = GetInstance();

	// 入れたフラグがtrueの時、元がfalseの場合、スレッドがサスペンドしているため.
	// 音量を変更するスレッドを起床させる.
	pI->m_ResumeSoundVolumeThreadFlag = bFlag;
	if ( bFlag == true ) pI->m_SoundVolumeCv.notify_one();
}

//----------------------------.
// 音量の更新
//----------------------------.
void SoundManager::SoundVolumeUpdate()
{
	SoundManager* pI = GetInstance();

	if ( pI->m_MoveSoundVolumeThreadFlag == false ) return;

	// BGM用ループ.
	for ( size_t i = 0; i < pI->m_BGMNameList.size(); i++ ) {
		// 音量をセット.
		pI->m_pBgmSource[pI->m_BGMNameList[i]]->SetBGMVolume();
	}
	// SE用ループ.
	for ( size_t i = 0; i < pI->m_SENameList.size(); i++ ) {
		// 音量をセット.
		pI->m_pSeSource[pI->m_SENameList[i]]->SetSEVolume();
	}
	// Voice用ループ.
	for ( size_t i = 0; i < pI->m_VoiceNameList.size(); i++ ) {
		// 音量をセット.
		pI->m_pVoiceSource[pI->m_VoiceNameList[i]]->SetVoiceVolume();
	}
}

//----------------------------.
// BGM停止関数.
//----------------------------.
void SoundManager::StopBGM( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( CheckBGMDataIsTrue( Name ) == false ) return;

	// 指定したBGM停止.
	pI->m_pBgmSource[Name]->Stop();
	while ( StopBGMThread( Name ) == false );
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"BGM「" ) + Name + u8"」を停止しました", Color4::Cyan );
}

//----------------------------.
// BGMフェードアウト関数.
//----------------------------.
void SoundManager::FadeOutBGM( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( CheckBGMDataIsTrue( Name ) == false ) return;

	// フェードイン中だった場合、それを止めフェードアウトを始める.
	pI->m_pBgmSource[Name]->SetFadeInFlag( false );

	// フェードアウトフラグを立てる.
	if ( pI->m_pBgmSource[Name]->GetFadeOutFlag() == false ) {
		pI->m_pBgmSource[Name]->SetFadeOutFlag( true );
		if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"BGM「" ) + Name + u8"」をフェードアウト開始しました", Color4::Cyan );
	}
}

//----------------------------.
// 指定BGMの音量のフェードアウト実行中かを返す.
//----------------------------.
const bool SoundManager::GetFadeOutBGM( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( CheckBGMDataIsTrue( Name ) == false ) return false;
	return pI->m_pBgmSource[Name]->GetFadeOutFlag();
}

//----------------------------.
// BGMフェードイン関数.
//----------------------------.
void SoundManager::FadeInBGM( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( CheckBGMDataIsTrue( Name ) == false ) return;

	// フェードアウト中だった場合、それを止めフェードインを始める.
	pI->m_pBgmSource[Name]->SetFadeOutFlag( false );

	// フェードインフラグを立てる.
	if ( pI->m_pBgmSource[Name]->GetFadeInFlag() == false ) {
		pI->m_pBgmSource[Name]->SetFadeInFlag( true );
		if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"BGM「" ) + Name + u8"」をフェードイン開始しました", Color4::Cyan );
	}
}

//----------------------------.
// 指定BGMの音量のフェードイン実行中かを返す.
//----------------------------.
const bool SoundManager::GetFadeInBGM( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( CheckBGMDataIsTrue( Name ) == false ) return false;
	return pI->m_pBgmSource[Name]->GetFadeInFlag();
}

//----------------------------.
// BGMを停止し、そのBGMを再生していたスレッドを放棄する.
//----------------------------.
const bool SoundManager::StopBGMThread( const std::string BGMName )
{
	SoundManager* pI = GetInstance();

	// スレッドが解放されていればリターン.
	if ( pI->m_bisThreadRelease[BGMName] == true ) return true;

	// 再生時に設定したフラグをtrueにして、BGMを停止に向かわせる.
	pI->m_bisEndThread[BGMName] = true;
	DWORD ThreadExitCode = -1;

	// スレッドが停止したかどうかをID取得で取得.
	GetExitCodeThread( pI->m_pBGMThread[BGMName].native_handle(), &ThreadExitCode );

	// 返って来る値が0xFFFFFFFFの場合、そもそもスレッドが立ち上がっていない(再生していない)のでリターン.
	if ( ThreadExitCode == 0xFFFFFFFF ) return true;
	// 停止していたら0が返って来る.
	if ( ThreadExitCode == 0 ) {
		// スレッドから帰ってくるIDが、再生時に保持したIDと同じかをチェック.
		if ( pI->m_InThreadID[BGMName] == pI->m_pBGMThread[BGMName].get_id() ) {
			pI->m_pBGMThread[BGMName].detach();
			pI->m_bisThreadRelease[BGMName] = true;
			return true;
		}
		return false;
	}
	return false;
}

//----------------------------.
// 指定した名前のBGMスレッドが再生中かどうか.
//----------------------------.
const bool SoundManager::GetIsPlayBGM( const std::string BGMName )
{
	SoundManager* pI = GetInstance();

	// スレッドが停止したかどうかをID取得で取得.
	DWORD ThreadExitCode = -1;
	GetExitCodeThread( pI->m_pBGMThread[BGMName].native_handle(), &ThreadExitCode );
	// 返って来る値が0xFFFFFFFFの場合、再生していない.
	if ( ThreadExitCode == 0xFFFFFFFF ) return false;
	// 0ならスレッド停止済み.
	else if ( ThreadExitCode == 0 ) return false;
	return true;
}

//----------------------------.
// BGMのピッチ設定.
//----------------------------.
void SoundManager::SetBGMPitch( const std::string Name, const float Value )
{
	SoundManager* pI = GetInstance();

	// 存在しなければリターン.
	if ( pI->m_pBgmSource[Name] == nullptr ) return;
	pI->m_pBgmSource[Name]->SetPitch( Value );
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"BGM「" ) + Name + u8"」のピッチを「" + std::to_string( Value ) + u8"」に設定しました", Color4::Cyan );
}

//----------------------------.
// BGMを全て止めて、スレッドを放棄する関数.
//----------------------------.
void SoundManager::StopAllBGM()
{
	SoundManager* pI = GetInstance();

	for ( auto& name : pI->m_BGMNameList ) {
		StopBGM( name );
	}
}

//----------------------------.
// SE再生関数.
//----------------------------.
void SoundManager::PlaySE( const std::string& Name )
{
	SoundManager* pI = GetInstance();

	// mapコンテナにキーが存在してなければリターン.
	if ( pI->m_pSeSource.find( Name )	== pI->m_pSeSource.end() ) return;
	if ( pI->m_pSeSource[Name]			== nullptr				 ) return;

	// 再生.
	pI->m_pSeSource[Name]->Play( pI->m_pOggWavData[Name] );
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"SE「" ) + Name + u8"」を再生しました", Color4::Cyan );
}

//----------------------------.
// 多重再生しないSE再生関数.
//----------------------------.
void SoundManager::NoMultipleSEPlay( const std::string& Name )
{
	SoundManager* pI = GetInstance();

	// mapコンテナにキーが存在してなければリターン.
	if ( pI->m_pSeSource.find( Name )	== pI->m_pSeSource.end() ) return;
	if ( pI->m_pSeSource[Name]			== nullptr				 ) return;

	// 再生.
	pI->m_pSeSource[Name]->NoMultiplePlay( pI->m_pOggWavData[Name] );
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"SE「" ) + Name + u8"」を再生しました(多重再生しない)", Color4::Cyan );
}

//----------------------------.
// SE停止関数.
//----------------------------.
void SoundManager::StopSE( const std::string Name, const size_t ArrayNum )
{
	SoundManager* pI = GetInstance();

	// 停止
	if ( pI->m_pSeSource[Name] == nullptr ) return;
	pI->m_pSeSource[Name]->SeStop( ArrayNum );
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"SE「" ) + Name + u8"」を停止しました", Color4::Cyan );
}

//----------------------------.
// 同じ音源の全てのSEソース停止関数.
//----------------------------.
void SoundManager::StopAllSE()
{
	SoundManager* pI = GetInstance();

	for ( auto& name : pI->m_SENameList ) {
		StopAllSE( name );
	}
}
void SoundManager::StopAllSE( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_pSeSource[Name] == nullptr ) return;
	pI->m_pSeSource[Name]->AllSeStop();
	if ( !pI->m_LogStopFlag ) MessageWindow::PushMessage( std::string( u8"SE「" ) + Name + u8"」を全て停止しました", Color4::Cyan );
}

//----------------------------.
// 指定した名前のSEが再生中かどうかを返す : 再生中ならtrue.
//----------------------------.
bool SoundManager::GetIsPlaySE( const std::string Name, const size_t ArrayNum )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_pSeSource[Name] == nullptr ) return false;
	return pI->m_pSeSource[Name]->IsPlayingSE( ArrayNum );
}

//----------------------------.
// Voice再生関数.
//----------------------------.
void SoundManager::PlayVoice( const std::string& Name )
{
	SoundManager* pI = GetInstance();

	// mapコンテナにキーが存在してなければリターン.
	if ( pI->m_pVoiceSource.find( Name )	== pI->m_pVoiceSource.end()	) return;
	if ( pI->m_pVoiceSource[Name]			== nullptr					) return;

	// 再生.
	pI->m_pVoiceSource[Name]->Play( pI->m_pOggWavData[Name] );
}

//----------------------------.
// 多重再生しないVoice再生関数.
//----------------------------.
void SoundManager::NoMultipleVoicePlay( const std::string& Name )
{
	SoundManager* pI = GetInstance();

	// mapコンテナにキーが存在してなければリターン.
	if ( pI->m_pVoiceSource.find( Name )	== pI->m_pVoiceSource.end()	) return;
	if ( pI->m_pVoiceSource[Name]			== nullptr					) return;

	// 再生.
	pI->m_pVoiceSource[Name]->NoMultiplePlay( pI->m_pOggWavData[Name] );
}

//----------------------------.
// Voice停止関数.
//----------------------------.
void SoundManager::StopVoice( const std::string Name, const size_t ArrayNum )
{
	SoundManager* pI = GetInstance();

	// 停止
	if ( pI->m_pVoiceSource[Name] == nullptr ) return;
	pI->m_pVoiceSource[Name]->VoiceStop( ArrayNum );
}

//----------------------------.
// 同じ音源の全てのVoiceソース停止関数.
//----------------------------.
void SoundManager::StopAllVoice()
{
	SoundManager* pI = GetInstance();

	for ( auto& name : pI->m_VoiceNameList ) {
		StopAllVoice( name );
	}
}
void SoundManager::StopAllVoice( const std::string Name )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_pVoiceSource[Name] == nullptr ) return;
	pI->m_pVoiceSource[Name]->AllVoiceStop();
}

//----------------------------.
// 指定した名前のVoiceが再生中かどうかを返す : 再生中ならtrue.
//----------------------------.
bool SoundManager::GetIsPlayVoice( const std::string Name, const size_t ArrayNum )
{
	SoundManager* pI = GetInstance();

	if ( pI->m_pVoiceSource[Name] == nullptr ) return false;
	return pI->m_pVoiceSource[Name]->IsPlayingVoice( ArrayNum );
}

//----------------------------.
// ピーク地点の取得.
//----------------------------.
float SoundManager::GetPeakValue()
{
	SoundManager* pI = GetInstance();

	float Value = 0.0f;
	if (pI->m_pMeterInfo == NULL) return Value;
	pI->m_pMeterInfo->GetPeakValue(&Value);
	return Value;
}

//----------------------------.
// 解放関数.
//----------------------------.
void SoundManager::Release()
{
	SoundManager* pI = GetInstance();

	// 音量を変更するスレッドの解放待ち.
	while ( ReleaseChangeSoundVolumeThread() == false );

	// 音量をバイナリデータにセーブ.
	SaveSoundVolumeData();

	// Voiceの名前リスト数分ループを回す.
	for ( size_t i = 0; i < pI->m_VoiceNameList.size(); i++ ) {
		StopAllVoice( pI->m_VoiceNameList[i] );
		pI->m_pOggWavData[pI->m_VoiceNameList[i]]->Close();			// WavDataを閉じる.
		pI->m_pVoiceSource[pI->m_VoiceNameList[i]]->DestoroySource();	// SoundSourceを解放.
	}
	// SEの名前リスト数分ループを回す.
	for ( size_t i = 0; i < pI->m_SENameList.size(); i++ ) {
		StopAllSE( pI->m_SENameList[i] );
		pI->m_pOggWavData[pI->m_SENameList[i]]->Close();				// WavDataを閉じる.
		pI->m_pSeSource[pI->m_SENameList[i]]->DestoroySource();		// SoundSourceを解放.
	}
	// BGMの名前リスト数分ループを回す.
	for ( size_t i = 0; i < pI->m_BGMNameList.size(); i++ ) {
		pI->m_pBgmSource[pI->m_BGMNameList[i]]->Stop();
		while ( StopBGMThread( pI->m_BGMNameList[i] ) == false );
		pI->m_pOggWavData[pI->m_BGMNameList[i]]->Close();				// WavDataを閉じる.
		pI->m_pBgmSource[pI->m_BGMNameList[i]]->DestoroySource();		// SoundSourceを解放.
	}

	// ゲーム終了フラグを立てる.
	pI->m_EndGameFlag = true;
}

//----------------------------.
// サウンド音量データを保存
//----------------------------.
void SoundManager::SaveSoundVolumeData()
{
	// 音量をバイナリデータにセーブ.
	XAudio2File::CreateBinary( BINARY_FILE_PATH, GetInstance()->m_SoundVolume );
}

//----------------------------.
// マスター音量セット.
//----------------------------.
void SoundManager::SetSoundVolume( const SSoundVolume& volume )
{
	SoundManager* pI = GetInstance();

	pI->m_SoundVolume = volume;
	if (		pI->m_SoundVolume.Master	>= 1.0f ) pI->m_SoundVolume.Master	= 1.0f;
	else if (	pI->m_SoundVolume.Master	<= 0.0f ) pI->m_SoundVolume.Master	= 0.0f;
	if (		pI->m_SoundVolume.BGM		>= 1.0f ) pI->m_SoundVolume.BGM		= 1.0f;
	else if (	pI->m_SoundVolume.BGM		<= 0.0f ) pI->m_SoundVolume.BGM		= 0.0f;
	if (		pI->m_SoundVolume.SE		>= 1.0f ) pI->m_SoundVolume.SE		= 1.0f;
	else if (	pI->m_SoundVolume.SE		<= 0.0f ) pI->m_SoundVolume.SE		= 0.0f;
	if (		pI->m_SoundVolume.Voice		>= 1.0f ) pI->m_SoundVolume.Voice	= 1.0f;
	else if (	pI->m_SoundVolume.Voice		<= 0.0f ) pI->m_SoundVolume.Voice	= 0.0f;
}

//----------------------------.
// BGM,SEの音量を変更するためのスレッドを立ち上げる.
//----------------------------.
void SoundManager::CreateChangeSoundVolumeThread()
{
	SoundManager* pI = GetInstance();

	// スレッド作成のフラグが立っていればリターン.
	if ( pI->m_isCreateThread == true ) return;

	// スレッド作成.
	SetChangeVolumeThread();
	pI->m_isCreateThread = true;
}

//----------------------------.
// 音量を変更するスレッドを立てる.
//----------------------------.
void SoundManager::SetChangeVolumeThread()
{
	SoundManager* pI = GetInstance();

	// スレッド状態を取得.
	DWORD ThreadExitCode = -1;
	GetExitCodeThread( pI->m_SoundSourceVolume.native_handle(), &ThreadExitCode );

	// スレッドが動いていればリターン.
	if ( ThreadExitCode != 0xFFFFFFFF && ThreadExitCode != 0 ) {
		return;
	}

	pI->m_MoveSoundVolumeThreadFlag = true;
	auto SetBGMVolme = [&]()
	{
		SoundManager* pI = GetInstance();

		while ( pI->m_MoveSoundVolumeThreadFlag )
		{
			std::unique_lock<std::mutex> lk( pI->m_SoundVolumemtx ); // mutex.
			// スレッドをm_bMoveSoundVolumeThreadがfalseの間ここでサスペンド(一切動かさない)、trueで再開.
			pI->m_SoundVolumeCv.wait( lk, [&] { return pI->m_ResumeSoundVolumeThreadFlag; } );
			SoundVolumeUpdate();
		}
	};
	pI->m_SoundSourceVolume = std::thread( SetBGMVolme );
}

//----------------------------.
// 音量初期設定関数.
//----------------------------.
void SoundManager::VolumeInit()
{
	SoundManager* pI = GetInstance();

	// バイナリデータから音量を読み込む.
	HRESULT hr;
	hr = XAudio2File::LoadBinary( BINARY_FILE_PATH, pI->m_SoundVolume );

	// バイナリデータが存在しない場合.
	if ( hr == E_FAIL ) {
		// バイナリデータ新規作成.
		XAudio2File::CreateBinary( BINARY_FILE_PATH, INIT_VOLUME );
		pI->m_SoundVolume = INIT_VOLUME;
	}
	SoundVolumeUpdate();
}

#ifdef ENABLE_WINDOWS_AUDIO_DEVICE
//----------------------------.
// オーディオインターフェースの初期化
//----------------------------.
HRESULT SoundManager::AudioInterfaceInit()
{
	SoundManager* pI = GetInstance();

	if ( FAILED( CoInitialize( NULL ) ) ) {
		Log::PushLogWarning( "CoInitialize : 失敗（サウンドメーターが無効化されています）" );
		return S_OK;
	}
	if ( FAILED( CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, __uuidof( IMMDeviceEnumerator ), (void**) &pI->m_pEnumerator ) ) || pI->m_pEnumerator == NULL ) {
		Log::PushLogWarning( "MMDeviceEnumerator create : 失敗 (SoundMeterが無効)" );
		return S_OK;
	}
	if ( FAILED( pI->m_pEnumerator->GetDefaultAudioEndpoint( eRender, eConsole, &pI->m_pDevice ) ) || pI->m_pDevice == NULL ) {
		Log::PushLogWarning( "GetDefaultAudioEndpoint : 失敗 (オーディオデバイスがありません )" );
		return S_OK;
	}
	if ( FAILED( pI->m_pDevice->Activate( __uuidof( IAudioMeterInformation ), CLSCTX_ALL, NULL, (void**) &pI->m_pMeterInfo ) ) ) {
		Log::PushLogWarning( "IAudioMeterInformation activate : 失敗" );
		pI->m_pMeterInfo = NULL;
	}
	return S_OK;
}
#endif

bool SoundManager::CheckBGMDataIsTrue( const std::string Name )
{
	SoundManager* pI = GetInstance();

	// mapコンテナにキーが存在しているか.
	if ( pI->m_pBgmSource.find( Name ) == pI->m_pBgmSource.end() ) return false;
	// 中身があるか.
	if ( pI->m_pBgmSource[Name]	== nullptr	) return false;
	// ゲーム終了フラグがたっていないか.
	if ( pI->m_EndGameFlag == true			) return false;
	return true;
}

//----------------------------.
// BGM,SEの音量を変更するためのスレッドを解放する.
//----------------------------.
bool SoundManager::ReleaseChangeSoundVolumeThread()
{
	SoundManager* pI = GetInstance();

	// スレッドを起床させる.
	pI->m_ResumeSoundVolumeThreadFlag = true;
	pI->m_SoundVolumeCv.notify_one();

	// スレッドが解放できる状態(スレッドが動いている). 
	if ( pI->m_SoundSourceVolume.joinable() == true ) {
		pI->m_MoveSoundVolumeThreadFlag = false;
		pI->m_SoundSourceVolume.join();
		while ( 1 )
		{
			if ( pI->m_SoundSourceVolume.joinable() != true ) {
				break;
			}
		}
	}

	pI->m_isCreateThread = false;
	return true;
}

//----------------------------.
// サウンドデータjsonの読み込み
//----------------------------.
json SoundManager::LoadSoundDataFile( const std::string filePath )
{
	// 同じ名前のテキストの読み込み.
	std::string dataPath = filePath;
	dataPath.erase( dataPath.rfind( "." ), dataPath.size() ) += ".json";
#ifndef _DEBUG
	// 暗号化されているため元のパスに戻す
	auto dp = dataPath.rfind( "." );
	auto fp = dataPath.rfind( "\\" ) + 1;
	if ( dp != std::string::npos && fp != std::string::npos ) {
		std::string n = dataPath.substr( fp, dp - fp );
		n.erase( 0, 1 );
		n.erase( n.length() - 2 );
		dataPath.erase( fp, dp - fp );
		dataPath.insert( fp, n );
	}
#endif
	Json spriteStateData = FileManager::JsonLoad( dataPath );

#ifdef _DEBUG
	// ファイルが無いためファイルを作成する.
	if ( spriteStateData.empty() ) {
		Log::PushLogWarning( dataPath + " が無いため作成します。" );
		if ( FAILED( CreateSoundDataFile( dataPath ) ) ) return E_FAIL;

		// 作成できたためもう一度読み込み直す.
		Log::PushLogWarning( dataPath + " をもう一度読み込み直します。" );
		return LoadSoundDataFile( filePath );
	}
#endif
	return spriteStateData;
}

//----------------------------.
// サウンドデータの作成
//----------------------------.
HRESULT SoundManager::CreateSoundDataFile( const std::string filePath )
{
	// 情報を追加していく.
	Json soundDataFile;
	soundDataFile["SoundVolume"]	= DEFAULT_SOUND_VOLUME;
	soundDataFile["MaxPitch"]		= DEFAULT_MAX_PITCH;
	soundDataFile["StartPitch"]		= DEFAULT_START_PITCH;

	// スプライト情報の作成.
	if ( FAILED( FileManager::JsonSave( filePath, soundDataFile ) ) ) return E_FAIL;
	Log::PushLogInfo( filePath + " ファイルの作成 : 成功" );

	// 作成成功.
	return S_OK;
}
#endif // ENABLE_SOUND
