#include "FontResource.h"
#include "..\..\Common\Font\FontCreate\FontCreate.h"
#include "..\..\Utility\FileManager\FileManager.h"

namespace {
	// �t�H���g�t�@�C���p�X.
	constexpr char FILE_PATH[] = "Data\\Sprite\\Font";
}

FontResource::FontResource()
	: m_FontList		()
	, m_FontTextureList	()
	, m_FontDataList	()
	, m_FontNames		()
	, m_Mutex			()
	, m_IsLoadEnd		( false )
{
}

FontResource::~FontResource()
{
	for ( auto& [FileName, List] : m_FontTextureList ) {
		for ( auto& [Key, Texture] : List ) {
			SAFE_RELEASE( Texture );
		}
	}
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
FontResource* FontResource::GetInstance()
{
	static std::unique_ptr<FontResource> pInstance = std::make_unique<FontResource>();
	return pInstance.get();
}

//---------------------------.
// �t�H���g�̓ǂݍ���.
//---------------------------.
HRESULT FontResource::FontLoad()
{
	FontResource* pI = GetInstance();
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

		// �g���q�� ".ttf" / ".otf" �ł͂Ȃ��ꍇ�ǂݍ��܂Ȃ�.
		if ( Extension != ".ttf" && Extension != ".TTF" &&
			 Extension != ".otf" && Extension != ".OTF" ) return;

#ifndef _DEBUG
		// �s�v�ȕ������폜s
		FileName.erase( 0, 1 );
		FileName.erase( FileName.length() - 2 );
#endif

		// �t�H���g�f�[�^��ۑ�.
		pI->m_FontDataList[FileName] = FilePath;

		// �t�H���g�̓ǂݍ���.
		pI->m_FontList[FileName] = std::make_unique<CFont>();
		if ( FAILED( pI->m_FontList[FileName]->Init( FilePath, FileName ) ) ) throw E_FAIL;
		pI->m_FontNames.emplace_back( FileName );

		Log::PushLog( FilePath + " �ǂݍ��� : ����" );
	};

	Log::PushLog( "------ �t�H���g�ǂݍ��݊J�n ------" );
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
	Log::PushLog( "------ �t�H���g�ǂݍ��ݏI�� ------" );

	// �ǂݍ��ݏI��.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//---------------------------.
// �t�H���g�̎擾.
//---------------------------.
CFont* FontResource::GetFont( const std::string& FileName, SFontRenderState* pState )
{
	// �ǂݍ��݂��I����Ă��Ȃ������� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// �w�肵���G�t�F�N�g���擾.
	for ( auto& s : GetInstance()->m_FontList ) {
		if ( s.first != FileName	) continue;
		if ( pState  != nullptr		) *pState = s.second->GetRenderState();
		return s.second.get();
	}

	ErrorMessage( FileName + " sounds not found" );
	return nullptr;
}
CFont* FontResource::GetFont( const std::string& FileName, SFontRenderStateList* pState )
{
	CFont*				Out;
	SFontRenderState	State;
	Out = GetFont( FileName, &State );
	for ( auto& s : *pState ) s = State;
	return Out;
}

//---------------------------.
// �t�H���g�e�N�X�`���擾�֐�.
//---------------------------.
ID3D11ShaderResourceView* FontResource::GetFontTexture( const std::string& FileName, const std::string& Key )
{
	// �ǂݍ��݂��I����Ă��Ȃ������� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false ) 
		return nullptr;

	// �w�肵�����f�����擾.
	for( auto& m : GetInstance()->m_FontTextureList[FileName] ){
		if( m.first == Key ) return m.second;
	}

	// �܂��쐬���Ă��Ȃ������̂��ߍ쐬����.
	if( FAILED( CreateTexture( FileName, Key ) ) ) return nullptr;
	return GetFontTexture( FileName, Key );
}

//---------------------------.
// �e�N�X�`���̍쐬.
//---------------------------.
HRESULT FontResource::CreateTexture( const std::string& FileName, const std::string& Key )
{
	FontResource* pI = GetInstance();
	
	// �쐬���镶�����擾.
	std::string f = Key.substr( 0, 1 );
	if ( IsDBCSLeadByte( Key[0] ) == TRUE ) {
		f = Key.substr( 0, 2 );	// �S�p����.
	}
	else {
		f = Key.substr( 0, 1 );	// ���p����.
	}

	// ���łɍ쐬�ς݂�.
	const bool KeyFind = ( pI->m_FontTextureList[FileName].find( f ) != pI->m_FontTextureList[FileName].end() );
	if ( KeyFind ) return E_FAIL;

	// �e�N�X�`���̍쐬.
	std::unique_ptr<CFontCreate> pFontCreate = std::make_unique<CFontCreate>( pI->m_FontDataList[FileName], FileName );
	pFontCreate->CreateFontTexture2D( f.c_str(), &pI->m_FontTextureList[FileName][f] );

	Log::PushLog( "�t�H���g�u" + FileName + "�v�́u" + Key + "�v�e�N�X�`���쐬 : ����");
	return S_OK;
}
