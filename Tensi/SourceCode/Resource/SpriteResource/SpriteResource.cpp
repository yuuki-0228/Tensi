#include "SpriteResource.h"
#include "..\..\Utility\FileManager\FileManager.h"

namespace {
	constexpr char FILE_PATH[] = "Data\\Sprite"; // �X�v���C�g�t�@�C���p�X.
}

SpriteResource::SpriteResource()
	: m_SpriteList		()
	, m_SpriteNames		()
	, m_Mutex			()
	, m_IsLoadEnd		( false )
{
}

SpriteResource::~SpriteResource()
{
	m_SpriteList.clear();
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
SpriteResource* SpriteResource::GetInstance()
{
	static std::unique_ptr<SpriteResource> pInstance = std::make_unique<SpriteResource>();
	return pInstance.get();
}

//---------------------------.
// �X�v���C�g�̓ǂݍ���.
//---------------------------.
HRESULT SpriteResource::SpriteLoad()
{
	SpriteResource* pI = GetInstance();
	std::unique_lock<std::mutex> Lock( pI->m_Mutex );
	
	auto SpriteLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string FilePath	= Entry.path().string();				// �t�@�C���p�X.
		std::string		  Extension = Entry.path().extension().string();	// �g���q.
		std::string		  FileName	= Entry.path().stem().string();			// �t�@�C����.

#ifndef _DEBUG
		// �Í����O�̊g�������擾.
		Extension = encrypt::GetExtension( FilePath );
#endif
		// �g���q�� ".png" / ".bmp" / ".jpg" �ł͂Ȃ��ꍇ�ǂݍ��܂Ȃ�.
		if ( Extension != ".png" && Extension != ".PNG" &&
			 Extension != ".bmp" && Extension != ".BMP" &&
			 Extension != ".jpg" && Extension != ".JPG" ) return;
		
#ifndef _DEBUG
		// �s�v�ȕ������폜
		FileName.erase( 0, 1 );
		FileName.erase( FileName.length() - 2 );
#endif

		// �X�v���C�g�t�@�C���̓ǂݍ���.
		pI->m_SpriteList[FileName] = std::make_unique<CSprite>();
		if ( FAILED( pI->m_SpriteList[FileName]->Init( FilePath ) ) ) throw E_FAIL;
		pI->m_SpriteNames.emplace_back( FileName );
	};

	Log::PushLog( "------ �X�v���C�g�ǂݍ��݊J�n ------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SpriteLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// �t�@�C����������Ȃ��G���[�͖�������.
		if ( std::string( e.what() ).find( "�w�肳�ꂽ�p�X��������܂���B" ) != std::string::npos ) return S_OK;

		// �G���[���b�Z�[�W��\��.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	Log::PushLog( "------ �X�v���C�g�ǂݍ��ݏI�� ------" );

	// �ǂݍ��ݏI��.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//---------------------------.
// �X�v���C�g�擾�֐�.
//---------------------------.
CSprite* SpriteResource::GetSprite( const std::string& FileName, SSpriteRenderState* pState )
{
	// �ǂݍ��݂��I����Ă��Ȃ������� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// �w�肵���G�t�F�N�g���擾.
	for ( auto& s : GetInstance()->m_SpriteList ) {
		if ( s.first != FileName	) continue;
		if ( pState  != nullptr		) *pState = s.second->GetRenderState();
		return s.second.get();
	}

	ErrorMessage( FileName + " sounds not found" );
	return nullptr;
}
CSprite* SpriteResource::GetSprite( const std::string& FileName, SSpriteRenderStateList* pState )
{
	CSprite*			Out;
	SSpriteRenderState	State;
	Out = GetSprite( FileName, &State );
	for ( auto& s : *pState ) s = State;
	return Out;
}