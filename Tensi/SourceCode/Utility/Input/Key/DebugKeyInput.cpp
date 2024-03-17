#include "DebugKeyInput.h"

//----------------------------.
// �L�[�������ꂽ���Ƃ��擾����.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
bool DebugKeyInput::IsKeyPress( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyPress( Key );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �������u��.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
bool DebugKeyInput::IsKeyDown( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyDown( Key );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// �������u��.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
bool DebugKeyInput::IsKeyUp( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyUp( Key );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// ���������Ă���.
//	Release�̏ꍇ�͏����͍s��Ȃ�.
//----------------------------.
bool DebugKeyInput::IsKeyRepeat( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyRepeat( Key );
#endif // #ifdef _DEBUG.
	return false;
}