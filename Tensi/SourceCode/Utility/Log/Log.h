#pragma once
#include <Windows.h>
#include <memory>
#include <string>
#include <time.h>

/************************************************
*	���O�N���X.
**/
class Log final
{
public:
	Log();
	~Log();

	// ���O�e�L�X�g���쐬.
	//	�A�v���N�����ɍ쐬����.
	static HRESULT OpenLogText();

	// ���O�e�L�X�g�����.
	static HRESULT CloseLogText();

	// ���O�t�@�C���̔j��.
	static HRESULT DeleteLogText();

	// ���O�̓���.
	static HRESULT PushLog( const char*			Log );
	static HRESULT PushLog( const std::string&	Log );

private:
	// �C���X�^���X�̎擾.
	static Log* GetInstance();
	
private:
	bool m_Stop;	// ���O���~���邩.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	Log( const Log& )					= delete;
	Log& operator = ( const Log& )	= delete;
	Log( Log&& )						= delete;
	Log& operator = ( Log&& )			= delete;
};
