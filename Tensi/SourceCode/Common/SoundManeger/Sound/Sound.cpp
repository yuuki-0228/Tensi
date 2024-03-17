#include "Sound.h"

CSound::CSound()
	: m_hWnd		( nullptr )
	, m_sAlias		()
	, m_Volume		()
	, m_MaxVolume	()
{
}

CSound::~CSound()
{
}

//----------------------------.
// 音声ファイルを開く.
//----------------------------.
bool CSound::Open( LPCTSTR sFileName, LPCTSTR sAlias, HWND hWnd, int Volume )
{
	// 最大音量を設定.
	m_MaxVolume = Volume;

	// 初期設定.
	SetInitParam( sAlias, hWnd, Volume );

	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "open %s alias %s" ), sFileName, m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 音声ファイルを閉じる.
//----------------------------.
bool CSound::Close()
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "close %s" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 音楽を再生する.
//----------------------------.
bool CSound::Play( bool bNotify )
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( bNotify == true ) {
		swprintf_s( cmd, _T( "play %s notify" ), m_sAlias );
	}
	else {
		swprintf_s( cmd, _T( "play %s" ), m_sAlias );
	}

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 音声を逆再生する.
//----------------------------.
bool CSound::ReversePlay( bool bNotify )
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( bNotify == true ) {
		swprintf_s( cmd, _T( "play %s reverse notify" ), m_sAlias );
	}
	else {
		swprintf_s( cmd, _T( "play %s reverse" ), m_sAlias );
	}

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 音声を再生する(SEで使う).
//----------------------------.
bool CSound::PlaySE( bool StopPlay, bool IsReverse )
{
	if ( StopPlay == false || IsStopped() == true ) SeekToStart();
	if ( IsReverse ) {
		if ( ReversePlay() == true ) {
			return true;
		}
	}
	else {
		if ( Play() == true ) {
			return true;
		}
	}
	return false;
}

//----------------------------.
// 音声を再生する(ループ).
//----------------------------.
bool CSound::PlayLoop( bool IsReverse )
{
	if ( IsStopped() == true ) {
		SeekToStart();
		if ( IsReverse ) {
			if ( ReversePlay( true ) == true ) {
				return true;
			}
		}
		else {
			if ( Play( true ) == true ) {
				return true;
			}
		}
	}
	return false;
}

//----------------------------.
// 音声を停止する.
//----------------------------.
bool CSound::Stop()
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "stop %s" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 音声を一時停止する.
//----------------------------.
bool CSound::Pause()
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "pause %s" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 状態を取得する.
//----------------------------.
bool CSound::GetStatus( LPTSTR sStaus )
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "status %s mode" ), m_sAlias );

	if ( mciSendString( cmd, sStaus, STR_BUFF_MAX, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 音声の停止を確認する.
//----------------------------.
bool CSound::IsStopped()
{
	TCHAR sStatus[STR_BUFF_MAX] = _T( "" );

	// 状態を取得.
	if ( GetStatus( sStatus ) == true ) {
		// 文字列比較.
		if ( wcscmp( sStatus, _T( "stopped" ) ) == 0 ) {
			return true;
		}
	}
	return false;
}

//----------------------------.
// 再生位置を最初に戻す.
//----------------------------.
bool CSound::SeekToStart()
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "seek %s to start" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// 初期設定.
//----------------------------.
void CSound::SetInitParam( LPCTSTR sAlias, HWND hWnd, int Volume )
{
	// ウィンドウハンドルを登録.
	m_hWnd = hWnd;

	// エイリアスを登録(文字列のコピー).
	wcscpy_s( m_sAlias, sAlias );

	// 音量を登録.
	m_Volume = Volume;
}

//---------------------------.
// 左右のオーディオのON/OFF.
//---------------------------.
bool CSound::SetIsAudioAllON( bool Flag )
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( Flag ) swprintf_s( cmd, _T( "set %s audio all on"  ), m_sAlias );
	else		swprintf_s( cmd, _T( "set %s audio all off" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//---------------------------.
// 右オーディオのON/OFF.
//---------------------------.
bool CSound::SetIsAudioLeftON( bool Flag )
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( Flag ) swprintf_s( cmd, _T( "set %s audio left on"  ), m_sAlias );
	else		swprintf_s( cmd, _T( "set %s audio left off" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//---------------------------.
// 左オーディオのON/OFF.
//---------------------------.
bool CSound::SetIsAudioRightON( bool Flag )
{
	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( Flag ) swprintf_s( cmd, _T( "set %s audio right on"  ), m_sAlias );
	else		swprintf_s( cmd, _T( "set %s audio right off" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//---------------------------.
// 音量を元に戻す.
//---------------------------.
void CSound::ResetVolume()
{
	SetVolume( m_MaxVolume );
}

//----------------------------.
// 音量を設定する.
//----------------------------.
bool CSound::SetVolume( int Volume )
{
	// 範囲内に丸める.
	if		( Volume < 0			) { Volume = 0; }
	else if ( m_MaxVolume < Volume	) { Volume = m_MaxVolume; }
	m_Volume = Volume;

	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "setaudio %s volume to %d" ), m_sAlias, m_Volume );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}
bool CSound::AddVolume( int Volume )
{
	return SetVolume( m_Volume + Volume );
}

//---------------------------.
// 再生速度を元に戻す.
//---------------------------.
void CSound::ResetPlaySpeed()
{
	SetPlaySpeed( 1000 );
}

//---------------------------.
// 再生速度を設定する.
//---------------------------.
bool CSound::SetPlaySpeed( int Speed )
{
	// 範囲内に丸める.
	if		( Speed < 0		) { Speed = 0; }
	else if ( 2000 < Speed	) { Speed = 2000; }
	m_PlaySpeed = Speed;

	// コマンド.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "set %s speed %d" ), m_sAlias, m_PlaySpeed );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}
bool CSound::AddPlaySpeed( int Speed )
{
	return SetVolume( m_PlaySpeed + Speed );
}
