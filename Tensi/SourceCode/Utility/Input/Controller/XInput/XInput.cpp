#include "XInput.h"
#include "..\..\..\FileManager\FileManager.h"

namespace {
	constexpr char WINDOW_SETTING_FILE_PATH[] = "Data\\Parameter\\Config\\WindowSetting.json";	// �E�B���h�E�̐ݒ�̃t�@�C���p�X.
}

// �l��͈͓��Ɏ��߂�֐�.
template<typename T>
T Clamp( T val, T min, T max )
{
	return ( val < min ) ? min : ( max < val ) ? max : val;
}

XInput::XInput()
	: m_padId			( 0 )
	, m_state			()
	, m_stateOld		()
	, m_vibration		()
#ifdef ENABLE_CLASS_BOOL
	, m_IsVibration		( false, u8"XInput�̐U�����g�p���邩",		"Input" )
	, m_ButStop			( false, u8"XInput�̃{�^�����~�����邩",	"Input" )
#else
	, m_IsVibration		( false )
	, m_ButStop			( false )

#endif
	, m_connect			( false )
	, m_IsNotActiveStop	( false )
{
	// �E�B���h�E�̐ݒ�̎擾.
	json WndSetting		= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	m_IsNotActiveStop	= WndSetting["IsInputNotActiveStop"];
}

XInput::~XInput()
{
	EndProc();
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
XInput* XInput::GetInstance()
{
	static std::unique_ptr<XInput> pInstance = std::make_unique<XInput>();
	return pInstance.get();
}

//----------------------------.
// �L�[���͂̍X�V.
//----------------------------.
bool XInput::Update()
{
	XInput* pI = GetInstance();

	// �L�[�����X�V����O�ɑޔ�.
	pI->m_stateOld = pI->m_state;
	if( pI->UpdateStatus() == false ){
		return false;
	}
	return true;
}

//----------------------------.
// �I������.
//----------------------------.
void XInput::EndProc()
{
	SetVibration( 0, 0 );
}

//----------------------------.
// �L�[�������ꂽ���Ƃ��擾.
//	������:XINPUT_GAMEPAD_A �Ȃ�.
//----------------------------.
bool XInput::IsKeyPress( WORD GamePad )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop == true ) return false;
	switch ( GamePad ) {
	case XINPUT_LSTICK_U:
	case XINPUT_LSTICK_D:
	case XINPUT_LSTICK_L:
	case XINPUT_LSTICK_R:
	case XINPUT_RSTICK_U:
	case XINPUT_RSTICK_D:
	case XINPUT_RSTICK_L:
	case XINPUT_RSTICK_R:
		return IsStiPressed( GamePad );
	case XINPUT_LTRIGGER:
	case XINPUT_RTRIGGER:
		return IsTriPressed( GamePad );
	default:
		return IsButPressed( GamePad );
	}
}

//----------------------------.
// �������u��.
//	������:XINPUT_GAMEPAD_A �Ȃ�.
//----------------------------.
bool XInput::IsKeyDown( WORD GamePad )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop == true ) return false;
	switch ( GamePad ) {
	case XINPUT_LSTICK_U:
	case XINPUT_LSTICK_D:
	case XINPUT_LSTICK_L:
	case XINPUT_LSTICK_R:
	case XINPUT_RSTICK_U:
	case XINPUT_RSTICK_D:
	case XINPUT_RSTICK_L:
	case XINPUT_RSTICK_R:
		return IsStiDown( GamePad );
	case XINPUT_LTRIGGER:
	case XINPUT_RTRIGGER:
		return IsTriDown( GamePad );
	default:
		return IsButDown( GamePad );
	}
}

//----------------------------.
// �������u��.
//	������:XINPUT_GAMEPAD_A �Ȃ�.
//----------------------------.
bool XInput::IsKeyUp( WORD GamePad )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop == true ) return false;
	switch ( GamePad ) {
	case XINPUT_LSTICK_U:
	case XINPUT_LSTICK_D:
	case XINPUT_LSTICK_L:
	case XINPUT_LSTICK_R:
	case XINPUT_RSTICK_U:
	case XINPUT_RSTICK_D:
	case XINPUT_RSTICK_L:
	case XINPUT_RSTICK_R:
		return IsStiUp( GamePad );
	case XINPUT_LTRIGGER:
	case XINPUT_RTRIGGER:
		return IsTriUp( GamePad );
	default:
		return IsButUp( GamePad );
	}
}

//----------------------------.
// ���������Ă���.
//	������:XINPUT_GAMEPAD_A �Ȃ�.
//----------------------------.
bool XInput::IsKeyRepeat( WORD GamePad )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop == true ) return false;
	switch ( GamePad ) {
		case XINPUT_LSTICK_U:
		case XINPUT_LSTICK_D:
		case XINPUT_LSTICK_L:
		case XINPUT_LSTICK_R:
		case XINPUT_RSTICK_U:
		case XINPUT_RSTICK_D:
		case XINPUT_RSTICK_L:
		case XINPUT_RSTICK_R:
			return IsStiRepeat( GamePad );
		case XINPUT_LTRIGGER:
		case XINPUT_RTRIGGER:
			return IsTriRepeat( GamePad );
		default:
			return IsButRepeat( GamePad );
	}
}

