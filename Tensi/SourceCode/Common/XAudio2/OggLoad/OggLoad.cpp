// ogg_int64_t を DWORD型に入れる時の警告を消す.
#pragma warning( disable: 4244 )

#include "OggLoad.h"
#ifdef ENABLE_SOUND
#include <memory.h>
#include <crtdbg.h>
#include <tchar.h>
#include "..\..\..\Utility\StringConversion\StringConversion.h"
#ifndef _DEBUG
#include <encrypt/file.h>
#endif

namespace
{
	constexpr unsigned int REQUEST_SIZE = 4096;	// 読み込み単位

#ifndef _DEBUG
	//----------------------------.
	// メモリ上から読み込むためのコールバック群(リリース時のみ使用).
	//----------------------------.
	// メモリから読み込む.
	size_t OggMemRead( void* ptr, size_t size, size_t nmemb, void* datasource )
	{
		COggLoad::SOggMemFile* mem = static_cast<COggLoad::SOggMemFile*>( datasource );
		if ( size == 0 ) return 0;

		// 残りバイト数と要求バイト数を比較し、読み込める要素数を求める.
		const size_t remain  = mem->Size - mem->Pos;
		size_t		 request = size * nmemb;
		if ( request > remain ) request = remain;

		memcpy( ptr, mem->Data + mem->Pos, request );
		mem->Pos += request;
		return request / size;
	}
	// 読み込み位置を移動する.
	int OggMemSeek( void* datasource, ogg_int64_t offset, int whence )
	{
		COggLoad::SOggMemFile* mem = static_cast<COggLoad::SOggMemFile*>( datasource );

		ogg_int64_t newPos = 0;
		switch ( whence ) {
		case SEEK_SET: newPos = offset;											break;
		case SEEK_CUR: newPos = static_cast<ogg_int64_t>( mem->Pos )  + offset;	break;
		case SEEK_END: newPos = static_cast<ogg_int64_t>( mem->Size ) + offset;	break;
		default:	   return -1;
		}
		if ( newPos < 0 || newPos > static_cast<ogg_int64_t>( mem->Size ) ) return -1;

		mem->Pos = static_cast<size_t>( newPos );
		return 0;
	}
	// ファイルを閉じる(メモリ上からのため何もしない).
	int OggMemClose( void* datasource )
	{
		return 0;
	}
	// 現在の読み込み位置を取得する.
	long OggMemTell( void* datasource )
	{
		COggLoad::SOggMemFile* mem = static_cast<COggLoad::SOggMemFile*>( datasource );
		return static_cast<long>( mem->Pos );
	}
#endif
}

COggLoad::COggLoad()
	: m_OggVF			()
	, m_pFile			()
	, m_isReady			( false )
	, m_sFileName		()
	, m_ChannelNumber	()
	, m_SamplingRate	()
	, m_BitRate			()
	, m_SeFileSize		( 0 )
	, m_SeWaveBuffer	( nullptr )
	, m_WaveFormat		( nullptr )
	, m_Size			( 0 )
#ifndef _DEBUG
	, m_OggMemFile		()
	, m_pMemBuffer		( nullptr )
#endif
{
}

COggLoad::~COggLoad()
{
	Close();
}

//----------------------------.
// oggデータ作成.
//----------------------------.
HRESULT COggLoad::CreateOggData( std::string sFilePath, std::string sFileName, const bool& isSE, bool isBuffered )
{
	HRESULT ret;
	if ( FAILED( ret = Open( sFilePath, sFileName, isBuffered ) ) )
	{
		_ASSERT_EXPR( "error ogg file open ret=%d\n", ret );
		return E_FAIL;
	}
	// フラグが立っていればSEの作成.
	if ( isSE == true ) {
		// SEファイルのサイズ
		m_SeFileSize	= GetSize();
		// SEファイルの読み込み用バッファー
		m_SeWaveBuffer	= new BYTE[m_SeFileSize];

		// SEファイルのデータをバッファーに読み込み
		if ( FAILED( ret = Read( m_SeWaveBuffer, m_SeFileSize, &m_SeFileSize ) ) ) {
			_ASSERT_EXPR( "error read SE File %#X\n", ret );
			return E_FAIL;
		}
	}
	return S_OK;
}

