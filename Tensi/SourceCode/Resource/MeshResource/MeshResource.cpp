#include "MeshResource.h"
#include "..\..\Common\Mesh\StaticMesh\StaticMesh.h"
#include "..\..\Common\Mesh\SkyBoxMesh\SkyBoxMesh.h"
#include "..\..\Common\Mesh\CollisionMesh\CollisionMesh.h"
#include <encrypt/file.h>

namespace {
	constexpr char FILE_PATH[] = "Data\\Mesh";	// ���b�V���t�@�C���p�X.
}

MeshResource::MeshResource()
	: m_CollisionMeshList	()
	, m_SkyBoxMeshList		()
	, m_StaticMeshList		()
	, m_SkinMeshList		()
	, m_CollisionMeshNames	()
	, m_SkyBoxMeshNames		()
	, m_StaticMeshNames		()
	, m_SkinMeshNames		()
	, m_IsLoadEnd			( false )
	, m_TexturePathList		()
{
}

MeshResource::~MeshResource()
{
	m_SkinMeshList.clear();
	m_StaticMeshList.clear();
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
MeshResource* MeshResource::GetInstance()
{
	static std::unique_ptr<MeshResource> pInstance = std::make_unique<MeshResource>();
	return pInstance.get();
}

//---------------------------.
// ���f���̓ǂݍ���.
//---------------------------.
HRESULT MeshResource::MeshLoad()
{
	MeshResource* pI = GetInstance();
	std::unique_lock<std::mutex> lock( pI->m_Mutex );

	auto MeshLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		std::string		  Extension = Entry.path().extension().string();	// �g���q.
		std::string		  FileName	= Entry.path().stem().string();			// �t�@�C����.
		const std::string FilePath	= Entry.path().string();				// �t�@�C���p�X.
		const std::string LoadMsg	= FilePath + " �ǂݍ��� : ����";			// ���炩���߃��[�h�������b�Z�[�W��ݒ肷��.
		const std::string ErrorMsg	= FileName + " : X �t�@�C���ǂݍ��ݎ��s";	// ���炩���߃G���[���b�Z�[�W��ݒ肷��.

#ifndef _DEBUG
// �Í����O�̊g�������擾.
		Extension = encrypt::GetExtension( FilePath );
#endif

		// �g���q�� ".x" �ł͂Ȃ��ꍇ�ǂݍ��܂Ȃ�.
		if ( Extension != ".x" && Extension != ".X" ) return;

#ifndef _DEBUG
		// �s�v�ȕ������폜s
		FileName.erase( 0, 1 );
		FileName.erase( FileName.length() - 2 );
#endif

		// �t�@�C�����̍Ō�� "_s" �����邽�߃X�L�����b�V���Ƃ��ĕۑ�.
		const std::wstring wFilePath = StringConversion::to_wString( FilePath );
		if ( FileName.substr( FileName.length() - 2 ) == "_s" ) {
			// ���b�V���̓ǂݍ���.
			pI->m_SkinMeshList[FileName] = std::make_unique<CSkinMesh>();
			if ( FAILED( pI->m_SkinMeshList[FileName]->Init( wFilePath.c_str() ) ) )
				throw ErrorMsg.c_str();
			pI->m_SkinMeshNames.emplace_back( FileName );
		}
		// �t�@�C�����̍Ō�� "_c" �����邽�߃R���W�������b�V���Ƃ��ĕۑ�.
		else if ( FileName.substr( FileName.length() - 2 ) == "_c" ) {
			// ���b�V���̓ǂݍ���.
			pI->m_CollisionMeshList[FileName] = std::make_unique<CCollisionMesh>();
			if ( FAILED( pI->m_CollisionMeshList[FileName]->Init( wFilePath.c_str() ) ) )
				throw ErrorMsg.c_str();
			pI->m_CollisionMeshNames.emplace_back( FileName );
		}
		// �t�@�C�����̍Ō�� "_b" �����邽�߃X�J�C�{�b�N�X���b�V���Ƃ��ĕۑ�.
		else if ( FileName.substr( FileName.length() - 2 ) == "_b" ) {
			// ���b�V���̓ǂݍ���.
			pI->m_SkyBoxMeshList[FileName] = std::make_unique<CSkyBoxMesh>();
			if ( FAILED( pI->m_SkyBoxMeshList[FileName]->Init( wFilePath.c_str() ) ) )
				throw ErrorMsg.c_str();
			pI->m_SkyBoxMeshNames.emplace_back( FileName );
		}
		// ����ȊO�̂��߃X�^�e�B�b�N���b�V���Ƃ��ĕۑ�.
		else {
			// ���b�V���̓ǂݍ���.
			pI->m_StaticMeshList[FileName] = std::make_unique<CStaticMesh>();
			if ( FAILED( pI->m_StaticMeshList[FileName]->Init( wFilePath.c_str() ) ) )
				throw ErrorMsg.c_str();
			pI->m_StaticMeshNames.emplace_back( FileName );
		}
		Log::PushLog( LoadMsg.c_str() );
	};

	Log::PushLog( "------ ���b�V���ǂݍ��݊J�n ------" );
	try {
		std::filesystem::recursive_directory_iterator dir_itr( FILE_PATH ), end_itr;
		std::for_each( dir_itr, end_itr, MeshLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// �G���[���b�Z�[�W��\��.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	catch ( const char* e ) {
		// �G���[���b�Z�[�W��\��.
		ErrorMessage( e );
		return E_FAIL;
	}
	Log::PushLog( "------ ���b�V���ǂݍ��ݏI�� ------" );

	// �ǂݍ��ݏI��.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//---------------------------.
// �R���W�������b�V���̎擾.
//---------------------------.
CCollisionMesh* MeshResource::GetCollision( const std::string& name )
{
	// ���f���̓ǂݍ��݂��I�����ĂȂ��̂� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// �w�肵�����f����Ԃ�.
	for ( auto& m : GetInstance()->m_CollisionMeshList ) {
		if ( m.first == name ) return m.second.get();
	}

	ErrorMessage( name + " mesh not found" );
	return nullptr;
}

//---------------------------.
// �X�J�C�{�b�N�X���b�V���̎擾.
//---------------------------.
CSkyBoxMesh* MeshResource::GetSkyBox( const std::string& name )
{
	// ���f���̓ǂݍ��݂��I�����ĂȂ��̂� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// �w�肵�����f����Ԃ�.
	for ( auto& m : GetInstance()->m_SkyBoxMeshList ) {
		if ( m.first == name ) return m.second.get();
	}

	ErrorMessage( name + " mesh not found" );
	return nullptr;
}

//---------------------------.
// �X�^�e�B�b�N���b�V���̎擾.
//---------------------------.
CStaticMesh* MeshResource::GetStatic( const std::string& name )
{
	// ���f���̓ǂݍ��݂��I�����ĂȂ��̂� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// �w�肵�����f����Ԃ�.
	for ( auto& m : GetInstance()->m_StaticMeshList ) {
		if ( m.first == name ) return m.second.get();
	}

	ErrorMessage( name + " mesh not found" );
	return nullptr;
}

//---------------------------.
// �X�L�����b�V���̎擾.
//---------------------------.
CSkinMesh* MeshResource::GetSkin( const std::string& name, SAnimationController* pOutAC )
{
	// ���f���̓ǂݍ��݂��I�����ĂȂ��̂� null ��Ԃ�.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// �w�肵�����f����Ԃ�.
	for ( auto& m : GetInstance()->m_SkinMeshList ) {
		if ( m.first != name ) continue;

		// �A�j���[�V�����R���g���[���[�̃R�s�[���쐬����.
		if ( pOutAC != nullptr ) {
			LPD3DXANIMATIONCONTROLLER pAC = m.second.get()->GetAnimationController();
			if ( FAILED( pOutAC->SetAnimController( pAC ) ) ) return nullptr;
		}
		return m.second.get();
	}

	ErrorMessage( name + " mesh not found" );
	return nullptr;
}
