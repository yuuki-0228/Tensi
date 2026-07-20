#include "FbxLoader.h"
#ifdef ENABLE_MESH
#include "..\..\..\Global.h"
#include <ufbx/ufbx.h>
#include <encrypt/file.h>
#include <filesystem>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <cctype>

namespace
{
	// アニメーションのサンプリングレート(1秒あたりのキー数).
	constexpr double	ANIM_SAMPLE_RATE	= 60.0;
	// 最大アニメーションセット数(SkinMeshParser の MAX_ANIM_SET に合わせる).
	constexpr DWORD		MAX_ANIM_SET		= 100;
	// 最大ボーン数(SkinMeshParser の MAX_BONES に合わせる).
	constexpr DWORD		MAX_BONES			= 255;
	// 16bitインデックスで扱える最大頂点数.
	constexpr DWORD		MAX_VERTEX_16BIT	= 0xFFFE;

	// 頂点フォーマット(既存メッシュクラスの頂点レイアウトと一致させる).
	constexpr DWORD MESH_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

	// FVF頂点構造体.
	struct FVF_VERTEX
	{
		float px, py, pz;	// 頂点座標.
		float nx, ny, nz;	// 法線.
		float u, v;			// UV座標.

		bool operator==( const FVF_VERTEX& r ) const {
			return std::memcmp( this, &r, sizeof( FVF_VERTEX ) ) == 0;
		}
	};
	// スキンメッシュ用の頂点キー.
	//	※ウェイトが頂点単位のため、同じ座標でも元のFBX頂点が違う場合は別頂点として扱う.
	struct SKIN_VERTEX_KEY
	{
		UINT32		FbxVertex;	// 元のFBX頂点番号.
		FVF_VERTEX	Vertex;		// 頂点情報.

		bool operator==( const SKIN_VERTEX_KEY& r ) const {
			return FbxVertex == r.FbxVertex && Vertex == r.Vertex;
		}
	};

	// バイト列のFNV-1aハッシュ.
	size_t HashBytes( const void* pData, size_t Size )
	{
		const unsigned char* p = static_cast<const unsigned char*>( pData );
		size_t Hash = 0x811C9DC5u;
		for ( size_t i = 0; i < Size; ++i ) {
			Hash ^= p[i];
			Hash *= 0x01000193u;
		}
		return Hash;
	}
	struct FVF_VERTEX_HASH {
		size_t operator()( const FVF_VERTEX& v ) const { return HashBytes( &v, sizeof( v ) ); }
	};
	struct SKIN_VERTEX_KEY_HASH {
		size_t operator()( const SKIN_VERTEX_KEY& v ) const { return HashBytes( &v, sizeof( v ) ); }
	};

	//---------------------------.
	// ワイド文字列をUTF-8文字列に変換する(ufbxのパス用).
	//---------------------------.
	std::string ToUTF8( const std::wstring& wStr )
	{
		if ( wStr.empty() ) return std::string();
		const int Size = ::WideCharToMultiByte( CP_UTF8, 0, wStr.c_str(), -1, nullptr, 0, nullptr, nullptr );
		std::string Out( Size, '\0' );
		::WideCharToMultiByte( CP_UTF8, 0, wStr.c_str(), -1, Out.data(), Size, nullptr, nullptr );
		Out.resize( std::strlen( Out.c_str() ) );
		return Out;
	}

	//---------------------------.
	// ufbxの行列(列ベクトル形式)をD3DXの行列(行ベクトル形式)に変換する.
	//---------------------------.
	D3DXMATRIX ToD3DXMatrix( const ufbx_matrix& m )
	{
		return D3DXMATRIX(
			static_cast<float>( m.m00 ), static_cast<float>( m.m10 ), static_cast<float>( m.m20 ), 0.0f,
			static_cast<float>( m.m01 ), static_cast<float>( m.m11 ), static_cast<float>( m.m21 ), 0.0f,
			static_cast<float>( m.m02 ), static_cast<float>( m.m12 ), static_cast<float>( m.m22 ), 0.0f,
			static_cast<float>( m.m03 ), static_cast<float>( m.m13 ), static_cast<float>( m.m23 ), 1.0f );
	}

	//---------------------------.
	// ufbxシーンの読み込み.
	//	※書き出しツールごとの差異(軸/単位/ジオメトリ変換/スケール継承)はここで吸収する.
	//---------------------------.
	ufbx_scene* LoadScene( LPCTSTR lpFileName, std::string* pOutError )
	{
		ufbx_load_opts Opts = {};
		Opts.target_axes					= ufbx_axes_left_handed_y_up;						// DirectXの左手系Y-upに変換.
		Opts.target_unit_meters				= 1.0f;												// 単位をメートルに統一.
		Opts.space_conversion				= UFBX_SPACE_CONVERSION_ADJUST_TRANSFORMS;			// ノードの姿勢側で座標変換する.
		Opts.handedness_conversion_axis		= UFBX_MIRROR_AXIS_Z;								// 右手系から左手系はZ軸反転で行う.
		Opts.geometry_transform_handling	= UFBX_GEOMETRY_TRANSFORM_HANDLING_HELPER_NODES;	// Mayaのジオメトリ変換対応.
		Opts.inherit_mode_handling			= UFBX_INHERIT_MODE_HANDLING_COMPENSATE;			// 3ds Maxのスケール継承対応.
		Opts.generate_missing_normals		= true;												// 法線が無い場合は生成する.

		ufbx_error	Error;
		ufbx_scene*	pScene = nullptr;
		const std::wstring wFileName = lpFileName;
#ifdef _DEBUG
		pScene = ufbx_load_file( ToUTF8( wFileName ).c_str(), &Opts, &Error );
#else
		// 暗号化されている場合はメモリ上で復号して読み込む.
		const std::string Path		= StringConversion::to_String( wFileName );
		const std::string EncPath	= encrypt::GetEncryptionFilePath( Path );
		auto rf = encrypt::GetRestoreFile( StringConversion::to_wString( EncPath ) );
		if ( rf.first == nullptr ) {
			if ( pOutError != nullptr ) *pOutError = "ファイルが見つかりません";
			return nullptr;
		}
		pScene = ufbx_load_memory( rf.first, rf.second, &Opts, &Error );
		delete[] rf.first;
#endif
		if ( pScene == nullptr && pOutError != nullptr ) {
			*pOutError = Error.description.data;
		}
		return pScene;
	}