//----------------------------.
// ファイルを開く.
//----------------------------.
HRESULT COggLoad::Open( std::string sFilePath, std::string sFileName, bool isBuffered )
{
	if ( sFilePath == "" ) return E_INVALIDARG;
	if ( sFileName == "" ) return E_INVALIDARG;

#ifdef _DEBUG
	// ファイルを検索.
	const std::wstring filePath = StringConversion::to_wString( sFilePath );
	if ( _tfopen_s( &m_pFile, filePath.c_str(), _T( "rb" ) ) != 0 ) {
		// データが存在しない.
		return E_FAIL;
	}

	// Ogg構造体に情報を取り込む.
	errno_t error = ov_open( m_pFile, &m_OggVF, nullptr, 0 );
	if ( error != 0 ) return E_FAIL;
#else
	// リリース時は暗号化されたoggファイルを復号し、メモリから読み込む.
	std::string es = encrypt::GetEncryptionFilePath( sFilePath );
	auto rf = encrypt::GetRestoreFile( es );
	if ( rf.first == nullptr ) return E_FAIL;

	// 破棄までバッファを保持する必要があるためバッファを保持する.
	m_pMemBuffer		= rf.first;
	m_OggMemFile.Data	= rf.first;
	m_OggMemFile.Size	= rf.second;
	m_OggMemFile.Pos	= 0;

	// メモリ読み込み用のコールバックを設定する.
	ov_callbacks callbacks = { OggMemRead, OggMemSeek, OggMemClose, OggMemTell };

	// Ogg構造体に情報を取り込む(メモリ上から).
	errno_t error = ov_open_callbacks( &m_OggMemFile, &m_OggVF, nullptr, 0, callbacks );
	if ( error != 0 ) return E_FAIL;
#endif
	// oggファイル情報を取得してセット
	vorbis_info* info = ov_info( &m_OggVF, -1 );
	// ファイル名セット.
	SetFileName( sFileName );
	// チャンネルナンバーセット.
	SetChannelNumber( info->channels );
	// bitrateは16にしておく
	SetBitRate( 16 );
	// サンプリングレートセット.
	SetSamplingRate( info->rate );
	// セットした情報使ってフォーマットファイル作成
	if ( !CreateWaveFormatEx() ) return E_FAIL;
	// サイズを計算して入れておく
	m_Size = info->channels * 2 * ov_pcm_total( &m_OggVF, -1 );

	// ファイルの準備が出来た
	m_isReady = true;
	return S_OK;
}

//----------------------------.
// ファイルの読み取りポインターをリセットする.
// これを呼んだらOggが初めから再生される.
//----------------------------.
HRESULT COggLoad::ResetFile()
{
	if ( !GetReady() ) return E_FAIL;

	ov_time_seek( &m_OggVF, 0.0 );
	return S_OK;
}

//----------------------------.
// ファイル内容をすべてバッファに読み込み.
//----------------------------.
HRESULT COggLoad::Read( BYTE* buffer, DWORD size, DWORD* readSize )
{
	// 読み込むバッファーをメモリに割り当て
	memset( buffer, 0, this->m_Size );
	char*	tmpBuffer	= new char[this->m_Size];
	int		bitStream	= 0;
	int		tmpReadSize = 0;
	int		comSize		= 0;

	*readSize = 0;
	while ( 1 )
	{
		tmpReadSize = ov_read( &m_OggVF, ( char* )tmpBuffer, 4096, 0, 2, 1, &bitStream );
		// 読み込みサイズ超えてたり、ファイル末尾まで読み込みしてたらbreak
		if ( comSize + tmpReadSize >= static_cast<int>( this->m_Size ) || tmpReadSize == 0 || tmpReadSize == EOF ) break;
		// メモリーのバッファーにポインター位置ずらしながら書き込み
		memcpy( buffer + comSize, tmpBuffer, tmpReadSize );
		comSize += tmpReadSize;
	}

	// 読み込んだサイズをセット
	*readSize = comSize;
	// 後始末
	delete[] tmpBuffer;
	return S_OK;
}

