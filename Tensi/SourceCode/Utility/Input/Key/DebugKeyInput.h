#pragma once
#include "KeyInput.h"

/************************************************
*	キーボードのキーのデバック用入力クラス.
*		﨑田友輝.
**/
namespace DebugKeyInput {
	// キーが押されたことを取得する.
	bool IsKeyPress( const int Key );
	// 押した瞬間.
	bool IsKeyDown( const int Key );
	// 離した瞬間.
	bool IsKeyUp( const int Key );
	// 押し続けている.
	bool IsKeyRepeat( const int Key );

	// 複数個のキーのどれかが押されたことを取得する.
	template<typename... T>
	bool IsORPress( const int Key, T... t );
	// 複数個のキーのどれかを押した瞬間.
	template<typename... T>
	bool IsORKeyDown( const int Key, T... t );
	// 複数個のキーのどれかを離した瞬間.
	template<typename... T>
	bool IsORKeyUp( const int Key, T... t );
	// 複数個のキーのどれかを押し続けている.
	template<typename... T>
	bool IsORKeyRepeat( const int Key, T... t );

	// 複数個のキーが全て押されたことを取得する.
	template<typename... T>
	bool IsANDPress( const int Key, T... t );
	// 複数個のキーが押した瞬間.
	//	※最後に指定したキーが押された瞬間に全て押しておく必要がある.
	template<typename... T>
	bool IsANDKeyDown( const int Key, T... t );
	// 複数個のキーが離した瞬間.
	//	※最後に指定したキーが離された瞬間に全て離しておく必要がある.
	template<typename... T>
	bool IsANDKeyUp( const int Key, T... t );
	// 複数個のキーが全て押し続けている.
	template<typename... T>
	bool IsANDKeyRepeat( const int Key, T... t );
};


//----------------------------.
// 複数個のキーのどれかが押されたことを取得する.
//	Releaseの場合は処理は行わない.
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
// 複数個のキーのどれかを押した瞬間.
//	Releaseの場合は処理は行わない.
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
// 複数個のキーのどれかを離した瞬間.
//	Releaseの場合は処理は行わない.
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
// 複数個のキーのどれかを押し続けている.
//	Releaseの場合は処理は行わない.
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
// 複数個のキーが全て押されたことを取得する.
//	Releaseの場合は処理は行わない.
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
// 複数個のキーが押した瞬間.
//	Releaseの場合は処理は行わない.
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
// 複数個のキーが離した瞬間.
//	Releaseの場合は処理は行わない.
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
// 複数個のキーが全て押し続けている.
//	Releaseの場合は処理は行わない.
//----------------------------.
template<typename... T>
bool DebugKeyInput::IsANDKeyRepeat( const int Key, T... t )
{
#ifdef _DEBUG
	return KeyInput::IsANDKeyRepeat( Key, t... );
#endif // #ifdef _DEBUG.
	return false;
}