#pragma once
#include "..\..\..\Global.h"

/************************************************
*	ドラック＆ドロップクラス.
*		﨑田友輝.
**/
class DragAndDrop final
{
public:
	DragAndDrop();
	~DragAndDrop();

	// ドラック&ドロップを使用する.
	static void Open();
	// ドラック&ドロップを使用しない.
	static void Close();
	
	// ファイルがドロップされたか.
	static bool GetIsDrop() { return !GetInstance()->m_FilePathList.empty(); }
	// ファイルパスを取得.
	static std::vector<std::string> GetFilePath() { return GetInstance()->m_FilePathList; }

	// ファイルパスを追加.
	static void AddFilePath( const std::string& FilePath ) { GetInstance()->m_FilePathList.emplace_back( FilePath ); }

	// ウィンドウハンドルを設定.
	static void SethWnd( const HWND& hWnd ) { GetInstance()->m_hWnd = hWnd; }

private:
	// インスタンスの取得.
	static DragAndDrop* GetInstance();

private:
	HWND						m_hWnd;			// ウィンドウハンドル.
	std::vector<std::string>	m_FilePathList;	// ファイルパスリスト.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	DragAndDrop( const DragAndDrop& )					= delete;
	DragAndDrop( DragAndDrop&& )						= delete;
	DragAndDrop& operator = ( const DragAndDrop& )	= delete;
	DragAndDrop& operator = ( DragAndDrop&& )			= delete;
};