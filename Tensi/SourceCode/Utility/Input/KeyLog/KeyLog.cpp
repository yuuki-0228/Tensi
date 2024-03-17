#include "KeyLog.h"
#include <fstream>
#include <sstream>

namespace {
	constexpr char TEXT_PATH[]	= "Data\\Parameter\\Key.txt";	// キーテキストのファイルパス.
	constexpr char END_CHAR[]	= "E";							// 終了文字.
	constexpr char DELIM		= ',';							// 区切り文字.
}

CKeyLog::CKeyLog()
	: CKeyLog( false )
{}

CKeyLog::CKeyLog( const bool IsPlay )
	: m_KeyLog				()
	, m_OldFrameKeyStatus	( 0 )
	, m_FrameKeyStatus		( 0 )
	, m_RoadNumCnt			( 0 )
	, m_Chain				( 1 )
	, m_KeyLogPlay			( IsPlay )
{
	if ( m_KeyLogPlay == false ){
		// テキストを作成する( テキストがある場合を初期化する ).
		std::ofstream KeyText;
		KeyText.open( TEXT_PATH, std::ios::trunc );

		// テキストを閉じる.
		KeyText.close();
	}
}

CKeyLog::~CKeyLog()
{
	if ( m_KeyLogPlay == false ){
		// テキストを開く.
		std::ofstream KeyText( TEXT_PATH, std::ios::app );

		// 省略中なら省略フレーム数を出力.
		KeyText << std::hex << m_FrameKeyStatus << DELIM << m_Chain << DELIM;

		// 終了文字を出力.
		KeyText << std::dec << END_CHAR;

		// テキストを閉じる.
		KeyText.close();
	}
}

//----------------------------.
// ログを出力する.
//----------------------------.
void CKeyLog::Output()
{
	// テキストを開く.
	std::ofstream KeyText( TEXT_PATH, std::ios::app );

	// 前回と動作が違う場合.
	if ( m_OldFrameKeyStatus != m_FrameKeyStatus ){
		// テキストを出力.
		KeyText << std::hex << m_OldFrameKeyStatus << DELIM << m_Chain << DELIM;
		
		// 連続する個数を初期化.
		m_Chain = 1;
	}
	else{
		// 連続する個数を増やしていく.
		m_Chain++;
	}

	// テキストを閉じる.
	KeyText.close();

	// キーログを退避し初期化.
	m_OldFrameKeyStatus = m_FrameKeyStatus;
	m_FrameKeyStatus	= 0;
}

//----------------------------.
// 押したキーを追加する.
//----------------------------.
void CKeyLog::AddKeyState( const int KeyNo )
{
	// 押された場所のビットを1にする.
	if ( m_KeyLogPlay == false ) m_FrameKeyStatus |= 1 << KeyNo;
}

//----------------------------.
// 全てのキーログを読み込む.
//----------------------------.
void CKeyLog::ReadKeyLog()
{
	// テキストを開く.
	std::ifstream ifs( TEXT_PATH );

	// 1行ずつ読み込む.
	std::string line;
	while ( std::getline( ifs, line ) ) {
		// 区切り文字ごとに読み込むためにistringstream型にする.
		std::istringstream stream( line );

		// 区切り文字ごとにデータを読み込む.
		std::string field;
		std::vector<std::string> result;
		while ( std::getline( stream, field, DELIM ) ) {
			result.emplace_back( field );
		}
		
		// 区切ったデータを保存.
		m_KeyLog = result;
	}
	ifs.close();
}

//----------------------------.
// このフレームのキーログを読み込む.
//----------------------------.
int CKeyLog::ReadFrameKey()
{
	if( m_KeyLogPlay == true && m_KeyLog.at( m_RoadNumCnt ) != END_CHAR ){
		// 前回と動作が違う場合連続する個数を初期化.
		if ( m_OldFrameKeyStatus != m_FrameKeyStatus ) m_Chain = 1;

		// キーの状態を退避し取得.
		m_OldFrameKeyStatus = m_FrameKeyStatus;
		m_FrameKeyStatus	= strtoul( m_KeyLog.at( m_RoadNumCnt ).c_str(), NULL, 16 );

		// 現在のフレームのキーログを10進数で渡す.
		return m_FrameKeyStatus;
	}
	return 0;
}

//----------------------------.
// 読み込むフレームのカウントを増やす.
//	終了文字か前回と違うキーを押している場合は処理は行わない.
//----------------------------.
void CKeyLog::AddFrameCnt()
{
	if ( m_KeyLog.at( m_RoadNumCnt ) == END_CHAR ) return;
	if ( m_OldFrameKeyStatus != m_FrameKeyStatus ) return;
	m_Chain++;

	// 連続する個数分フレームを待ったら.
	const int& NumCnt = m_RoadNumCnt + 1;
	if ( static_cast<int>( strtoul( m_KeyLog.at( NumCnt ).c_str(), NULL, 16 ) ) < m_Chain ){
		m_RoadNumCnt += 2;

		// キーログが終了したらメッセージを出す.
		if ( m_KeyLog.at( m_RoadNumCnt ) == END_CHAR ){
			MessageBox( NULL, TEXT( "キーログの再生が終了しました" ), TEXT( "お知らせ" ), MB_OK );
		}
	}
}
