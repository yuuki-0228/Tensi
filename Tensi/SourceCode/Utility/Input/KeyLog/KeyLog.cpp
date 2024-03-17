#include "KeyLog.h"
#include <fstream>
#include <sstream>

namespace {
	constexpr char TEXT_PATH[]	= "Data\\Parameter\\Key.txt";	// �L�[�e�L�X�g�̃t�@�C���p�X.
	constexpr char END_CHAR[]	= "E";							// �I������.
	constexpr char DELIM		= ',';							// ��؂蕶��.
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
		// �e�L�X�g���쐬����( �e�L�X�g������ꍇ������������ ).
		std::ofstream KeyText;
		KeyText.open( TEXT_PATH, std::ios::trunc );

		// �e�L�X�g�����.
		KeyText.close();
	}
}

CKeyLog::~CKeyLog()
{
	if ( m_KeyLogPlay == false ){
		// �e�L�X�g���J��.
		std::ofstream KeyText( TEXT_PATH, std::ios::app );

		// �ȗ����Ȃ�ȗ��t���[�������o��.
		KeyText << std::hex << m_FrameKeyStatus << DELIM << m_Chain << DELIM;

		// �I���������o��.
		KeyText << std::dec << END_CHAR;

		// �e�L�X�g�����.
		KeyText.close();
	}
}

//----------------------------.
// ���O���o�͂���.
//----------------------------.
void CKeyLog::Output()
{
	// �e�L�X�g���J��.
	std::ofstream KeyText( TEXT_PATH, std::ios::app );

	// �O��Ɠ��삪�Ⴄ�ꍇ.
	if ( m_OldFrameKeyStatus != m_FrameKeyStatus ){
		// �e�L�X�g���o��.
		KeyText << std::hex << m_OldFrameKeyStatus << DELIM << m_Chain << DELIM;
		
		// �A���������������.
		m_Chain = 1;
	}
	else{
		// �A��������𑝂₵�Ă���.
		m_Chain++;
	}

	// �e�L�X�g�����.
	KeyText.close();

	// �L�[���O��ޔ���������.
	m_OldFrameKeyStatus = m_FrameKeyStatus;
	m_FrameKeyStatus	= 0;
}

//----------------------------.
// �������L�[��ǉ�����.
//----------------------------.
void CKeyLog::AddKeyState( const int KeyNo )
{
	// �����ꂽ�ꏊ�̃r�b�g��1�ɂ���.
	if ( m_KeyLogPlay == false ) m_FrameKeyStatus |= 1 << KeyNo;
}

//----------------------------.
// �S�ẴL�[���O��ǂݍ���.
//----------------------------.
void CKeyLog::ReadKeyLog()
{
	// �e�L�X�g���J��.
	std::ifstream ifs( TEXT_PATH );

	// 1�s���ǂݍ���.
	std::string line;
	while ( std::getline( ifs, line ) ) {
		// ��؂蕶�����Ƃɓǂݍ��ނ��߂�istringstream�^�ɂ���.
		std::istringstream stream( line );

		// ��؂蕶�����ƂɃf�[�^��ǂݍ���.
		std::string field;
		std::vector<std::string> result;
		while ( std::getline( stream, field, DELIM ) ) {
			result.emplace_back( field );
		}
		
		// ��؂����f�[�^��ۑ�.
		m_KeyLog = result;
	}
	ifs.close();
}

//----------------------------.
// ���̃t���[���̃L�[���O��ǂݍ���.
//----------------------------.
int CKeyLog::ReadFrameKey()
{
	if( m_KeyLogPlay == true && m_KeyLog.at( m_RoadNumCnt ) != END_CHAR ){
		// �O��Ɠ��삪�Ⴄ�ꍇ�A���������������.
		if ( m_OldFrameKeyStatus != m_FrameKeyStatus ) m_Chain = 1;

		// �L�[�̏�Ԃ�ޔ����擾.
		m_OldFrameKeyStatus = m_FrameKeyStatus;
		m_FrameKeyStatus	= strtoul( m_KeyLog.at( m_RoadNumCnt ).c_str(), NULL, 16 );

		// ���݂̃t���[���̃L�[���O��10�i���œn��.
		return m_FrameKeyStatus;
	}
	return 0;
}

//----------------------------.
// �ǂݍ��ރt���[���̃J�E���g�𑝂₷.
//	�I���������O��ƈႤ�L�[�������Ă���ꍇ�͏����͍s��Ȃ�.
//----------------------------.
void CKeyLog::AddFrameCnt()
{
	if ( m_KeyLog.at( m_RoadNumCnt ) == END_CHAR ) return;
	if ( m_OldFrameKeyStatus != m_FrameKeyStatus ) return;
	m_Chain++;

	// �A����������t���[����҂�����.
	const int& NumCnt = m_RoadNumCnt + 1;
	if ( static_cast<int>( strtoul( m_KeyLog.at( NumCnt ).c_str(), NULL, 16 ) ) < m_Chain ){
		m_RoadNumCnt += 2;

		// �L�[���O���I�������烁�b�Z�[�W���o��.
		if ( m_KeyLog.at( m_RoadNumCnt ) == END_CHAR ){
			MessageBox( NULL, TEXT( "�L�[���O�̍Đ����I�����܂���" ), TEXT( "���m�点" ), MB_OK );
		}
	}
}