	//---------------------------.
	// ノード名を取得する(空の場合は一意な名前を生成する).
	//---------------------------.
	std::string GetNodeName( const ufbx_node* pNode )
	{
		if ( pNode->name.length > 0 ) return std::string( pNode->name.data );
		if ( pNode->is_root ) return "RootFrame";
		return "Node_" + std::to_string( pNode->typed_id );
	}

	//---------------------------.
	// マテリアル情報(色/テクスチャ名)を取り出す.
	//---------------------------.
	void FillMaterial( const ufbx_material* pMaterial, D3DMATERIAL9* pOut, std::string* pOutTexName )
	{
		// 既定値(マテリアル無しの場合).
		ZeroMemory( pOut, sizeof( D3DMATERIAL9 ) );
		pOut->Diffuse.r = pOut->Diffuse.g = pOut->Diffuse.b = 1.0f;
		pOut->Diffuse.a = 1.0f;
		pOut->Ambient = pOut->Diffuse;
		pOutTexName->clear();
		if ( pMaterial == nullptr ) return;

		// ディフューズ(FBX標準が無い場合はPBRのベースカラーを使用する).
		const ufbx_material_map* pDiffuse = &pMaterial->fbx.diffuse_color;
		if ( pDiffuse->has_value == false ) pDiffuse = &pMaterial->pbr.base_color;
		if ( pDiffuse->has_value ) {
			pOut->Diffuse.r = static_cast<float>( pDiffuse->value_vec3.x );
			pOut->Diffuse.g = static_cast<float>( pDiffuse->value_vec3.y );
			pOut->Diffuse.b = static_cast<float>( pDiffuse->value_vec3.z );
		}
		// アンビエント.
		if ( pMaterial->fbx.ambient_color.has_value ) {
			pOut->Ambient.r = static_cast<float>( pMaterial->fbx.ambient_color.value_vec3.x );
			pOut->Ambient.g = static_cast<float>( pMaterial->fbx.ambient_color.value_vec3.y );
			pOut->Ambient.b = static_cast<float>( pMaterial->fbx.ambient_color.value_vec3.z );
			pOut->Ambient.a = 1.0f;
		}
		// スペキュラ.
		if ( pMaterial->fbx.specular_color.has_value ) {
			pOut->Specular.r = static_cast<float>( pMaterial->fbx.specular_color.value_vec3.x );
			pOut->Specular.g = static_cast<float>( pMaterial->fbx.specular_color.value_vec3.y );
			pOut->Specular.b = static_cast<float>( pMaterial->fbx.specular_color.value_vec3.z );
			pOut->Specular.a = 1.0f;
		}
		if ( pMaterial->fbx.specular_exponent.has_value ) {
			pOut->Power = static_cast<float>( pMaterial->fbx.specular_exponent.value_real );
		}

		// テクスチャ名(ディフューズテクスチャのみ).
		//	※既存の.x読み込みと同様に、メッシュと同じフォルダから読むためファイル名だけにする.
		const ufbx_texture* pTexture = pMaterial->fbx.diffuse_color.texture;
		if ( pTexture == nullptr ) pTexture = pMaterial->pbr.base_color.texture;
		if ( pTexture != nullptr ) {
			std::string TexPath;
			if ( pTexture->relative_filename.length > 0 )	TexPath = pTexture->relative_filename.data;
			else											TexPath = pTexture->filename.data;
			// パス区切りを統一してファイル名部分だけ取り出す.
			std::replace( TexPath.begin(), TexPath.end(), '/', '\\' );
			const size_t Pos = TexPath.find_last_of( '\\' );
			*pOutTexName = Pos == std::string::npos ? TexPath : TexPath.substr( Pos + 1 );
		}
	}

	// 静的メッシュ用の中間データ.
	struct SStaticMeshData
	{
		std::vector<FVF_VERTEX>				Vertices;		// 頂点リスト.
		std::vector<UINT32>					Indices;		// 頂点インデックス(3個で1面).
		std::vector<DWORD>					FaceMaterials;	// 面ごとのマテリアル番号.
		std::vector<const ufbx_material*>	Materials;		// マテリアルリスト.
	};

