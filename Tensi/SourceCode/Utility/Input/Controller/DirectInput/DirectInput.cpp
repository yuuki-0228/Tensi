#include "DirectInput.h"
#include <tchar.h>
#include <crtdbg.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <memory>

template <typename T>
T ToRadian( T degree )
{
	return static_cast<T>( degree * M_PI / 180.0 );
}

//����}�N��.
#define SAFE_RELEASE(p)	if(p!=nullptr){(p)->Release();(p)=nullptr;}

//�ŏI�I�ɂ͕����R���g���[���Ή�������.
//#define ENABLE_MULTIPLE_GAMEPAD

LPDIRECTINPUT8			g_pDICore = nullptr;	//DirectInput�I�u�W�F�N�g.
LPDIRECTINPUTDEVICE8	g_pPadCore = nullptr;	//�R���g���[��.	�������ڑ��Ȃ琔�𑝂₵�đΉ����邱��.

//�W���C�X�e�B�b�N�񋓊֐�.
BOOL CALLBACK EnumJoysticksCallBack( const DIDEVICEINSTANCE* pdidi, VOID* pContext );
//�I�u�W�F�N�g�̗񋓊֐�.
BOOL CALLBACK EnumObjectsCallBack( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );


DirectInput::DirectInput()
	: m_hWnd		( nullptr )
	, m_pDInput		( nullptr )
	, m_pPad		( nullptr )
	, m_JoyState	()
	, m_Status		( 0 )
	, m_StatusOld	( 0 )
#ifdef ENABLE_CLASS_BOOL
	, m_ButStop		( false, u8"DirectInput�̃{�^�����~�����邩", "Input" )
#else
	, m_ButStop		( false )
#endif
{
}

DirectInput::~DirectInput()
{
	m_hWnd		= nullptr;
	m_pDInput	= nullptr;
	m_pPad		= nullptr;

	//�ȉ��̃O���[�o���ϐ��̉���^�C�~���O���Ȃ��̂ł����ŏ�������.
	SAFE_RELEASE( g_pDICore );
	SAFE_RELEASE( g_pPadCore );
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
DirectInput* DirectInput::GetInstance()
{
	static std::unique_ptr<DirectInput> pInstance = std::make_unique<DirectInput>();
	return pInstance.get();
}

//----------------------------.
// DirectInput�̏����ݒ�.
//----------------------------.
bool DirectInput::Init()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  //�֐����A�l.

	hRslt = DirectInput8Create(
		GetModuleHandle( nullptr ),				//DirectInput���쐬����A�v���̃n���h��.
		DIRECTINPUT_VERSION,					//�Œ�FDirectInput�̃o�[�W����.
		IID_IDirectInput8,						//�Œ�FUnicode��Ansi�̋�ʗp.
		reinterpret_cast<LPVOID*>( &g_pDICore ),//(out)DirectInput�I�u�W�F�N�g.
		NULL );									//�Œ�:nullptr.
	if( hRslt != DI_OK ){
		MessageBox( NULL, _T( "DirectInput�̍쐬�Ɏ��s" ), _T( "�G���[" ), MB_OK );
		return false;
	}

	//�����o�[�ϐ��֐ݒ�.
	pI->m_pDInput = g_pDICore;

	//���p�\�ȃR���g���[����T��(�񋓂���).
	hRslt = g_pDICore->EnumDevices(
		DI8DEVCLASS_GAMECTRL,	//�S�ẴQ�[���R���g���[��.
		EnumJoysticksCallBack,	//�R���g���[���̗񋓊֐�.
		nullptr,				//�R�[���o�b�N�֐��ɑ�����.
		DIEDFL_ATTACHEDONLY );	//�q�����Ă��郂�m�̂�.
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "�R���g���[���̊m�F�Ɏ��s" ), _T( "�G���[" ), MB_OK );
		return false;
	}

	//�����o�[�ϐ��֐ݒ�.
	pI->m_pPad = g_pPadCore;

	if( FAILED( SettingController() ) ) return false;
	return true;
}

