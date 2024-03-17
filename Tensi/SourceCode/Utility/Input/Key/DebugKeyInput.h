#pragma once
#include "KeyInput.h"

/************************************************
*	�L�[�{�[�h�̃L�[�̃f�o�b�N�p���̓N���X.
*		���c�F�P.
**/
namespace DebugKeyInput {
	// �L�[�������ꂽ���Ƃ��擾����.
	bool IsKeyPress( const int Key );
	// �������u��.
	bool IsKeyDown( const int Key );
	// �������u��.
	bool IsKeyUp( const int Key );
	// ���������Ă���.
	bool IsKeyRepeat( const int Key );

	// �����̃L�[�̂ǂꂩ�������ꂽ���Ƃ��擾����.
	template<typename... T>
	bool IsORPress( const int Key, T... t );
	// �����̃L�[�̂ǂꂩ���������u��.
	template<typename... T>
	bool IsORKeyDown( const int Key, T... t );
	// �����̃L�[�̂ǂꂩ�𗣂����u��.
	template<typename... T>
	bool IsORKeyUp( const int Key, T... t );
	// �����̃L�[�̂ǂꂩ�����������Ă���.
	template<typename... T>
	bool IsORKeyRepeat( const int Key, T... t );

	// �����̃L�[���S�ĉ����ꂽ���Ƃ��擾����.
	template<typename... T>
	bool IsANDPress( const int Key, T... t );
	// �����̃L�[���������u��.
	//	���Ō�Ɏw�肵���L�[�������ꂽ�u�ԂɑS�ĉ����Ă����K�v������.
	template<typename... T>
	bool IsANDKeyDown( const int Key, T... t );
	// �����̃L�[���������u��.
	//	���Ō�Ɏw�肵���L�[�������ꂽ�u�ԂɑS�ė����Ă����K�v������.
	template<typename... T>
	bool IsANDKeyUp( const int Key, T... t );
	// �����̃L�[���S�ĉ��������Ă���.
	template<typename... T>
	bool IsANDKeyRepeat( const int Key, T... t );
};


//----------------------------.
// �����̃L�[�̂ǂꂩ�������ꂽ���Ƃ��擾����.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsORPress( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsORPress( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �����̃L�[�̂ǂꂩ���������u��.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsORKeyDown( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsORKeyDown( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �����̃L�[�̂ǂꂩ�𗣂����u��.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsORKeyUp( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsORKeyUp( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �����̃L�[�̂ǂꂩ�����������Ă���.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsORKeyRepeat( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsORKeyRepeat( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �����̃L�[���S�ĉ����ꂽ���Ƃ��擾����.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsANDPress( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsANDPress( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �����̃L�[���������u��.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsANDKeyDown( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsANDKeyDown( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �����̃L�[���������u��.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsANDKeyUp( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsANDKeyUp( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �����̃L�[���S�ĉ��������Ă���.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsANDKeyRepeat( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsANDKeyRepeat( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}