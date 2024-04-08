#pragma once
#include "..\..\..\..\Global.h"
#include "..\..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\KeyBind.h"

#pragma comment( lib, "xinput.lib" )

namespace{
	// �g���K�[���͔͈�.
	static constexpr BYTE	INPUT_TRIGGER_MAX	= 255;
	// �X�e�B�b�N���͔͈�.
	static constexpr SHORT	INPUT_THUMB_MIN		=-32768;
	static constexpr SHORT	INPUT_THUMB_MAX		= 32767;
	// �o�C�u���[�V�����l�͈�.
	static constexpr WORD	INPUT_VIBRATION_MIN	= 0;
	static constexpr WORD	INPUT_VIBRATION_MAX	= 65535;
}

/************************************************
*	Xinput�R���g���[���[�֌W�N���X.
**/
class XInput final
{
public:
	XInput();
	~XInput();

	// �L�[���͂̍X�V.
	static bool Update();

	// �{�^�����~�����邩.
	static void	IsAllKeyStop( const bool Flag ) { GetInstance()->m_ButStop = Flag; }

	// �L�[�������ꂽ���Ƃ��擾����.
	static bool IsKeyPress( const WORD GamePad );
	// �������u��.
	static bool IsKeyDown( const WORD GamePad );
	// �������u��.
	static bool IsKeyUp( const WORD GamePad );
	// ���������Ă���.
	static bool IsKeyRepeat( const WORD GamePad );

	// �����̃L�[�̂ǂꂩ�������ꂽ���Ƃ��擾����.
	template<typename... T>
	static bool IsORPress( WORD Key, T... t );
	// �����̃L�[�̂ǂꂩ���������u��.
	template<typename... T>
	static bool IsORKeyDown( WORD Key, T... t );
	// �����̃L�[�̂ǂꂩ�𗣂����u��.
	template<typename... T>
	static bool IsORKeyUp( WORD Key, T... t );
	// �����̃L�[�̂ǂꂩ�����������Ă���.
	template<typename... T>
	static bool IsORKeyRepeat( WORD Key, T... t );

	// �����̃L�[���S�ĉ����ꂽ���Ƃ��擾����.
	template<typename... T>
	static bool IsANDPress( WORD Key, T... t );
	// �����̃L�[���������u��.
	//	���Ō�Ɏw�肵���L�[�������ꂽ�u�ԂɑS�ĉ����Ă����K�v������.
	template<typename... T>
	static bool IsANDKeyDown( WORD Key, T... t );
	// �����̃L�[���������u��.
	//	���Ō�Ɏw�肵���L�[�������ꂽ�u�ԂɑS�ė����Ă����K�v������.
	template<typename... T>
	static bool IsANDKeyUp( WORD Key, T... t );
	// �����̃L�[���S�ĉ��������Ă���.
	template<typename... T>
	static bool IsANDKeyRepeat( WORD Key, T... t );


	// ���g���K�[�擾.
	static float GetLTrigger( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.bLeftTrigger		) / static_cast<float>( INPUT_TRIGGER_MAX / Max ); }
	// �E�g���K�[�擾.
	static float GetRTrigger( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.bRightTrigger	) / static_cast<float>( INPUT_TRIGGER_MAX / Max ); }

	// ���X�e�B�b�NX���擾.
	static float GetLThumbX( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbLX			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }
	// ���X�e�B�b�NY���擾.
	static float GetLThumbY( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbLY			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }

	// �E�X�e�B�b�NX���擾.
	static float GetRThumbX( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbRX			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }
	// �E�X�e�B�b�NY���擾.
	static float GetRThumbY( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbRY			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }

	static void SetIsVibration( const bool Flag ) { GetInstance()->m_IsVibration = Flag; }
	// �U���ݒ�(0.0f ~ 1.0f).
	static bool SetVibration( const float LMotorSpd, const float RMotorSpd );

	// �ڑ��`�F�b�N.
	static bool IsConnect() { return GetInstance()->m_connect;	}
	// �p�b�h�ԍ��擾.
	static DWORD GetPadID() { return GetInstance()->m_padId;	}

private:
	// �C���X�^���X�̎擾.
	static XInput* GetInstance();

	// �I������.
	static void EndProc();

	// �L�[���͂̍X�V.
	bool UpdateStatus();

	//--------------------.
	//	�{�^���̓��͏�Ԃ��擾����.
	//--------------------.
	// �{�^���������ꂽ���Ƃ��擾����.
	static bool IsButPressed( WORD GamePad );
	// �{�^�����������u��.
	static bool IsButDown( WORD GamePad );
	// �{�^���𗣂����u��.
	static bool IsButUp( WORD GamePad );
	// �{�^�������������Ă���.
	static bool IsButRepeat( WORD GamePad );

	//--------------------.
	//	�X�e�B�b�N�̏�Ԃ��擾����.
	//--------------------.
	// �X�e�B�b�N���|�������Ƃ��擾����.
	static bool IsStiPressed( WORD GamePad );
	// �X�e�B�b�N��|�����u��.
	static bool IsStiDown( WORD GamePad );
	// �X�e�B�b�N��߂����u��.
	static bool IsStiUp( WORD GamePad );
	// �X�e�B�b�N��|�������Ă���.
	static bool IsStiRepeat( WORD GamePad );

	//--------------------.
	//	�g���K�[�̏�Ԃ��擾����.
	//--------------------.
	// �g���K�[�������ꂽ���Ƃ��擾����.
	static bool IsTriPressed( WORD GamePad );
	// �g���K�[���������u��.
	static bool IsTriDown( WORD GamePad );
	// �g���K�[�𗣂����u��.
	static bool IsTriUp( WORD GamePad );
	// �g���K�[�����������Ă���.
	static bool IsTriRepeat( WORD GamePad );

private:
	DWORD				m_padId;			// �p�b�h�ԍ�(0~3).
	XINPUT_STATE		m_state;			// �L�[���͏��.
	XINPUT_STATE		m_stateOld;			// �L�[���͏��(�L�[�X�g���[�N����p).
	XINPUT_VIBRATION	m_vibration;		// �U��.
	CBool				m_IsVibration;		// �U�����g�p���邩.
	CBool				m_ButStop;			// �{�^�����~����.
	bool				m_connect;			// �ڑ�����.
	bool				m_IsNotActiveStop;	// �A�N�e�B�u�E�B���h�E�ł͂Ȃ����ɒ�~�����邩.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	XInput( const XInput& )				= delete;
	XInput( XInput&& )					= delete;
	XInput& operator = ( const XInput& )	= delete;
	XInput& operator = ( XInput&& )		= delete;
};


//----------------------------.
// �����̃L�[�̂ǂꂩ�������ꂽ���Ƃ��擾����.
//----------------------------.
template<typename... T>
bool XInput::IsORPress( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

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
bool XInput::IsORKeyDown( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

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
bool XInput::IsORKeyUp( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

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
bool XInput::IsORKeyRepeat( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

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
bool XInput::IsANDPress( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

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
bool XInput::IsANDKeyDown( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

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
bool XInput::IsANDKeyUp( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

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
bool XInput::IsANDKeyRepeat( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) == false ) return false;
	}
	return true;
}
