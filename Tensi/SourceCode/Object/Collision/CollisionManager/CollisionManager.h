#pragma once
#include "..\..\..\Global.h"

class CSphere;
class CCylinder;
class CRay;
class CCrossRay;
class CMesh;

class CBox2D;
class CSphere2D;

/************************************************
*	�����蔻��ꗗ.
**/
namespace Coll {
	// ���̓��m�̓����蔻��.
	bool IsSphereToSphere(
		CSphere* pMySphere,		// �����̋���.
		CSphere* pOpSphere );	// ����̋���.

	// ���̂Ɠ_�̓����蔻��.
	bool IsSphereToPosition(
		CSphere*				pMySphere,		// �����̋���.
		const D3DXPOSITION3&	pOpPos );		// ����̍��W.

	// �~���Ɠ_�̓����蔻��.
	bool IsCylinderToPosition(
		CCylinder*				pMyCylinder,	// �����̉~��.
		const D3DXPOSITION3&	pOpPos);		// ����̍��W.

	// ���C�ƃ��b�V���̓����蔻��.
	bool IsRayToMesh(
		CRay*				pRay,						// ���C.
		CMesh*				pMesh,						// ���b�V��.
		float*				pDistance		= nullptr,	// (out)����.
		D3DXPOSITION3*		pIntersect		= nullptr,	// (out)�����_.
		D3DXVECTOR3*		pNormal			= nullptr,	// (out)�@��.
		bool				IsWallScratch	= false,	// �ǂ��肷�邩.
		float				WallSpace		= 0.8f );	// �ǂƂ̌��E����.

	// �\�����C�ƕ�(���b�V��)�̓����蔻��.
	bool IsCrossRayToWallMesh( 
		CCrossRay*			pCrossRay,					// �\�����C.
		CMesh*				pWallMesh,					// ��(���b�V��).
		std::vector<int>*	pHitList = nullptr );		// ���������������X�g.
}

/************************************************
*	2D�̓����蔻��ꗗ.
**/
namespace Coll2D {
	// 2D�̃{�b�N�X�Ɠ_�̓����蔻��.
	bool IsBox2DToPoint2D(
		CBox2D*					pMyBox,			// �����̃{�b�N�X.
		const D3DXPOSITION2&	pOpPos );		// ����̍��W.

	// 2D�̃{�b�N�X���m�̓����蔻��.
	bool IsBox2DToBox2D(
		CBox2D*					pMyBox,			// �����̃{�b�N�X.
		CBox2D*					pOpBox );		// ����̃{�b�N�X.

	// 2D�̉~�Ɠ_�̓����蔻��.
	bool IsSphere2DToPoint2D(
		CSphere2D*				pMySphere,		// �����̉~.
		const D3DXPOSITION2&	pOpPos );		// ����̍��W.

	// 2D�̉~���m�̓����蔻��.
	bool IsSphere2DToSphere2D(
		CSphere2D*				pMySphere,		// �����̉~.
		CSphere2D*				pOpSphere );	// ����̉~.

	// 2D�̃{�b�N�X�Ɖ~�̓����蔻��.
	bool IsBox2DToSphere2D(
		CBox2D*					pMyBox,			// �����̃{�b�N�X.
		CSphere2D*				pOpSphere );	// ����̉~.
}