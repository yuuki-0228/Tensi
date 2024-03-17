#pragma once
#define DIRECTINPUT_VERSION	0x0800	//�x���������߂Ƀo�[�W�������w��.
#include <dinput.h>	//DirectInput�p.

//DirectX�֌W.
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "dinput8.lib" )
#include "..\..\..\Bool\Bool.h"

enum class DIRECT_INPUT_KEY : int	//���^��int�ɂ���.
{
	UP,		RIGHT,	DOWN,	LEFT,
	BTN_00,	BTN_01, BTN_02, BTN_03,
	BTN_04,	BTN_05, BTN_06, BTN_07,
	BTN_08,	BTN_09, BTN_10, BTN_11,

	MAX,
	BTN_FIRST	= BTN_00,
	BTN_LAST	= BTN_11,
	BTN_MAX		= BTN_LAST - BTN_FIRST,
};

/****************************************
*	DirectInput�N���X.
**/
class DirectInput final
{
public:
	DirectInput();
	~DirectInput();

	// �E�B���h�E�n���h���̐ݒ�.
	static void	SethWnd( HWND hWnd ) { GetInstance()->m_hWnd = hWnd; }
	// ������.
	static bool Init();

	// �L�[���͂̍X�V.
	static HRESULT Update();

	// �{�^�����~�����邩.
	static void	IsAllKeyStop( bool Flag ) { GetInstance()->m_ButStop = Flag; }

	// �L�[�������ꂽ���Ƃ��擾����.
	static bool IsKeyPress( DIRECT_INPUT_KEY Key );
	// �������u��.
	static bool IsKeyDown( DIRECT_INPUT_KEY Key );
	// �������u��.
	static bool IsKeyUp( DIRECT_INPUT_KEY Key );
	// ���������Ă���.
	static bool IsKeyRepeat( DIRECT_INPUT_KEY Key );

	// �����̃L�[�̂ǂꂩ�������ꂽ���Ƃ��擾����.
	template<typename... T>
	static bool IsORPress( DIRECT_INPUT_KEY Key, T... t );
	// �����̃L�[�̂ǂꂩ���������u��.
	template<typename... T>
	static bool IsORKeyDown( DIRECT_INPUT_KEY Key, T... t );
	// �����̃L�[�̂ǂꂩ�𗣂����u��.
	template<typename... T>
	static bool IsORKeyUp( DIRECT_INPUT_KEY Key, T... t );
	// �����̃L�[�̂ǂꂩ�����������Ă���.
	template<typename... T>
	static bool IsORKeyRepeat( DIRECT_INPUT_KEY Key, T... t );

	// �����̃L�[���S�ĉ����ꂽ���Ƃ��擾����.
	template<typename... T>
	static bool IsANDPress( DIRECT_INPUT_KEY Key, T... t );
	// �����̃L�[���������u��.
	//	���Ō�Ɏw�肵���L�[�������ꂽ�u�ԂɑS�ĉ����Ă����K�v������.
	template<typename... T>
	static bool IsANDKeyDown( DIRECT_INPUT_KEY Key, T... t );
	// �����̃L�[���������u��.
	//	���Ō�Ɏw�肵���L�[�������ꂽ�u�ԂɑS�ė����Ă����K�v������.
	template<typename... T>
	static bool IsANDKeyUp( DIRECT_INPUT_KEY Key, T... t );
	// �����̃L�[���S�ĉ��������Ă���.
	template<typename... T>
	static bool IsANDKeyRepeat( DIRECT_INPUT_KEY Key, T... t );

	// �ڑ��`�F�b�N.
	static bool		IsConnect();

private:
	// �C���X�^���X�̎擾.
	static DirectInput* GetInstance();

	// �R���g���[���ڑ��m�F.
	static HRESULT	SettingController();
	// ���͏�����֐�.
	static HRESULT	UpdateKeyInput();
	// �w��L�[�̓��͒ǉ�.
	static void		AddKey( DIRECT_INPUT_KEY key );
	// IsDown,IsUp,IsRepeat�֐����Ŏg�p����֐�.
	static bool		IsKeyCore( DIRECT_INPUT_KEY key, unsigned int status );

private:
	HWND					m_hWnd;			// �E�B���h�E�n���h��.
	LPDIRECTINPUT8			m_pDInput;		// DirectInput�I�u�W�F�N�g.
	LPDIRECTINPUTDEVICE8	m_pPad;			// �R���g���[��.	�������ڑ��Ȃ琔�𑝂₵�đΉ����邱��.

	DIJOYSTATE2				m_JoyState;		// �W���C�X�e�B�b�N���\����.
	unsigned int			m_Status;		// ���͏��.
	unsigned int			m_StatusOld;	// �O��t���[���̓��͏��.

	CBool					m_ButStop;		// �{�^�����~����.
};


//----------------------------.
// �����̃L�[�̂ǂꂩ�������ꂽ���Ƃ��擾����.
//----------------------------.
template<typename... T>
bool DirectInput::IsORPress( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return true;
	}
	return false;
}

//----------------------------.
// �����̃L�[�̂ǂꂩ���������u��.
//----------------------------.
template<typename... T>
bool DirectInput::IsORKeyDown( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyDown( Key ) ) return true;
	}
	return false;
}

//----------------------------.
// �����̃L�[�̂ǂꂩ�𗣂����u��.
//----------------------------.
template<typename... T>
bool DirectInput::IsORKeyUp( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyUp( k ) ) return true;
	}
	return false;
}

//----------------------------.
// �����̃L�[�̂ǂꂩ�����������Ă���.
//----------------------------.
template<typename... T>
bool DirectInput::IsORKeyRepeat( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) ) return true;
	}
	return false;
}

//----------------------------.
// �����̃L�[���S�ĉ����ꂽ���Ƃ��擾����.
//----------------------------.
template<typename... T>
bool DirectInput::IsANDPress( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// �����̃L�[���������u��.
//----------------------------.
template<typename... T>
bool DirectInput::IsANDKeyDown( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	if ( IsKeyDown( KeyList.end() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// �����̃L�[���������u��.
//----------------------------.
template<typename... T>
bool DirectInput::IsANDKeyUp( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	if ( IsKeyUp( KeyList.end() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return false;
	}
	return true;
}

//----------------------------.
// �����̃L�[���S�ĉ��������Ă���.
//----------------------------.
template<typename... T>
bool DirectInput::IsANDKeyRepeat( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) == false ) return false;
	}
	return true;
}
