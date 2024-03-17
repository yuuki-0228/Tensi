#include "Log.h"
#include "..\FileManager\FileManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace {
	typedef tm TIME_DATA;
	constexpr char LOG_TEXT_FILE_PATH[]			= "Data\\$Log.txt";									// ���O�e�L�X�g�̃t�@�C���p�X.
	constexpr char WINDOW_SETTING_FILE_PATH[]	= "Data\\Parameter\\Config\\WindowSetting.json";	// �E�B���h�E�̐ݒ�̃t�@�C���p�X.
}

Log::Log()
	: m_Stop	( false )
{
}

Log::~Log()
{
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
Log* Log::GetInstance()
{
	static std::unique_ptr<Log> pInstance = std::make_unique<Log>();
	return pInstance.get();
}

//----------------------------.
// ���O�e�L�X�g���쐬.
//----------------------------.
HRESULT Log::OpenLogText()
{
	// ���O���~���邩�擾.
	json WndSetting			= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	GetInstance()->m_Stop	= WndSetting["IsLogStop"];
	if ( GetInstance()->m_Stop ) return S_OK;

	// �t�@�C�����J��.
	std::ofstream LogText;
	LogText.open( LOG_TEXT_FILE_PATH, std::ios::trunc );

	// �t�@�C�����ǂݍ��߂ĂȂ�������I��.
	if( !LogText.is_open() ) return E_FAIL;

	// ���݂̎��Ԃ��l��.
	time_t		nowTime	= time( nullptr );
	TIME_DATA	timeData;	
	// ���[�J�����Ԃɕϊ�.
	localtime_s( &timeData, &nowTime );

	// ���݂̎��Ԃ��e�L�X�g�ɓ���.
	LogText << "[";
	LogText << timeData.tm_year + 1900	<< "/";	// 1900 �������ƂŌ��݂̔N�ɂȂ�.
	LogText << timeData.tm_mon + 1		<< "/";	// 1 �������ƂŌ��݂̔N�ɂȂ�.
	LogText << timeData.tm_mday			<< " ";
	LogText << timeData.tm_hour			<< ":";
	LogText << timeData.tm_min;
	LogText << "]" << " >> App launch" << std::endl << std::endl;

	// �t�@�C�������.
	LogText.close();
	return S_OK;
}

//----------------------------.
// ���O�e�L�X�g�����.
//----------------------------.
HRESULT Log::CloseLogText()
{
	if ( GetInstance()->m_Stop ) return S_OK;

	// �t�@�C����ǂݍ���.
	std::ofstream LogText( LOG_TEXT_FILE_PATH, std::ios::app );

	// �t�@�C�����ǂݍ��߂ĂȂ�������I��.
	if( !LogText.is_open() ) return E_FAIL;

	// ���݂̎��Ԃ��l��.
	time_t		nowTime = time( nullptr );
	TIME_DATA	timeData;
	// ���[�J�����Ԃɕϊ�.
	localtime_s( &timeData, &nowTime );

	// ���݂̎��Ԃ��e�L�X�g�ɓ���.
	LogText << std::endl		<<  "[";
	LogText << timeData.tm_hour	<< ":";
	LogText << timeData.tm_min	<< ":";
	LogText << timeData.tm_sec;
	LogText << "]" << " >> App End" << std::endl;

	// �t�@�C�������.
	LogText.close();

	return S_OK;
}

//----------------------------.
// ���O�t�@�C���̔j��.
//----------------------------.
HRESULT Log::DeleteLogText()
{
	std::filesystem::remove( LOG_TEXT_FILE_PATH );
	return S_OK;
}

//----------------------------.
// ���O�̓���.
//----------------------------.
HRESULT Log::PushLog( const char* Log )
{
	if ( GetInstance()->m_Stop ) return S_OK;

	// �t�@�C�����J��.
	std::ofstream LogText( LOG_TEXT_FILE_PATH, std::ios::app );

	// �t�@�C�����ǂݍ��߂ĂȂ�������I��.
	if( !LogText.is_open() ) return E_FAIL;

	// ���݂̎��Ԃ��l��.
	time_t		nowTime = time( nullptr );
	TIME_DATA	timeData;
	// ���[�J�����Ԃɕϊ�.
	localtime_s( &timeData, &nowTime );

	// ���݂̎��Ԃ��e�L�X�g�ɓ���.
	LogText << "[";
	LogText << timeData.tm_hour << ":";
	LogText << timeData.tm_min	<< ":";
	LogText << timeData.tm_sec;
	// ���O�̓���.
	LogText << "]" << " >> " << Log << std::endl;

	// �t�@�C�������.
	LogText.close();

	return S_OK;
}
HRESULT Log::PushLog( const std::string& Log )
{
	if ( GetInstance()->m_Stop ) return S_OK;

	// �t�@�C�����J��.
	std::ofstream LogText( LOG_TEXT_FILE_PATH, std::ios::app );

	// �t�@�C�����ǂݍ��߂ĂȂ�������I��.
	if( !LogText.is_open() ) return E_FAIL;

	// ���݂̎��Ԃ��l��.
	time_t		nowTime = time( nullptr );
	TIME_DATA	timeData;
	// ���[�J�����Ԃɕϊ�.
	localtime_s( &timeData, &nowTime );

	// ���݂̎��Ԃ��e�L�X�g�ɓ���.
	LogText << "[";
	LogText << timeData.tm_hour << ":";
	LogText << timeData.tm_min	<< ":";
	LogText << timeData.tm_sec;
	// ���O�̓���.
	LogText << "]" << " >> " << Log.c_str() << std::endl;

	// �t�@�C�������.
	LogText.close();

	return S_OK;
}
