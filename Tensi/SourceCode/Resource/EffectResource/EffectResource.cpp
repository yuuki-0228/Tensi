#include "EffectResource.h"
#include "..\..\Common\EffectManager\Effect\Effect.h"
#include <encrypt/file.h>

namespace {
	constexpr char FILE_PATH[] = "Data\\Effect"; // �G�t�F�N�g�t�@�C���p�X.
}

EffectResource::EffectResource()
	: m_EffectList		()
	, m_EffectNames		()
	, m_Mutex			()
	, m_IsLoadEnd		( false )
{
}

EffectResource::~EffectResource()
{
	m_EffectList.clear();
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
EffectResource* EffectResource::GetInstance()
{
	static std::unique_ptr<EffectResource> pInstance = std::make_unique<EffectResource>();
	return pInstance.get();
}

//---------------------------.
// �G�t�F�N�g�̓ǂݍ���.
//---------------------------.
HRESULT EffectResource::EffectLoad()
{
	EffectResource* pI = GetInstance();
	std::unique_lock<std::mutex> Lock( pI->m_Mutex );

	auto EffectLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string FilePath	= Entry.path().string();				// �t�@�C���p�X.
		std::string		  Extension = Entry.path().extension().string();	// �g���q.
		std::string		  FileName	= Entry.path().stem().string();			// �t�@�C����.

#ifndef _DEBUG
// �Í����O�̊g�������擾.
		Extension = encrypt::GetExtension( FilePath );
#endif

		// �g���q�� ".efk" �ł͂Ȃ��ꍇ�ǂݍ��܂Ȃ�.
		if ( Extension != ".efk" && Extension != ".EFK" ) return;

#ifndef _DEBUG
		// �s�v�ȕ������폜s
		FileName.erase( 0, 1 );
		FileName.erase( FileName.length() - 2 );
#endif

		// �G�t�F�N�g��ǂݍ��݃��X�g�Ɋi�[����.
		pI->m_EffectList[FileName] = std::make_unique<CEffect>( FilePath );
		pI->m_EffectNames.emplace_back( FileName );
	};

	Log::PushLog( "------ �G�t�F�N�g�ǂݍ��݊J�n ------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, EffectLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// �G���[���b�Z�[�W��\��.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ �G�t�F�N�g�ǂݍ��ݏI�� ------" );

	// �ǂݍ��ݏI��.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//---------------------------.
// �G�t�F�N�g�擾�֐�.
//---------------------------.
CEffect* EffectResource::GetEffect( const std::string& FileName )
{
	// �ǂݍ��݂��I����Ă��Ȃ������� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// �w�肵���G�t�F�N�g���擾.
	for ( auto& m : GetInstance()->m_EffectList ) {
		if ( m.first == FileName ) return m.second.get();
	}

	ErrorMessage( FileName + " effects not found" );
	return nullptr;
}