//----------------------------.
// �U���ݒ�.
//----------------------------.
bool XInput::SetVibration( const float LMotorSpd, const float RMotorSpd)
{
	XInput* pI = GetInstance();

	if ( pI->m_IsVibration == true ) {
		pI->m_vibration.wLeftMotorSpeed  =
			static_cast<WORD>( INPUT_VIBRATION_MAX * Clamp( LMotorSpd, 0.0f, 1.0f ) );;
		pI->m_vibration.wRightMotorSpeed =
			static_cast<WORD>( INPUT_VIBRATION_MAX * Clamp( RMotorSpd, 0.0f, 1.0f ) );;

		if ( ERROR_SUCCESS == XInputSetState(
			pI->m_padId, &pI->m_vibration ) ) {
			return true;
		}
	}
	return false;
}

//----------------------------.
// �L�[���͂̍X�V.
//----------------------------.
bool XInput::UpdateStatus()
{
	m_connect = false;
	if( ERROR_SUCCESS == XInputGetState(
		m_padId,
		&m_state ) )
	{
		m_connect = true;
		return true;
	}
	return false;
}

//----------------------------.
// �{�^���������ꂽ���Ƃ��擾����.
//----------------------------.
bool XInput::IsButPressed( WORD GamePad )
{
	return	GetInstance()->m_state.Gamepad.wButtons & GamePad;
}

//----------------------------.
// �{�^�����������u��.
//----------------------------.
bool XInput::IsButDown( WORD GamePad )
{
	XInput* pI = GetInstance();

	return	( pI->m_stateOld.Gamepad.wButtons & GamePad ) == 0 &&
			( pI->m_state.Gamepad.wButtons & GamePad ) != 0;
}

//----------------------------.
// �{�^���𗣂����u��.
//----------------------------.
bool XInput::IsButUp( WORD GamePad )
{
	XInput* pI = GetInstance();

	return	( pI->m_stateOld.Gamepad.wButtons & GamePad ) != 0 &&
			( pI->m_state.Gamepad.wButtons & GamePad ) == 0;
}

//----------------------------.
// �{�^�������������Ă���.
//----------------------------.
bool XInput::IsButRepeat( WORD GamePad )
{
	XInput* pI = GetInstance();

	return	( pI->m_stateOld.Gamepad.wButtons & GamePad ) != 0 &&
			( pI->m_state.Gamepad.wButtons & GamePad ) != 0;
}

//----------------------------.
// �X�e�B�b�N���|�������Ƃ��擾����.
//----------------------------.
bool XInput::IsStiPressed( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ) return pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	if ( GamePad == XINPUT_LSTICK_D ) return pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_LSTICK_L ) return pI->m_state.Gamepad.sThumbLX <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_LSTICK_R ) return pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	if ( GamePad == XINPUT_RSTICK_U ) return pI->m_state.Gamepad.sThumbRY >= INPUT_THUMB_MAX;
	if ( GamePad == XINPUT_RSTICK_D ) return pI->m_state.Gamepad.sThumbRY <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_RSTICK_L ) return pI->m_state.Gamepad.sThumbRX <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_RSTICK_R ) return pI->m_state.Gamepad.sThumbRX >= INPUT_THUMB_MAX;
	return false;
}

//----------------------------.
// �X�e�B�b�N��|�����u��.
//----------------------------.
bool XInput::IsStiDown( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_LSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbRY < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbRY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbRY > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbRY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbRX > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbRX <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbRX < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbRX >= INPUT_THUMB_MAX;
	}
	return false;
}

//----------------------------.
// �X�e�B�b�N��߂����u��.
//----------------------------.
bool XInput::IsStiUp( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY < INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_LSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX < INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY < INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX < INPUT_THUMB_MAX;
	}
	return false;
}

//----------------------------.
// �X�e�B�b�N��|�������Ă���.
//----------------------------.
bool XInput::IsStiRepeat( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_LSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	}
	return false;
}

//----------------------------.
// �g���K�[�������ꂽ���Ƃ��擾����.
//----------------------------.
bool XInput::IsTriPressed( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ) return pI->m_state.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX;
	if ( GamePad == XINPUT_RTRIGGER ) return pI->m_state.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX;
	return false;
}

//----------------------------.
// �g���K�[���������u��.
//----------------------------.
bool XInput::IsTriDown( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ){
		return	pI->m_stateOld.Gamepad.bLeftTrigger < INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX;
	}
	if ( GamePad == XINPUT_RTRIGGER ){
		return	pI->m_stateOld.Gamepad.bRightTrigger < INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX;
	}
	return false;
}

//----------------------------.
// �g���K�[�𗣂����u��.
//----------------------------.
bool XInput::IsTriUp( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ){
		return	pI->m_stateOld.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bLeftTrigger < INPUT_TRIGGER_MAX;
	}
	if ( GamePad == XINPUT_RTRIGGER ){
		return	pI->m_stateOld.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bRightTrigger < INPUT_TRIGGER_MAX;
	}
	return false;
}

//----------------------------.
// �g���K�[�����������Ă���.
//----------------------------.
bool XInput::IsTriRepeat( WORD GamePad )
{
	XInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ){
		return	pI->m_stateOld.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX;
	}
	if ( GamePad == XINPUT_RTRIGGER ){
		return	pI->m_stateOld.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX;
	}
	return false;
}