//----------------------------.
// �X�V�֐�.
//	�����t���[���Ăяo����Ȃ��Ɠ��͏�񂪍X�V����܂���.
//----------------------------.
HRESULT DirectInput::Update()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  // �֐����A�l.

	// �W���C�X�e�B�b�N�̐ڑ��m�F.
	if( pI->m_pPad == nullptr ){
		return E_FAIL;
	}

	// �W���C�X�e�B�b�N���ύX����Ă��邩���m�F.
	hRslt = pI->m_pPad->Poll();
	if( hRslt != DI_OK && hRslt != DI_NOEFFECT )    // DI_NOEFFECT:�m�F�s�v�ȃf�o�C�X.
	{
		// �R���g���[���ւ̃A�N�Z�X�����擾����.
		hRslt = pI->m_pPad->Acquire();
		while( hRslt == DIERR_INPUTLOST ){
			hRslt = pI->m_pPad->Acquire();
		}
		return E_FAIL;
	}

	// �R���g���[���̃L�[�����X�V.
	UpdateKeyInput();

	return S_OK;
}

//----------------------------.
// �L�[�������ꂽ���Ƃ��擾����.
//----------------------------.
bool DirectInput::IsKeyPress( DIRECT_INPUT_KEY Key )
{
	if( IsKeyCore( Key, GetInstance()->m_Status ) == true )
	{
		return true;
	}
	return false;
}

//----------------------------.
// ������.just=true�ɂ��邱�Ƃŉ������u�Ԃ̎擾���\.
//----------------------------.
bool DirectInput::IsKeyDown( DIRECT_INPUT_KEY Key )
{
	DirectInput* pI = GetInstance();

	if(  IsKeyCore( Key, pI->m_Status		) &&
		!IsKeyCore( Key, pI->m_StatusOld	) )
	{
		return true;
	}
	return false;
}

//----------------------------.
// ������.
//----------------------------.
bool DirectInput::IsKeyUp( DIRECT_INPUT_KEY Key )
{
	DirectInput* pI = GetInstance();

	//�O����͂ō��񖢓��́��������u�Ԃ̔���.
	if(  IsKeyCore( Key, pI->m_StatusOld	) &&
		!IsKeyCore( Key, pI->m_Status		) )
	{
		return true;
	}
	return false;
}

//----------------------------.
// ���������Ă���.
//----------------------------.
bool DirectInput::IsKeyRepeat( DIRECT_INPUT_KEY key )
{
	DirectInput* pI = GetInstance();

	//�O����͂ō�����́����������Ă锻��.
	if( IsKeyCore( key, pI->m_StatusOld ) &&
		IsKeyCore( key, pI->m_Status	) )
	{
		return true;
	}
	return false;
}

//----------------------------.
// �ڑ��`�F�b�N.
//----------------------------.
bool DirectInput::IsConnect()
{
	return g_pPadCore != nullptr;
}

//----------------------------.
// �R���g���[���ڑ��m�F.
//----------------------------.
HRESULT DirectInput::SettingController()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  // �֐����A�l.

	// �R���g���[���̐ڑ�����Ă��邩.
	if ( g_pPadCore == nullptr ) return S_OK;
	
	// �R���g���[���\���̂̃f�[�^�t�H�[�}�b�g���쐬.
	hRslt = g_pPadCore->SetDataFormat(
		&c_dfDIJoystick2 ); // �Œ�
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "�f�[�^�t�H�[�}�b�g�̍쐬���s" ), _T( "�G���[" ), MB_OK );
	}
	// (���̃f�o�C�X�Ƃ�)�������x���̐ݒ�.
	hRslt = g_pPadCore->SetCooperativeLevel(
		pI->m_hWnd,
		DISCL_EXCLUSIVE |   // �r���A�N�Z�X.
		DISCL_FOREGROUND ); // �t�H�A�O���E���h�A�N�Z�X��.
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "�������x���̐ݒ莸�s" ), _T( "�G���[" ), MB_OK );
	}
#if 0	// ����������_���H
	//	�g�p�\�ȃI�u�W�F�N�g(�{�^���Ȃ�)�̗�.
	hRslt = g_pPadCore->EnumObjects(
		EnumObjectsCallBack,					// �I�u�W�F�N�g�񋓊֐�.
		reinterpret_cast<LPVOID>( g_pPadCore ),	// �R�[���o�b�N�֐��ɑ�����.
		DIDFT_ALL );							// �S�ẴI�u�W�F�N�g.
