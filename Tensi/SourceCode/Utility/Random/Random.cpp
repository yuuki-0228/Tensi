#include "Random.h"
#include <fstream>
#include <sstream>
#include "..\FileManager\FileManager.h"

namespace{
	constexpr char TEXT_PATH[] = "Data\\Parameter\\Config\\Random.json";
}

Random::Random()
	: m_rnd			()
	, m_mt			()
	, m_RandLock	( false )
{
}

Random::~Random()
{
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
Random* Random::GetInstance()
{
	static std::unique_ptr<Random> pInstance = std::make_unique<Random>();
	return pInstance.get();
}

//----------------------------.
// �����̏�����.
//----------------------------.
HRESULT Random::Init()
{
	// �����̏������������擾.
	int InitNum = GetInstance()->m_rnd();

	// �����̌Œ艻���L���Ȃ�.
	if ( GetInstance()->m_RandLock ){
		// �O��̏������̒l�ŏ�����.
		const json& Text = FileManager::JsonLoad( TEXT_PATH );
		InitNum			 = Text["Init"];
	}
	else{
		// �����̏������̒l��ۑ�.
		json j;
		j[".Comment"]	= u8"��������������l";
		j["Init"]		= InitNum;
		FileManager::JsonSave( TEXT_PATH, j );
	}

	// �����̏�����.
	std::mt19937 InitMt( InitNum );
	GetInstance()->m_mt = InitMt;

	Log::PushLog( "�����̏����� : ����" );
	return S_OK;
}

//----------------------------.
// �����_���Ȑ��l( float�^ )���擾.
//	Min�ȏ�AMax������Ԃ�.
//----------------------------.
float Random::GetRand( float min, float max )
{
	// �����Ə�����t�Ȃ��ߓ���ւ���.
	if ( min > max ) std::swap( min, max );

	// �����Ə�����w�肵�����_���Ő��l��Ԃ�.
	std::uniform_real_distribution<float>	Rand_Num( min, max );
	return Rand_Num( GetInstance()->m_mt );
}

//----------------------------.
// �����_���Ȑ��l( int�^ )���擾.
//	Min�ȏ�AMax�ȉ���Ԃ�.
//----------------------------.
int Random::GetRand( int min, int max )
{
	// �����Ə�����t�Ȃ��ߓ���ւ���.
	if ( min > max ) std::swap( min, max );

	// �����Ə�����w�肵�����_���Ő��l��Ԃ�.
	std::uniform_int_distribution<>	Rand_Num( min, max );
	return Rand_Num( GetInstance()->m_mt );
}

//----------------------------.
// �m��( ���� or �Z/�Z ).
//----------------------------.
bool Random::Probability( int Molecule, int Denominator )
{
	return Probability( static_cast<float>( Molecule ) / static_cast<float>( Denominator ) );
}
bool Random::Probability( float Molecule, float Denominator )
{
	return Probability( Molecule / Denominator );
}
bool Random::Probability( float Ratio )
{
	return static_cast<float>( GetRand( 1, 100 ) ) <= Ratio * 100.0f;
}

//----------------------------.
// ��������擾.
//----------------------------.
std::string Random::Shuffle( std::string Text )
{
	auto* pI = GetInstance();

	std::shuffle( Text.begin(), Text.end(), pI->m_mt );
	return Text;
}