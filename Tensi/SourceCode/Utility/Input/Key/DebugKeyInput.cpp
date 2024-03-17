#include "DebugKeyInput.h"

//----------------------------.
// キーが押されたことを取得する.
//	Releaseの場合は処理は行わない.
//----------------------------.
bool DebugKeyInput::IsKeyPress( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyPress( Key );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// 押した瞬間.
//	Releaseの場合は処理は行わない.
//----------------------------.
bool DebugKeyInput::IsKeyDown( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyDown( Key );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// 離した瞬間.
//	Releaseの場合は処理は行わない.
//----------------------------.
bool DebugKeyInput::IsKeyUp( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyUp( Key );
#endif // #ifdef _DEBUG.
	return false;
}

//----------------------------.
// 押し続けている.
//	Releaseの場合は処理は行わない.
//----------------------------.
bool DebugKeyInput::IsKeyRepeat( const int Key )
{
#ifdef _DEBUG
	return KeyInput::IsKeyRepeat( Key );
#endif // #ifdef _DEBUG.
	return false;
}