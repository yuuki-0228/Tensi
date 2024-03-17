#include "Input.h"
#include "InputList.h"
#include "Key\KeyInput.h"
#include "Controller\XInput\XInput.h"
#include "KeyLog\KeyLog.h"
#include "..\Random\Random.h"
#include "..\FileManager\FileManager.h"
#include "..\..\Scene\FadeManager\FadeManager.h"

namespace {
	constexpr char KEY_BIND_FILE_PATH[]			= "Data\\Parameter\\Config\\KeyBind.json";	// �L�[�o�C���h�̕ۑ��ꏊ.
	constexpr char WINDOW_SETTING_FILE_PATH[]	= "Data\\Parameter\\Config\\WindowSetting.json";	// �E�B���h�E�̐ݒ�̃t�@�C���p�X.
}

Input::Input()
	: m_hWnd				()
	, m_KeyBindList			()
//	, m_pKeyLog				( nullptr )
	, m_MouseNowPoint		( { 0, 0 } )
	, m_MouseOldPoint		( { 0, 0 } )
	, m_MouseWheel			( 0 )
	, m_IsGrab				( false )
	, m_IsNotActiveStop		( false )
	, m_IsUpdateStop		( false )
	, m_IsLeftDoubleClick	( false )
	, m_IsCenterDoubleClick	( false )
	, m_IsRightDoubleClick	( false )
	, m_StartMouseSpeed		( INIT_INT )

	// �L�[���O���Đ��������ꍇ��( true )�ɂ���.
	, m_IsKeyLogPlay	( false )
{
//	m_pKeyLog = std::make_unique<CKeyLog>( m_IsKeyLogPlay );

	// �E�B���h�E�̐ݒ�̎擾.
	json WndSetting		= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	m_IsNotActiveStop	= WndSetting["IsInputNotActiveStop"];

	// �J�n���̃}�E�X���x�̎擾.
	SystemParametersInfo( SPI_GETMOUSESPEED, NULL, &m_StartMouseSpeed, NULL );
}

Input::~Input()
{
	ResetMouseSpeed();
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
Input* Input::GetInstance()
{
	static std::unique_ptr<Input> pInstance = std::make_unique<Input>();
	return pInstance.get();
}

//----------------------------.
// ��Ԃ��X�V.
//----------------------------.
void Input::Update()
{
	if ( GetInstance()->m_IsUpdateStop || FadeManager::GetIsFade() ) return;

	KeyInput	::Update();
	XInput		::Update();

	// �}�E�X�̍X�V.
	UpdateMouse();
}

//----------------------------.
// ���O���o��.
//----------------------------.
void Input::KeyLogOutput()
{
//	Input* pI = GetInstance();
//
//	if( pI->m_IsKeyLogPlay == false )	pI->m_pKeyLog->Output();
//	else								pI->m_pKeyLog->AddFrameCnt();
}

//----------------------------.
// �����ݒ�.
//----------------------------.
HRESULT Input::Init()
{
	Input* pI = GetInstance();

	// ���݂̐ݒ���擾����.
	json KeyBindData = FileManager::JsonLoad( KEY_BIND_FILE_PATH );
	for ( auto& [ActionName, Obj] : KeyBindData.items() ) {
		for ( auto& [KeyType, Value] : Obj.items() ) {
			// �L�[���蓖�Ă̎擾.
			if ( KeyType == "Key" ) {
				for ( auto& v : Value ) pI->m_KeyBindList[ActionName].Key.emplace_back( ToKeyData( v ) );
			}
			// �{�^�����蓖�Ă̎擾.
			else if ( KeyType == "But" ) {
				for ( auto& v : Value ) pI->m_KeyBindList[ActionName].But.emplace_back( ToButData( v ) );
			}
		}
	}

	// ���O���Đ����Ȃ�.
	if ( pI->m_IsKeyLogPlay == true ){
		// �L�[���O��ǂݍ���.
//		pI->m_pKeyLog->ReadKeyLog();

		// �L�[���~�߂�.
		KeyInput	::IsAllKeyStop( true );
		XInput		::IsAllKeyStop( true );

		// �������Œ�ɂ���.
		Random::IsRandLock( true );
	}

	Log::PushLog( "���̓N���X�̏����� : ����" );
	return S_OK;
}

//----------------------------.
// �����ꂽ���Ƃ��擾����.
//----------------------------.
bool Input::IsKeyPress( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// �L�[��������Ă��邩.
	bool IsButPress = false;	// �{�^���������ꂽ��.

	// �L�[�{�[�h��������Ă��邩���ׂ�.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyPress( k );
		if ( IsKeyPress ) break;
	}
	// �R���g���[����������Ă��邩���ׂ�.
	//	�L�[�{�[�h��������Ă����ꍇ���ׂ�K�v���Ȃ����ߏ����͍s��Ȃ�.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyPress( b );
			if ( IsButPress ) break;
		}
	}

	// �z��ԍ����擾.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// ���������ǂ���.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//----------------------------.