	//---------------------------.
	// シーン内の全メッシュを1つの静的メッシュデータにまとめる.
	//	※D3DXLoadMeshFromXと同様に、ノードの姿勢を頂点に焼き込んで統合する.
	//---------------------------.
	void BuildStaticMeshData( const ufbx_scene* pScene, SStaticMeshData* pOut )
	{
		std::unordered_map<FVF_VERTEX, UINT32, FVF_VERTEX_HASH>	VertexMap;		// 重複頂点の除去用.
		std::unordered_map<const ufbx_material*, DWORD>			MaterialMap;	// マテリアルの重複除去用.

		for ( size_t n = 0; n < pScene->nodes.count; ++n ) {
			const ufbx_node* pNode = pScene->nodes.data[n];
			const ufbx_mesh* pMesh = pNode->mesh;
			if ( pNode->is_root || pMesh == nullptr ) continue;

			// ノードの姿勢を頂点に焼き込むための行列.
			const ufbx_matrix GeomToWorld	= pNode->geometry_to_world;
			const ufbx_matrix NormalMatrix	= ufbx_matrix_for_normals( &GeomToWorld );

			// 面の三角形分割用バッファ.
			std::vector<uint32_t> TriIndices( pMesh->max_face_triangles * 3 );

			for ( size_t f = 0; f < pMesh->faces.count; ++f ) {
				const ufbx_face Face = pMesh->faces.data[f];

				// マテリアルを取得して、グローバルのマテリアル番号に変換する.
				const ufbx_material* pMaterial = nullptr;
				if ( f < pMesh->face_material.count && pMesh->materials.count > 0 ) {
					const uint32_t No = pMesh->face_material.data[f];
					if ( No < pMesh->materials.count ) pMaterial = pMesh->materials.data[No];
				}
				DWORD MaterialNo = 0;
				const auto mItr = MaterialMap.find( pMaterial );
				if ( mItr != MaterialMap.end() ) {
					MaterialNo = mItr->second;
				}
				else {
					MaterialNo = static_cast<DWORD>( pOut->Materials.size() );
					pOut->Materials.emplace_back( pMaterial );
					MaterialMap[pMaterial] = MaterialNo;
				}

				// 面を三角形に分割する.
				const uint32_t NumTris = ufbx_triangulate_face( TriIndices.data(), TriIndices.size(), pMesh, Face );
				for ( uint32_t i = 0; i < NumTris * 3; ++i ) {
					const uint32_t Index = TriIndices[i];

					// 頂点情報を作成する(ワールド空間に変換して焼き込む).
					const ufbx_vec3 Pos = ufbx_transform_position( &GeomToWorld, ufbx_get_vertex_vec3( &pMesh->vertex_position, Index ) );
					ufbx_vec3 Normal = {};
					if ( pMesh->vertex_normal.exists ) {
						Normal = ufbx_transform_direction( &NormalMatrix, ufbx_get_vertex_vec3( &pMesh->vertex_normal, Index ) );
						const double Len = std::sqrt( Normal.x * Normal.x + Normal.y * Normal.y + Normal.z * Normal.z );
						if ( Len > 0.0 ) { Normal.x /= Len; Normal.y /= Len; Normal.z /= Len; }
					}
					ufbx_vec2 UV = {};
					if ( pMesh->vertex_uv.exists ) UV = ufbx_get_vertex_vec2( &pMesh->vertex_uv, Index );

					FVF_VERTEX Vertex;
					Vertex.px = static_cast<float>( Pos.x );
					Vertex.py = static_cast<float>( Pos.y );
					Vertex.pz = static_cast<float>( Pos.z );
					Vertex.nx = static_cast<float>( Normal.x );
					Vertex.ny = static_cast<float>( Normal.y );
					Vertex.nz = static_cast<float>( Normal.z );
					Vertex.u  = static_cast<float>( UV.x );
					Vertex.v  = 1.0f - static_cast<float>( UV.y );	// FBXは左下原点のため上下を反転する.

					// 重複頂点はまとめる.
					UINT32 VertexNo = 0;
					const auto vItr = VertexMap.find( Vertex );
					if ( vItr != VertexMap.end() ) {
						VertexNo = vItr->second;
					}
					else {
						VertexNo = static_cast<UINT32>( pOut->Vertices.size() );
						pOut->Vertices.emplace_back( Vertex );
						VertexMap[Vertex] = VertexNo;
					}
					pOut->Indices.emplace_back( VertexNo );
				}
				// 面ごとのマテリアル番号を追加する.
				for ( uint32_t t = 0; t < NumTris; ++t ) {
					pOut->FaceMaterials.emplace_back( MaterialNo );
				}
			}
		}
	}

