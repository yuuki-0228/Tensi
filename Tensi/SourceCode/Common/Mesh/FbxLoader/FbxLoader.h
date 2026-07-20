#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_MESH

//警告についてのコード分析を無効にする。4005：再定義.
#pragma warning( disable : 4005 )

#include <d3dx9.h>
#include <d3d11.h>

// メッシュファイルの種類.
enum class EMeshType
{
	Auto,	// 拡張子から自動判定.
	X,		// .x ファイル.
	FBX,	// .fbx ファイル.
};

/************************************************
*	FBXメッシュ読み込み(D3DXオブジェクト構築).
*		各ツール(Blender/Maya/3ds Max等)の書き出し差異
*		(ASCII/バイナリ, 軸, 単位, ジオメトリ変換)は ufbx 側で吸収する.
**/
namespace FbxLoader
{
	// 拡張子からメッシュの種類を判定する.
	//	※暗号化ファイルの場合は暗号化前の拡張子で判定する.
	EMeshType GetMeshType( LPCTSTR lpFileName );

	// FBXファイルから静的メッシュを作成する.
	//	D3DXLoadMeshFromX と同等の出力(メッシュ/マテリアル情報)を構築する.
	//	※16bitインデックス指定時に頂点数が上限を超える場合は自動で32bitに切り替える.
	HRESULT LoadStaticMesh(
		LPCTSTR				lpFileName,		// FBXファイルパス.
		DWORD				Options,		// メッシュ作成オプション(D3DXMESH_～).
		LPDIRECT3DDEVICE9	pDevice9,		// Dx9デバイス.
		LPD3DXBUFFER*		ppMaterials,	// (out)マテリアル情報.
		DWORD*				pNumMaterials,	// (out)マテリアル数.
		LPD3DXMESH*			ppMesh );		// (out)メッシュオブジェクト.

	// FBXファイルからスキンメッシュを作成する.
	//	D3DXLoadMeshHierarchyFromX と同等の出力
	//	(フレーム階層/スキン情報/アニメーションコントローラ)を構築する.
	HRESULT LoadSkinnedMesh(
		LPCTSTR						lpFileName,			// FBXファイルパス.
		LPDIRECT3DDEVICE9			pDevice9,			// Dx9デバイス.
		LPD3DXALLOCATEHIERARCHY		pAlloc,				// フレーム/メッシュコンテナのアロケータ.
		LPD3DXFRAME*				ppFrameRoot,		// (out)フレーム階層のルート.
		LPD3DXANIMATIONCONTROLLER*	ppAnimController );	// (out)アニメーションコントローラ.
}
#endif	// ENABLE_MESH