#else
	// �g�p�\�ȃI�u�W�F�N�g(�{�^���Ȃ�)�̗�.
	hRslt = g_pPadCore->EnumObjects(
		EnumObjectsCallBack,					// �I�u�W�F�N�g�񋓊֐�.
		reinterpret_cast<VOID*>( pI->m_hWnd ),	// �R�[���o�b�N�֐��ɑ�����.
		DIDFT_ALL );							// �S�ẴI�u�W�F�N�g.
#endif
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "�I�u�W�F�N�g�̗񋓂Ɏ��s" ), _T( "�G���[" ), MB_OK );
	}

	return S_OK;
}

//----------------------------.
// ���͏�����֐�.
//----------------------------.
HRESULT DirectInput::UpdateKeyInput()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  // �֐����A�l.

	// �R���g���[�����璼�ڃf�[�^���擾����.
	hRslt = pI->m_pPad->GetDeviceState(
		sizeof( DIJOYSTATE2 ),	// �擾����T�C�Y.
		&pI->m_JoyState );		// (out)�擾�f�[�^.
	if( hRslt != DI_OK ){
		return hRslt;
	}

	// �O����͏���ޔ�.
	pI->m_StatusOld = pI->m_Status;
	pI->m_Status	= 0;	// ������.

	//======================================.
	// �ȍ~�A�L�[�̓��͎擾.
	//   �{�^���Ƃ��Ă̏����̂ݐݒ肵�Ă���̂ŁA
	//   �X�e�B�b�N�̌X���Ȃǂ�ێ��������ꍇ�́A
	//   �ʓr�����o�ϐ���ǉ����Ċi�[���邱�Ƃ�!!.
	//======================================.

	// �W���C�X�e�B�b�N.
	//	js.lX, js.lY �� ���ꂼ��͈̔͂́u-1000 �` 1000�v.
	//	�y���E�z��-500�`500�̊Ԃ͖����͂Ɣ��肷��.
	if(	pI->m_JoyState.lX < -500 )		{ AddKey( DIRECT_INPUT_KEY::LEFT );		}	// ����.
	else if( 500 < pI->m_JoyState.lX )	{ AddKey( DIRECT_INPUT_KEY::RIGHT );	}	// �E��.
	//	�y�㉺�z��-500�`500�̊Ԃ͖����͂Ɣ��肷��.
	if( pI->m_JoyState.lY < -500 )		{ AddKey( DIRECT_INPUT_KEY::UP );		}	// �㷰.
	else if( 500 < pI->m_JoyState.lY )	{ AddKey( DIRECT_INPUT_KEY::DOWN );		}	// ����.

	// �\���L�[.
	//	�L���l�`�F�b�N.
	if( pI->m_JoyState.rgdwPOV[0] != 0xFFFFFFFF ) {
#if 1
		float rad = ToRadian( static_cast<float>( pI->m_JoyState.rgdwPOV[0] ) / 100.0f );
		// �{���́Ax��cos�Ay��sin�����ArgdwPOV��0���ォ�玞�v���Ȃ̂ŁA
		//	cos��sin���t�ɂ��������s�����悢.
		float x = sinf( rad );
		float y = cosf( rad );
		if( x < -0.01 )		{	AddKey( DIRECT_INPUT_KEY::LEFT );	}
		else if( x > 0.01 )	{	AddKey( DIRECT_INPUT_KEY::RIGHT );	}
		if( y > 0.01 )		{	AddKey( DIRECT_INPUT_KEY::UP );		}
		else if( y <-0.01 )	{	AddKey( DIRECT_INPUT_KEY::DOWN );	}
#else
		// rgdwPOV��0���玞�v���œx����100�{�̒l.
		switch( js.rgdwPOV[0] ){
		case 0:		AddKey( KEY::UP );								break;
		case 4500:	AddKey( KEY::UP );		AddKey( KEY::RIGHT );	break;
		case 9000:	AddKey( KEY::RIGHT );							break;
		case 13500:	AddKey( KEY::DOWN );	AddKey( KEY::RIGHT );	break;
		case 18000:	AddKey( KEY::DOWN );							break;
		case 22500:	AddKey( KEY::DOWN );	AddKey( KEY::LEFT );	break;
		case 27000:	AddKey( KEY::LEFT );							break;
		case 31500:	AddKey( KEY::UP );		AddKey( KEY::LEFT );	break;
		default:													break;
		}
#endif
	}

	for( int i = 0; i < static_cast<int>( DIRECT_INPUT_KEY::BTN_MAX ) + 1; i++ ) {
		//�{�^��.
		if( pI->m_JoyState.rgbButtons[i] & 0x80 ) {
			AddKey( static_cast<DIRECT_INPUT_KEY>( static_cast<int>( DIRECT_INPUT_KEY::BTN_00 ) + i ) );
		}
	}

	return S_OK;
}