	//---------------------------.
	// 中間データからD3DXメッシュを作成する.
	//---------------------------.
	HRESULT CreateD3DXMesh(
		const std::vector<FVF_VERTEX>&	Vertices,
		const std::vector<UINT32>&		Indices,
		const std::vector<DWORD>&		FaceMaterials,
		DWORD							Options,
		LPDIRECT3DDEVICE9				pDevice9,
		LPD3DXMESH*						ppOutMesh )
	{
		*ppOutMesh = nullptr;
		const DWORD NumFaces	= static_cast<DWORD>( Indices.size() / 3 );
		const DWORD NumVertices	= static_cast<DWORD>( Vertices.size() );
		if ( NumFaces == 0 || NumVertices == 0 ) {
			ErrorMessage( "FBXファイル内にメッシュがありません" );
			return E_FAIL;
		}

		// 16bitインデックスで頂点数が上限を超える場合は32bitに切り替える.
		if ( ( Options & D3DXMESH_32BIT ) == 0 && NumVertices > MAX_VERTEX_16BIT ) {
			Options |= D3DXMESH_32BIT;
			Log::PushLogInfo( "FBX読み込み : 頂点数が多いため32bitインデックスに切り替えました" );
		}

		// メッシュの作成.
		LPD3DXMESH pMesh = nullptr;
		if ( FAILED( D3DXCreateMeshFVF( NumFaces, NumVertices, Options, MESH_FVF, pDevice9, &pMesh ) ) ) {
			ErrorMessage( "FBX読み込み : メッシュ作成失敗" );
			return E_FAIL;
		}

		// 頂点バッファの書き込み.
		void* pVertexData = nullptr;
		if ( FAILED( pMesh->LockVertexBuffer( 0, &pVertexData ) ) ) {
			pMesh->Release();
			return E_FAIL;
		}
		std::memcpy( pVertexData, Vertices.data(), sizeof( FVF_VERTEX ) * NumVertices );
		pMesh->UnlockVertexBuffer();

		// インデックスバッファの書き込み.
		void* pIndexData = nullptr;
		if ( FAILED( pMesh->LockIndexBuffer( 0, &pIndexData ) ) ) {
			pMesh->Release();
			return E_FAIL;
		}
		if ( ( Options & D3DXMESH_32BIT ) != 0 ) {
			std::memcpy( pIndexData, Indices.data(), sizeof( UINT32 ) * Indices.size() );
		}
		else {
			WORD* pIndex16 = static_cast<WORD*>( pIndexData );
			for ( size_t i = 0; i < Indices.size(); ++i ) {
				pIndex16[i] = static_cast<WORD>( Indices[i] );
			}
		}
		pMesh->UnlockIndexBuffer();

		// 属性バッファ(面ごとのマテリアル番号)の書き込み.
		DWORD* pAttribute = nullptr;
		if ( FAILED( pMesh->LockAttributeBuffer( 0, &pAttribute ) ) ) {
			pMesh->Release();
			return E_FAIL;
		}
		std::memcpy( pAttribute, FaceMaterials.data(), sizeof( DWORD ) * NumFaces );
		pMesh->UnlockAttributeBuffer();

		*ppOutMesh = pMesh;
		return S_OK;
	}
}

