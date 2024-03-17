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
// �����t�@�C�����J��.
//----------------------------.
bool CSound::Open( LPCTSTR sFileName, LPCTSTR sAlias, HWND hWnd, int Volume )
{
	// �ő剹�ʂ�ݒ�.
	m_MaxVolume = Volume;

	// �����ݒ�.
	SetInitParam( sAlias, hWnd, Volume );

	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "open %s alias %s" ), sFileName, m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// �����t�@�C�������.
//----------------------------.
bool CSound::Close()
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "close %s" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// ���y���Đ�����.
//----------------------------.
bool CSound::Play( bool bNotify )
{
	// �R�}���h.
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
// �������t�Đ�����.
//----------------------------.
bool CSound::ReversePlay( bool bNotify )
{
	// �R�}���h.
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
// �������Đ�����(SE�Ŏg��).
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
// �������Đ�����(���[�v).
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
// �������~����.
//----------------------------.
bool CSound::Stop()
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "stop %s" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// �������ꎞ��~����.
//----------------------------.
bool CSound::Pause()
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "pause %s" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// ��Ԃ��擾����.
//----------------------------.
bool CSound::GetStatus( LPTSTR sStaus )
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "status %s mode" ), m_sAlias );

	if ( mciSendString( cmd, sStaus, STR_BUFF_MAX, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// �����̒�~���m�F����.
//----------------------------.
bool CSound::IsStopped()
{
	TCHAR sStatus[STR_BUFF_MAX] = _T( "" );

	// ��Ԃ��擾.
	if ( GetStatus( sStatus ) == true ) {
		// �������r.
		if ( wcscmp( sStatus, _T( "stopped" ) ) == 0 ) {
			return true;
		}
	}
	return false;
}

//----------------------------.
// �Đ��ʒu���ŏ��ɖ߂�.
//----------------------------.
bool CSound::SeekToStart()
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	swprintf_s( cmd, _T( "seek %s to start" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//----------------------------.
// �����ݒ�.
//----------------------------.
void CSound::SetInitParam( LPCTSTR sAlias, HWND hWnd, int Volume )
{
	// �E�B���h�E�n���h����o�^.
	m_hWnd = hWnd;

	// �G�C���A�X��o�^(������̃R�s�[).
	wcscpy_s( m_sAlias, sAlias );

	// ���ʂ�o�^.
	m_Volume = Volume;
}

//---------------------------.
// ���E�̃I�[�f�B�I��ON/OFF.
//---------------------------.
bool CSound::SetIsAudioAllON( bool Flag )
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( Flag ) swprintf_s( cmd, _T( "set %s audio all on"  ), m_sAlias );
	else		swprintf_s( cmd, _T( "set %s audio all off" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//---------------------------.
// �E�I�[�f�B�I��ON/OFF.
//---------------------------.
bool CSound::SetIsAudioLeftON( bool Flag )
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( Flag ) swprintf_s( cmd, _T( "set %s audio left on"  ), m_sAlias );
	else		swprintf_s( cmd, _T( "set %s audio left off" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//---------------------------.
// ���I�[�f�B�I��ON/OFF.
//---------------------------.
bool CSound::SetIsAudioRightON( bool Flag )
{
	// �R�}���h.
	TCHAR cmd[STR_BUFF_MAX] = _T( "" );

	if ( Flag ) swprintf_s( cmd, _T( "set %s audio right on"  ), m_sAlias );
	else		swprintf_s( cmd, _T( "set %s audio right off" ), m_sAlias );

	if ( mciSendString( cmd, nullptr, 0, m_hWnd ) == 0 ) {
		return true;
	}
	return false;
}

//---------------------------.
// ���ʂ����ɖ߂�.
//---------------------------.
void CSound::ResetVolume()
{
	SetVolume( m_MaxVolume );
}

//----------------------------.
// ���ʂ�ݒ肷��.
//----------------------------.
bool CSound::SetVolume( int Volume )
{
	// �͈͓��Ɋۂ߂�.
	if		( Volume < 0			) { Volume = 0; }
	else if ( m_MaxVolume < Volume	) { Volume = m_MaxVolume; }
	m_Volume = Volume;

	// �R�}���h.
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
// �Đ����x�����ɖ߂�.
//---------------------------.
void CSound::ResetPlaySpeed()
{
	SetPlaySpeed( 1000 );
}

//---------------------------.
// �Đ����x��ݒ肷��.
//---------------------------.
bool CSound::SetPlaySpeed( int Speed )
{
	// �͈͓��Ɋۂ߂�.
	if		( Speed < 0		) { Speed = 0; }
	else if ( 2000 < Speed	) { Speed = 2000; }
	m_PlaySpeed = Speed;

	// �R�}���h.
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