//----------------------------.
// 指定のサイズまで埋めたセグメントデータを取得する.
//----------------------------.
bool COggLoad::GetSegment( char* buffer, unsigned int size, int* writeSize, bool* isEnd )
{
	// ファイルの準備が終わってなければ終了
	if ( GetReady() == false ) return false;

	// バッファーの指定チェック
	if ( buffer == 0 )
	{
		if ( isEnd		) *isEnd		= true;
		if ( writeSize	) *writeSize	= 0;
		return false;
	}
	if ( isEnd ) *isEnd = false;

	// メモリ上にバッファー領域を確保
	memset( buffer, 0, size );
	unsigned int	requestSize	= REQUEST_SIZE;
	int				bitStream	= 0;
	int				readSize	= 0;
	unsigned int	comSize		= 0;
	bool			isAdjust	= false; // 端数のデータの調整中フラグ

	// 指定サイズが予定サイズより小さい場合は調整中とみなす
	if ( size < requestSize )
	{
		requestSize	= size;
		isAdjust	= true;
	}

	// バッファーを指定サイズで埋めるまで繰り返す
	while ( 1 )
	{
		// ファイルエンドに達した
		readSize = ov_read( &m_OggVF, ( char* ) ( buffer + comSize ), requestSize, 0, 2, 1, &bitStream );
		if ( readSize == 0 )
		{
			// 終了
			if ( isEnd		) *isEnd	 = true;
			if ( writeSize	) *writeSize = comSize;
			return true;
		}

		// 読み取りサイズを加算
		comSize += readSize;
		_ASSERT( comSize <= size );	// バッファオーバー

		// バッファを埋め尽くした
		if ( comSize >= size )
		{
			if ( writeSize ) *writeSize = comSize;
			return true;
		}

		// 端数データの調整
		if ( size - comSize < REQUEST_SIZE )
		{
			isAdjust	= true;
			requestSize = size - comSize;
		}
	}

	// エラー
	if ( writeSize ) *writeSize = 0;
	return false;
}

//----------------------------.
// ファイルからチャンクデータを読み込んでバッファを入れる.
//----------------------------.
HRESULT COggLoad::ReadChunk( BYTE** buffer, int bufferCount, const int len, int* readSize )
{
	if ( readSize != NULL ) *readSize = 0;

	// ファイルから指定されたサイズだけデータを読み取る
	bool result = GetSegment( ( char* ) buffer[bufferCount], len, readSize, 0 );
	if ( result == false ) return E_FAIL;
	return S_OK;
}

//----------------------------.
// データ解放.
//----------------------------.
HRESULT COggLoad::Close()
{
	ov_clear( &m_OggVF );	// OggVorbis_File構造体をクリア.
	if ( m_pFile )			// ファイルが開かれている.
	{
		fclose( m_pFile );	// ファイルを閉じる.
		m_pFile = nullptr;	// 中身にnullptr.
	}

	// 解放.
#ifndef _DEBUG
	// メモリ復号済みバッファを解放する(ov_clear後に解放する).
	if ( m_pMemBuffer != nullptr )
	{
		delete[] m_pMemBuffer;
		m_pMemBuffer		= nullptr;
		m_OggMemFile.Data	= nullptr;
		m_OggMemFile.Size	= 0;
		m_OggMemFile.Pos	= 0;
	}
#endif

	SAFE_DELETE( m_SeWaveBuffer );
	SAFE_DELETE( m_WaveFormat	);
	return S_OK;
}

//----------------------------.
// Wavフォーマットデータ作成.
//----------------------------.
const bool COggLoad::CreateWaveFormatEx()
{
	m_WaveFormat = nullptr;
	m_WaveFormat = ( WAVEFORMATEX* )new CHAR[sizeof( WAVEFORMATEX )];
	if ( nullptr == m_WaveFormat ) return false;

	m_WaveFormat->wFormatTag		= WAVE_FORMAT_PCM;					// PCM形式を設定.
	m_WaveFormat->nChannels			= m_ChannelNumber;					// チャンネル数を設定.
	m_WaveFormat->nSamplesPerSec	= m_SamplingRate;					// サンプリング周波数を設定.
	m_WaveFormat->wBitsPerSample	= m_BitRate;						// 量子化ビット数(ビットレート)を設定.
	m_WaveFormat->nBlockAlign		= m_ChannelNumber * m_BitRate / 8;	// データの最小単位を設定.
	m_WaveFormat->nAvgBytesPerSec	= m_WaveFormat->nSamplesPerSec * m_WaveFormat->nBlockAlign;	// 1秒間のバイト数を設定.
	m_WaveFormat->cbSize			= 0;
	return true;
}
#endif // ENABLE_SOUND