//----------------------------.
// �w��L�[�̓��͒ǉ�.
//----------------------------.
void DirectInput::AddKey( DIRECT_INPUT_KEY key )
{
	GetInstance()->m_Status |= 1 << static_cast<unsigned int>( key );
}


//----------------------------.
// IsDown,IsUp,IsRepeat�֐����Ŏg�p����֐�.
//----------------------------.
bool DirectInput::IsKeyCore( DIRECT_INPUT_KEY key, unsigned int status )
{
	if( ( status & ( 1 << static_cast<unsigned int>( key ) ) ) != 0 ){
		return true;
	}
	return false;
}

//-------------------------------------------------.
// �W���C�X�e�B�b�N�񋓊֐�(���[�J���֐�).
//-------------------------------------------------.
BOOL CALLBACK EnumJoysticksCallBack(
	const DIDEVICEINSTANCE* pdidi,
	VOID* pContext )
{
	HRESULT hRslt;  //�֐����A�l.

	//�f�o�C�X(�R���g���[��)�̍쐬.
	hRslt = g_pDICore->CreateDevice(
		pdidi->guidInstance,		//�f�o�C�X�̔ԍ�.
		&g_pPadCore,	//(out)�쐬�����f�o�C�X�I�u�W�F�N�g.
		nullptr );
	if( hRslt != DI_OK ){
		return DIENUM_CONTINUE;	//���̃f�o�C�X��v��.
	}
	//�ŏ��̈��������ΏI��.
	return DIENUM_STOP;	//�񋓒�~.
}

//-------------------------------------------------.
// �I�u�W�F�N�g�̗񋓊֐�(���[�J���֐�).
//-------------------------------------------------.
BOOL CALLBACK EnumObjectsCallBack(
	const DIDEVICEOBJECTINSTANCE* pdidoi,
	VOID* pContext )
{
	HRESULT hr;

	//��(�X�e�B�b�N)�������Ă��邩�H.
	if( pdidoi->dwType & DIDFT_AXIS )
	{
		//���͏��͈̔͂�ݒ肷��.
		DIPROPRANGE diprg;  //�͈͐ݒ�\����.
		ZeroMemory( &diprg, sizeof( diprg ) );

		diprg.diph.dwSize = sizeof( diprg );			//�\���̂̃T�C�Y.
		diprg.diph.dwHeaderSize = sizeof( diprg.diph );	//�w�b�_�[���̃T�C�Y.
		diprg.diph.dwObj = pdidoi->dwType;				//��(�I�u�W�F�N�g).
		diprg.diph.dwHow = DIPH_BYID;					//dwObj�ɐݒ肳�����̂�,
														//�I�u�W�F�N�g�̎�ނ��H�C���X�^���X�ԍ����H.
		diprg.lMin = -1000; // �ŏ��l.
		diprg.lMax = +1000; // �ő�l.

#if 0	//�O������Pad���߲���������Ă����H.
		//�͈͂�ݒ�.
		hr = reinterpret_cast<LPDIRECTINPUTDEVICE8>( pContext )->SetProperty(
			DIPROP_RANGE,   // �͈�.
			&diprg.diph );	// �͈͐ݒ�\����.
#else//
		// �͈͂�ݒ�.
		hr = g_pPadCore->SetProperty(
			DIPROP_RANGE,   // �͈�.
			&diprg.diph );	// �͈͐ݒ�\����.
#endif//
		if( FAILED( hr ) )
		{
			return DIENUM_STOP;
		}
	}
	return DIENUM_CONTINUE;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// 
//	�Q�l�T�C�g.
// 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//http://www.eonet.ne.jp/~maeda/directx9/controller2.htm	//�Q�R���g���[��.
//http://www.eonet.ne.jp/~maeda/directx9/directx9.htm#INPUT	//�R���g���[��.
//http://princess-tiara.biz/directx/?chapter=14				//DirectInput.
//http://www.charatsoft.com/develop/otogema/index.htm		//DirectInput.
//https://yttm-work.jp/directx/directx_0027.html