//---------------------------.
// 拡張子からメッシュの種類を判定する.
//---------------------------.
EMeshType FbxLoader::GetMeshType( LPCTSTR lpFileName )
{
	const std::wstring wFileName = lpFileName;
	std::string Extension = std::filesystem::path( wFileName ).extension().string();
#ifndef _DEBUG
	// 暗号化ファイルの場合は暗号化前の拡張子を取得する.
	Extension = encrypt::GetExtension( StringConversion::to_String( wFileName ) );
#endif
	// 小文字に統一して判定する.
	std::transform( Extension.begin(), Extension.end(), Extension.begin(),
		[]( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
	if ( Extension == ".fbx" ) return EMeshType::FBX;
	return EMeshType::X;
}

//---------------------------.
// FBXファイルから静的メッシュを作成する.
//---------------------------.
HRESULT FbxLoader::LoadStaticMesh(
	LPCTSTR				lpFileName,
	DWORD				Options,
	LPDIRECT3DDEVICE9	pDevice9,
	LPD3DXBUFFER*		ppMaterials,
	DWORD*				pNumMaterials,
	LPD3DXMESH*			ppMesh )
{
	if ( ppMesh == nullptr ) return E_FAIL;
	*ppMesh = nullptr;

	// シーンの読み込み.
	std::string ErrorText;
	ufbx_scene* pScene = LoadScene( lpFileName, &ErrorText );
	if ( pScene == nullptr ) {
		ErrorMessage( "FBXファイル読込失敗 : " + ErrorText );
		return E_FAIL;
	}

	// シーン内の全メッシュを1つにまとめる.
	SStaticMeshData MeshData;
	BuildStaticMeshData( pScene, &MeshData );

	// D3DXメッシュの作成.
	LPD3DXMESH pMesh = nullptr;
	if ( FAILED( CreateD3DXMesh( MeshData.Vertices, MeshData.Indices, MeshData.FaceMaterials, Options, pDevice9, &pMesh ) ) ) {
		ufbx_free_scene( pScene );
		return E_FAIL;
	}

	// マテリアル情報の作成.
	if ( ppMaterials != nullptr && pNumMaterials != nullptr ) {
		const DWORD NumMaterials = static_cast<DWORD>( max( 1, MeshData.Materials.size() ) );

		// 先にマテリアル情報とテクスチャ名を取り出す.
		std::vector<D3DMATERIAL9>	MaterialList( NumMaterials );
		std::vector<std::string>	TexNameList( NumMaterials );
		size_t StringBytes = 0;
		for ( DWORD i = 0; i < NumMaterials; ++i ) {
			const ufbx_material* pMaterial = i < MeshData.Materials.size() ? MeshData.Materials[i] : nullptr;
			FillMaterial( pMaterial, &MaterialList[i], &TexNameList[i] );
			if ( TexNameList[i].empty() == false ) StringBytes += TexNameList[i].size() + 1;
		}

		// マテリアル配列と文字列領域をまとめたバッファを作成する.
		//	※D3DXLoadMeshFromXの出力(D3DXMATERIAL配列)と同じ形にする.
		LPD3DXBUFFER pBuffer = nullptr;
		const DWORD BufferSize = static_cast<DWORD>( sizeof( D3DXMATERIAL ) * NumMaterials + StringBytes );
		if ( FAILED( D3DXCreateBuffer( BufferSize, &pBuffer ) ) ) {
			pMesh->Release();
			ufbx_free_scene( pScene );
			ErrorMessage( "FBX読み込み : マテリアルバッファ作成失敗" );
			return E_FAIL;
		}
		D3DXMATERIAL*	pMaterials	= static_cast<D3DXMATERIAL*>( pBuffer->GetBufferPointer() );
		char*			pString		= reinterpret_cast<char*>( pMaterials + NumMaterials );
		ZeroMemory( pMaterials, BufferSize );
		for ( DWORD i = 0; i < NumMaterials; ++i ) {
			pMaterials[i].MatD3D = MaterialList[i];
			if ( TexNameList[i].empty() ) {
				pMaterials[i].pTextureFilename = nullptr;
				continue;
			}
			// テクスチャ名はバッファ内の文字列領域を指すようにする.
			std::memcpy( pString, TexNameList[i].c_str(), TexNameList[i].size() + 1 );
			pMaterials[i].pTextureFilename = pString;
			pString += TexNameList[i].size() + 1;
		}
		*ppMaterials	= pBuffer;
		*pNumMaterials	= NumMaterials;
	}

	*ppMesh = pMesh;
	ufbx_free_scene( pScene );
	return S_OK;
}

namespace
{
	//---------------------------.
	// 親フレームの末尾に子フレームを追加する(元の並び順を維持する).
	//---------------------------.
	void AppendChildFrame( LPD3DXFRAME pParent, LPD3DXFRAME pChild )
	{
		if ( pParent->pFrameFirstChild == nullptr ) {
			pParent->pFrameFirstChild = pChild;
			return;
		}
		LPD3DXFRAME pSibling = pParent->pFrameFirstChild;
		while ( pSibling->pFrameSibling != nullptr ) {
			pSibling = pSibling->pFrameSibling;
		}
		pSibling->pFrameSibling = pChild;
	}

	//---------------------------.
	// スキンメッシュ用のメッシュコンテナを作成してフレームに設定する.
	//---------------------------.
	HRESULT CreateSkinMeshContainer(
		const ufbx_scene*				pScene,
		const ufbx_node*				pNode,
		LPD3DXFRAME						pFrame,
		const std::vector<std::string>&	NodeNames,
		LPD3DXALLOCATEHIERARCHY			pAlloc,
		LPDIRECT3DDEVICE9				pDevice9 )
	{
		const ufbx_mesh* pFbxMesh = pNode->mesh;

		// 頂点/インデックスを作成する(スキン用は頂点をジオメトリ空間のまま使用する).
		std::vector<FVF_VERTEX>	Vertices;
		std::vector<UINT32>		Indices;
		std::vector<DWORD>		FaceMaterials;
		std::vector<UINT32>		FinalToFbxVertex;	// 作成した頂点から元のFBX頂点番号への変換.
		std::unordered_map<SKIN_VERTEX_KEY, UINT32, SKIN_VERTEX_KEY_HASH> VertexMap;

		std::vector<uint32_t> TriIndices( pFbxMesh->max_face_triangles * 3 );
		for ( size_t f = 0; f < pFbxMesh->faces.count; ++f ) {
			const ufbx_face Face = pFbxMesh->faces.data[f];

			// マテリアル番号(メッシュ内のマテリアルリスト基準).
			DWORD MaterialNo = 0;
			if ( f < pFbxMesh->face_material.count && pFbxMesh->materials.count > 0 ) {
				const uint32_t No = pFbxMesh->face_material.data[f];
				if ( No < pFbxMesh->materials.count ) MaterialNo = No;
			}

			// 面を三角形に分割する.
			const uint32_t NumTris = ufbx_triangulate_face( TriIndices.data(), TriIndices.size(), pFbxMesh, Face );
			for ( uint32_t i = 0; i < NumTris * 3; ++i ) {
				const uint32_t Index = TriIndices[i];

				const ufbx_vec3 Pos = ufbx_get_vertex_vec3( &pFbxMesh->vertex_position, Index );
				ufbx_vec3 Normal = {};
				if ( pFbxMesh->vertex_normal.exists ) Normal = ufbx_get_vertex_vec3( &pFbxMesh->vertex_normal, Index );
				ufbx_vec2 UV = {};
				if ( pFbxMesh->vertex_uv.exists ) UV = ufbx_get_vertex_vec2( &pFbxMesh->vertex_uv, Index );

				SKIN_VERTEX_KEY Key;
				Key.FbxVertex	= pFbxMesh->vertex_indices.data[Index];
				Key.Vertex.px	= static_cast<float>( Pos.x );
				Key.Vertex.py	= static_cast<float>( Pos.y );
				Key.Vertex.pz	= static_cast<float>( Pos.z );
				Key.Vertex.nx	= static_cast<float>( Normal.x );
				Key.Vertex.ny	= static_cast<float>( Normal.y );
				Key.Vertex.nz	= static_cast<float>( Normal.z );
				Key.Vertex.u	= static_cast<float>( UV.x );
				Key.Vertex.v	= 1.0f - static_cast<float>( UV.y );	// FBXは左下原点のため上下を反転する.

				// 重複頂点はまとめる.
				UINT32 VertexNo = 0;
				const auto vItr = VertexMap.find( Key );
				if ( vItr != VertexMap.end() ) {
					VertexNo = vItr->second;
				}
				else {
					VertexNo = static_cast<UINT32>( Vertices.size() );
					Vertices.emplace_back( Key.Vertex );
					FinalToFbxVertex.emplace_back( Key.FbxVertex );
					VertexMap[Key] = VertexNo;
				}
				Indices.emplace_back( VertexNo );
			}
			for ( uint32_t t = 0; t < NumTris; ++t ) {
				FaceMaterials.emplace_back( MaterialNo );
			}
		}
		// メッシュが空の場合はコンテナを作成しない.
		if ( Indices.empty() ) return S_OK;

		// 既存のスキンメッシュ処理は16bitインデックス前提のため頂点数を確認する.
		if ( Vertices.size() > MAX_VERTEX_16BIT ) {
			ErrorMessage( "FBX読み込み : スキンメッシュの頂点数が上限(65534)を超えています" );
			return E_FAIL;
		}

		// D3DXメッシュの作成(既存の.x読み込みと同じMANAGED/16bitで作成する).
		LPD3DXMESH pMesh = nullptr;
		if ( FAILED( CreateD3DXMesh( Vertices, Indices, FaceMaterials, D3DXMESH_MANAGED, pDevice9, &pMesh ) ) ) {
			return E_FAIL;
		}

		// 隣接情報の作成.
		std::vector<DWORD> Adjacency( static_cast<size_t>( pMesh->GetNumFaces() ) * 3 );
		if ( FAILED( pMesh->GenerateAdjacency( 1.0e-6f, Adjacency.data() ) ) ) {
			pMesh->Release();
			ErrorMessage( "FBX読み込み : 隣接情報作成失敗" );
			return E_FAIL;
		}

		// マテリアル情報の作成(文字列はコンテナ作成時にコピーされるため一時領域でよい).
		const DWORD NumMaterials = static_cast<DWORD>( max( 1, pFbxMesh->materials.count ) );
		std::vector<D3DXMATERIAL>	Materials( NumMaterials );
		std::vector<std::string>	TexNames( NumMaterials );
		for ( DWORD i = 0; i < NumMaterials; ++i ) {
			const ufbx_material* pMaterial = i < pFbxMesh->materials.count ? pFbxMesh->materials.data[i] : nullptr;
			ZeroMemory( &Materials[i], sizeof( D3DXMATERIAL ) );
			FillMaterial( pMaterial, &Materials[i].MatD3D, &TexNames[i] );
			Materials[i].pTextureFilename = TexNames[i].empty() ? nullptr : const_cast<char*>( TexNames[i].c_str() );
		}

		// スキン情報の作成.
		LPD3DXSKININFO pSkinInfo = nullptr;
		if ( pFbxMesh->skin_deformers.count > 0 ) {
			const ufbx_skin_deformer* pSkin = pFbxMesh->skin_deformers.data[0];
			const DWORD NumBones = static_cast<DWORD>( pSkin->clusters.count );
			if ( NumBones > MAX_BONES ) {
				pMesh->Release();
				ErrorMessage( "FBX読み込み : ボーン数が上限(255)を超えています" );
				return E_FAIL;
			}
			if ( NumBones > 0 ) {
				if ( FAILED( D3DXCreateSkinInfoFVF( static_cast<DWORD>( Vertices.size() ), MESH_FVF, NumBones, &pSkinInfo ) ) ) {
					pMesh->Release();
					ErrorMessage( "FBX読み込み : スキン情報作成失敗" );
					return E_FAIL;
				}

				// 元のFBX頂点番号から作成した頂点番号への変換表を作成する.
				std::vector<std::vector<UINT32>> FbxToFinal( pFbxMesh->num_vertices );
				for ( UINT32 v = 0; v < FinalToFbxVertex.size(); ++v ) {
					FbxToFinal[FinalToFbxVertex[v]].emplace_back( v );
				}

				for ( DWORD b = 0; b < NumBones; ++b ) {
					const ufbx_skin_cluster* pCluster = pSkin->clusters.data[b];

					// ボーン名(フレーム名と一致させる).
					const char* pBoneName = "";
					if ( pCluster->bone_node != nullptr ) {
						pBoneName = NodeNames[pCluster->bone_node->typed_id].c_str();
					}
					pSkinInfo->SetBoneName( b, pBoneName );

					// バインドポーズ行列(ジオメトリ空間からボーン空間への変換).
					const D3DXMATRIX BoneOffset = ToD3DXMatrix( pCluster->geometry_to_bone );
					pSkinInfo->SetBoneOffsetMatrix( b, &BoneOffset );

					// このボーンが影響する頂点とウェイトを設定する.
					std::vector<DWORD> InfluenceVertices;
					std::vector<float> InfluenceWeights;
					for ( size_t w = 0; w < pCluster->num_weights; ++w ) {
						const uint32_t	FbxVertex	= pCluster->vertices.data[w];
						const float		Weight		= static_cast<float>( pCluster->weights.data[w] );
						if ( FbxVertex >= FbxToFinal.size() ) continue;
						for ( const UINT32 FinalVertex : FbxToFinal[FbxVertex] ) {
							InfluenceVertices.emplace_back( FinalVertex );
							InfluenceWeights.emplace_back( Weight );
						}
					}
					if ( InfluenceVertices.empty() == false ) {
						pSkinInfo->SetBoneInfluence( b,
							static_cast<DWORD>( InfluenceVertices.size() ),
							InfluenceVertices.data(), InfluenceWeights.data() );
					}
				}
			}
		}

		// メッシュコンテナの作成(MY_HIERARCHY側でメッシュの複製や文字列のコピーが行われる).
		D3DXMESHDATA MeshData;
		MeshData.Type	= D3DXMESHTYPE_MESH;
		MeshData.pMesh	= pMesh;
		LPD3DXMESHCONTAINER pContainer = nullptr;
		const HRESULT hr = pAlloc->CreateMeshContainer(
			NodeNames[pNode->typed_id].c_str(),	// コンテナ名.
			&MeshData,							// メッシュ情報.
			Materials.data(),					// マテリアル情報.
			nullptr,							// エフェクト情報(未使用).
			static_cast<DWORD>( pFbxMesh->materials.count ),	// マテリアル数.
			Adjacency.data(),					// 隣接情報.
			pSkinInfo,							// スキン情報.
			&pContainer );

		// コンテナ側で複製/参照追加されるため、こちらの参照は解放する.
		pMesh->Release();
		if ( pSkinInfo != nullptr ) pSkinInfo->Release();

		if ( FAILED( hr ) ) {
			ErrorMessage( "FBX読み込み : メッシュコンテナ作成失敗" );
			return E_FAIL;
		}
		pFrame->pMeshContainer = pContainer;
		return S_OK;
	}

	//---------------------------.
	// アニメーションコントローラを作成する.
	//---------------------------.
	HRESULT CreateAnimationController(
		const ufbx_scene*				pScene,
		const std::vector<LPD3DXFRAME>&	Frames,
		LPD3DXANIMATIONCONTROLLER*		ppOut )
	{
		*ppOut = nullptr;
		const size_t	NumNodes	= pScene->nodes.count;
		const DWORD		NumStacks	= static_cast<DWORD>( min( pScene->anim_stacks.count, MAX_ANIM_SET ) );

		// アニメーションコントローラの作成.
		LPD3DXANIMATIONCONTROLLER pAC = nullptr;
		if ( FAILED( D3DXCreateAnimationController(
			static_cast<UINT>( NumNodes ),	// 最大アニメーション出力数.
			max( 1, NumStacks ),			// 最大アニメーションセット数.
			2,								// 最大トラック数(ブレンド用に2つ).
			30,								// 最大イベント数.
			&pAC ) ) )
		{
			ErrorMessage( "FBX読み込み : アニメーションコントローラ作成失敗" );
			return E_FAIL;
		}

		// 各フレームの変換行列をアニメーションの出力先として登録する.
		for ( size_t n = 0; n < NumNodes; ++n ) {
			pAC->RegisterAnimationOutput( Frames[n]->Name, &Frames[n]->TransformationMatrix, nullptr, nullptr, nullptr );
		}

		// アニメーションセット(FBXのアニメーションスタック)を作成する.
		for ( DWORD s = 0; s < NumStacks; ++s ) {
			const ufbx_anim_stack* pStack = pScene->anim_stacks.data[s];

			// 再生時間とキー数を求める.
			const double TimeBegin	= pStack->time_begin;
			const double Duration	= max( pStack->time_end - TimeBegin, 1.0 / ANIM_SAMPLE_RATE );
			const UINT   NumKeys	= static_cast<UINT>( Duration * ANIM_SAMPLE_RATE ) + 1;

			// セット名(空の場合は番号から生成する).
			std::string SetName = pStack->name.length > 0 ? pStack->name.data : "";
			if ( SetName.empty() ) SetName = "AnimSet" + std::to_string( s );

			// キーフレームアニメーションセットの作成.
			//	※TicksPerSecond=1 とし、キー時間を秒単位で登録する(GetPeriodが秒を返すようにする).
			LPD3DXKEYFRAMEDANIMATIONSET pSet = nullptr;
			if ( FAILED( D3DXCreateKeyframedAnimationSet(
				SetName.c_str(), 1.0, D3DXPLAY_LOOP,
				static_cast<UINT>( NumNodes ), 0, nullptr, &pSet ) ) )
			{
				pAC->Release();
				ErrorMessage( "FBX読み込み : アニメーションセット作成失敗" );
				return E_FAIL;
			}

			// 各ノードの姿勢を一定間隔でサンプリングしてキーとして登録する.
			std::vector<D3DXKEY_VECTOR3>	ScaleKeys( NumKeys );
			std::vector<D3DXKEY_VECTOR3>	TransKeys( NumKeys );
			std::vector<D3DXKEY_QUATERNION>	RotateKeys( NumKeys );
			DWORD AnimIndex = 0;
			for ( size_t n = 0; n < NumNodes; ++n ) {
				const ufbx_node* pNode = pScene->nodes.data[n];
				if ( pNode->is_root ) continue;

				for ( UINT k = 0; k < NumKeys; ++k ) {
					const double Time = min( TimeBegin + k / ANIM_SAMPLE_RATE, pStack->time_end );
					const ufbx_transform Transform = ufbx_evaluate_transform( pStack->anim, pNode, Time );
					const FLOAT KeyTime = static_cast<FLOAT>( Time - TimeBegin );

					ScaleKeys[k].Time		= KeyTime;
					ScaleKeys[k].Value.x	= static_cast<float>( Transform.scale.x );
					ScaleKeys[k].Value.y	= static_cast<float>( Transform.scale.y );
					ScaleKeys[k].Value.z	= static_cast<float>( Transform.scale.z );

					TransKeys[k].Time		= KeyTime;
					TransKeys[k].Value.x	= static_cast<float>( Transform.translation.x );
					TransKeys[k].Value.y	= static_cast<float>( Transform.translation.y );
					TransKeys[k].Value.z	= static_cast<float>( Transform.translation.z );

					// 回転キーは共役クォータニオンで登録する(D3DXのアニメーションセットの仕様).
					RotateKeys[k].Time		= KeyTime;
					RotateKeys[k].Value.x	= -static_cast<float>( Transform.rotation.x );
					RotateKeys[k].Value.y	= -static_cast<float>( Transform.rotation.y );
					RotateKeys[k].Value.z	= -static_cast<float>( Transform.rotation.z );
					RotateKeys[k].Value.w	=  static_cast<float>( Transform.rotation.w );
				}
				pSet->RegisterAnimationSRTKeys(
					Frames[n]->Name, NumKeys, NumKeys, NumKeys,
					ScaleKeys.data(), RotateKeys.data(), TransKeys.data(), &AnimIndex );
			}

			// コントローラにセットを登録する(コントローラ側で参照が保持される).
			pAC->RegisterAnimationSet( pSet );
			pSet->Release();
		}

		// 既定状態として最初のセットをトラック0に設定する.
		if ( NumStacks > 0 ) {
			LPD3DXANIMATIONSET pFirstSet = nullptr;
			if ( SUCCEEDED( pAC->GetAnimationSet( 0, &pFirstSet ) ) ) {
				pAC->SetTrackAnimationSet( 0, pFirstSet );
				pAC->SetTrackEnable( 0, TRUE );
				pAC->SetTrackWeight( 0, 1.0f );
				pAC->SetTrackSpeed( 0, 1.0f );
				pFirstSet->Release();
			}
		}

		*ppOut = pAC;
		return S_OK;
	}
}

//---------------------------.
// FBXファイルからスキンメッシュを作成する.
//---------------------------.
HRESULT FbxLoader::LoadSkinnedMesh(
	LPCTSTR						lpFileName,
	LPDIRECT3DDEVICE9			pDevice9,
	LPD3DXALLOCATEHIERARCHY		pAlloc,
	LPD3DXFRAME*				ppFrameRoot,
	LPD3DXANIMATIONCONTROLLER*	ppAnimController )
{
	if ( pAlloc == nullptr || ppFrameRoot == nullptr || ppAnimController == nullptr ) return E_FAIL;
	*ppFrameRoot		= nullptr;
	*ppAnimController	= nullptr;

	// シーンの読み込み.
	std::string ErrorText;
	ufbx_scene* pScene = LoadScene( lpFileName, &ErrorText );
	if ( pScene == nullptr ) {
		ErrorMessage( "FBXファイル読込失敗 : " + ErrorText );
		return E_FAIL;
	}

	const size_t NumNodes = pScene->nodes.count;
	std::vector<LPD3DXFRAME>	Frames( NumNodes, nullptr );
	std::vector<std::string>	NodeNames( NumNodes );
	LPD3DXFRAME					pFrameRoot = nullptr;

	// 失敗時の解放処理.
	auto Cleanup = [&]()
	{
		if ( pFrameRoot != nullptr ) pAlloc->DestroyFrame( pFrameRoot );
		ufbx_free_scene( pScene );
	};

	// フレーム階層の構築(ufbxのノードリストは親が先に並ぶことが保証されている).
	for ( size_t n = 0; n < NumNodes; ++n ) {
		const ufbx_node* pNode = pScene->nodes.data[n];
		NodeNames[n] = GetNodeName( pNode );

		if ( FAILED( pAlloc->CreateFrame( NodeNames[n].c_str(), &Frames[n] ) ) ) {
			Cleanup();
			ErrorMessage( "FBX読み込み : フレーム作成失敗" );
			return E_FAIL;
		}
		// 親からの相対姿勢を設定する.
		Frames[n]->TransformationMatrix = ToD3DXMatrix( pNode->node_to_parent );

		// 親フレームに接続する.
		if ( pNode->is_root || pNode->parent == nullptr ) {
			pFrameRoot = Frames[n];
		}
		else {
			AppendChildFrame( Frames[pNode->parent->typed_id], Frames[n] );
		}
	}
	if ( pFrameRoot == nullptr ) {
		Cleanup();
		ErrorMessage( "FBX読み込み : ルートフレームがありません" );
		return E_FAIL;
	}

	// メッシュコンテナの構築.
	for ( size_t n = 0; n < NumNodes; ++n ) {
		const ufbx_node* pNode = pScene->nodes.data[n];
		if ( pNode->is_root || pNode->mesh == nullptr ) continue;

		if ( FAILED( CreateSkinMeshContainer( pScene, pNode, Frames[n], NodeNames, pAlloc, pDevice9 ) ) ) {
			Cleanup();
			return E_FAIL;
		}
	}

	// アニメーションコントローラの構築.
	LPD3DXANIMATIONCONTROLLER pAC = nullptr;
	if ( FAILED( CreateAnimationController( pScene, Frames, &pAC ) ) ) {
		Cleanup();
		return E_FAIL;
	}

	*ppFrameRoot		= pFrameRoot;
	*ppAnimController	= pAC;
	ufbx_free_scene( pScene );
	return S_OK;
}
#endif	// ENABLE_MESH