// �������u��.
//----------------------------.
bool Input::IsKeyDown( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// �L�[��������Ă��邩.
	bool IsButPress = false;	// �{�^���������ꂽ��.

	// �L�[�{�[�h��������Ă��邩���ׂ�.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyDown( k );
		if ( IsKeyPress ) break;
	}
	// �R���g���[����������Ă��邩���ׂ�.
	//	�L�[�{�[�h��������Ă����ꍇ���ׂ�K�v���Ȃ����ߏ����͍s��Ȃ�.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyDown( b );
			if ( IsButPress ) break;
		}
	}

	// �z��ԍ����擾.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// ���������ǂ���.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//----------------------------.
// �������u��.
//----------------------------.
bool Input::IsKeyUp( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// �L�[��������Ă��邩.
	bool IsButPress = false;	// �{�^���������ꂽ��.

	// �L�[�{�[�h��������Ă��邩���ׂ�.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyUp( k );
		if ( IsKeyPress ) break;
	}
	// �R���g���[����������Ă��邩���ׂ�.
	//	�L�[�{�[�h��������Ă����ꍇ���ׂ�K�v���Ȃ����ߏ����͍s��Ȃ�.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyUp( b );
			if ( IsButPress ) break;
		}
	}

	// �z��ԍ����擾.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// ���������ǂ���.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//----------------------------.
// ���������Ă���.
//----------------------------.
bool Input::IsKeyRepeat( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// �L�[��������Ă��邩.
	bool IsButPress = false;	// �{�^���������ꂽ��.

	// �L�[�{�[�h��������Ă��邩���ׂ�.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyRepeat( k );
		if ( IsKeyPress ) break;
	}
	// �R���g���[����������Ă��邩���ׂ�.
	//	�L�[�{�[�h��������Ă����ꍇ���ׂ�K�v���Ȃ����ߏ����͍s��Ȃ�.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyRepeat( b );
			if ( IsButPress ) break;
		}
	}

	// �z��ԍ����擾.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// ���������ǂ���.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//---------------------------.
// �}�E�X���X�N���[���̒���.
//---------------------------.
bool Input::IsScreenMiddleMouse()
{
	return false;
}

//---------------------------.
// �}�E�X�̑��x��ύX.
//---------------------------.
void Input::SetMouseSpeed( const __int64& Speed )
{
	SystemParametersInfoA( SPI_SETMOUSESPEED, NULL, ( PVOID ) Speed, NULL );
}

//---------------------------.
// �}�E�X�̑��x�����ɖ߂�.
//---------------------------.
void Input::ResetMouseSpeed()
{
	Input* pI = GetInstance();

	SystemParametersInfoA( SPI_SETMOUSESPEED, NULL, ( PVOID ) pI->m_StartMouseSpeed, NULL );
}

//---------------------------.
// �L�[�o�C���h�̃f�[�^�ϊ�.
//---------------------------.
int Input::ToKeyData( std::string Key )
{
	for ( auto& l : InputList::KeyList ) {
		if ( l.first == Key ) return l.second;
	}
	return 0;
}
WORD Input::ToButData( std::string But )
{
	for ( auto& l : InputList::ButList ) {
		if ( l.first == But ) return l.second;
	}
	return 0;
}

//---------------------------.
// �}�E�X�̍X�V.
//---------------------------.
void Input::UpdateMouse()
{
	Input* pI = GetInstance();

	// �O���ݒ�.
	pI->m_MouseOldPoint = pI->m_MouseNowPoint;
	// �}�E�X�J�[�\���ʒu�擾.
	GetCursorPos( &pI->m_MouseNowPoint );
	// �X�N���[�����W���N���C�A���g���W�ɕϊ�.
	ScreenToClient( pI->m_hWnd, &pI->m_MouseNowPoint );
}