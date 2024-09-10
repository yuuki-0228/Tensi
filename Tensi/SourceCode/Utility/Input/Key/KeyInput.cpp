#include "KeyInput.h"
#include "..\..\FileManager\FileManager.h"

namespace {
	constexpr char WINDOW_SETTING_FILE_PATH[] = "Data\\Parameter\\Config\\WindowSetting.json";	// �E�B���h�E�̐ݒ�̃t�@�C���p�X.
}

KeyInput::KeyInput()
	: m_NowState		()
	, m_OldState		()
#ifdef ENABLE_CLASS_BOOL
	, m_KeyStop			( false, u8"�L�[�{�[�h�̃L�[���~���邩", "Input" )
#else
	, m_KeyStop			( false )
#endif
	, m_IsNotActiveStop	( false )
{
	// �E�B���h�E�̐ݒ�̎擾.
	json WndSetting		= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	m_IsNotActiveStop	= WndSetting["IsInputNotActiveStop"];
}

KeyInput::~KeyInput()
{
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
KeyInput* KeyInput::GetInstance()
{
	static std::unique_ptr<KeyInput> pInstance = std::make_unique<KeyInput>();
	return pInstance.get();
}

//----------------------------.
// ��Ԃ��X�V.
//----------------------------.
void KeyInput::Update()
{
	KeyInput* pI = GetInstance();

	// ��Ԃ��X�V����O�Ɍ��݂̏�Ԃ��R�s�[.
	memcpy_s( pI->m_OldState, sizeof( m_OldState ), pI->m_NowState, sizeof( m_NowState ) );

	// �����Ă���L�[�𒲂ׂ�.
	if( !GetKeyboardState( pI->m_NowState ) ) return;
}

//----------------------------.
// �L�[�������ꂽ���Ƃ��擾����.
//	�L�[����~���Ȃ珈���͍s��Ȃ�.
//----------------------------.
bool KeyInput::IsKeyPress( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	if (		pI->m_KeyStop == true ) return false;
	return (	pI->m_NowState[Key] & 0x80 ) != 0;
}

//----------------------------.
// �������u��.
//	�L�[����~���Ȃ珈���͍s��Ȃ�.
//----------------------------.
bool KeyInput::IsKeyDown( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	// ���ݓ��͂őO����͂��ĂȂ����.
	if (		pI->m_KeyStop == true ) return false;
	return	(	pI->m_OldState[Key] & 0x80 ) == 0 &&
			(	pI->m_NowState[Key] & 0x80 ) != 0;
}

//----------------------------.
// �������u��.
//	�L�[����~���Ȃ珈���͍s��Ȃ�.
//----------------------------.
bool KeyInput::IsKeyUp( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	// ���݂����͂őO������͂Ȃ�.
	if (		pI->m_KeyStop == true ) return false;
	return	(	pI->m_OldState[Key] & 0x80 ) != 0 &&
			(	pI->m_NowState[Key] & 0x80 ) == 0;
}

//----------------------------.
// ���������Ă���.
//	�L�[����~���Ȃ珈���͍s��Ȃ�.
//----------------------------.
bool KeyInput::IsKeyRepeat( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	// ���݂����͂őO������͂Ȃ�.
	if (		pI->m_KeyStop == true ) return false;
	return	(	pI->m_OldState[Key] & 0x80 ) != 0 &&
			(	pI->m_NowState[Key] & 0x80 ) != 0;
}
