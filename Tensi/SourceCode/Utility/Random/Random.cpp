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
// インスタンスの取得.
//----------------------------.
Random* Random::GetInstance()
{
	static std::unique_ptr<Random> pInstance = std::make_unique<Random>();
	return pInstance.get();
}

//----------------------------.
// 乱数の初期化.
//----------------------------.
HRESULT Random::Init()
{
	// 乱数の初期化数字を取得.
	int InitNum = GetInstance()->m_rnd();

	// 乱数の固定化が有効なら.
	if ( GetInstance()->m_RandLock ){
		// 前回の初期化の値で初期化.
		const json& Text = FileManager::JsonLoad( TEXT_PATH );
		InitNum			 = Text["Init"];
	}
	else{
		// 乱数の初期化の値を保存.
		json j;
		j[".Comment"]	= u8"乱数を初期する値";
		j["Init"]		= InitNum;
		FileManager::JsonSave( TEXT_PATH, j );
	}

	// 乱数の初期化.
	std::mt19937 InitMt( InitNum );
	GetInstance()->m_mt = InitMt;

	Log::PushLog( "乱数の初期化 : 成功" );
	return S_OK;
}

//----------------------------.
// ランダムな数値( float型 )を取得.
//	Min以上、Max未満を返す.
//----------------------------.
float Random::GetRand( float min, float max )
{
	// 下限と上限が逆なため入れ替える.
	if ( min > max ) std::swap( min, max );

	// 下限と上限を指定しランダムで数値を返す.
	std::uniform_real_distribution<float>	Rand_Num( min, max );
	return Rand_Num( GetInstance()->m_mt );
}

//----------------------------.
// ランダムな数値( int型 )を取得.
//	Min以上、Max以下を返す.
//----------------------------.
int Random::GetRand( int min, int max )
{
	// 下限と上限が逆なため入れ替える.
	if ( min > max ) std::swap( min, max );

	// 下限と上限を指定しランダムで数値を返す.
	std::uniform_int_distribution<>	Rand_Num( min, max );
	return Rand_Num( GetInstance()->m_mt );
}

//----------------------------.
// 確率( 割合 or 〇/〇 ).
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
// 文字列を取得.
//----------------------------.
std::string Random::Shuffle( std::string Text )
{
	auto* pI = GetInstance();

	std::shuffle( Text.begin(), Text.end(), pI->m_mt );
	return Text;
}