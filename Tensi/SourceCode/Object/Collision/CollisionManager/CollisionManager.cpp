#include "CollisionManager.h"
#include "..\..\Collision\Sphere\Sphere.h"
#include "..\..\Collision\Cylinder\Cylinder.h"
#include "..\..\Collision\Ray\Ray.h"
#include "..\..\Collision\CrossRay\CrossRay.h"
#include "..\..\Collision\Mesh\Mesh.h"

#include "..\..\Collision\2D\Box2D\Box2D.h"
#include "..\..\Collision\2D\Sphere2D\Sphere2D.h"

namespace {
	// 2D�̃{�b�N�X�Ɖ~�̓����蔻��Ŏg�p.
	float DistanceSqrf( const float t_x1, const float t_y1, const float t_x2, const float t_y2 ) {
		float dx = t_x2 - t_x1;
		float dy = t_y2 - t_y1;

		return ( dx * dx ) + ( dy * dy );
	}
}

//---------------------------.
// ���̓��m�̓����蔻��.
//	���̂� null / ���a�� 0 �ȉ��̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll::IsSphereToSphere(
	CSphere* pMySphere,		// �����̋���.
	CSphere* pOpSphere )	// ����̋���.
{
	if ( pMySphere == nullptr			) return false;
	if ( pOpSphere == nullptr			) return false;
	if ( pMySphere->GetRadius() <= 0.0f ) return false;
	if ( pOpSphere->GetRadius() <= 0.0f ) return false;

	// ������.
	pMySphere->SetFlagOff();
	pOpSphere->SetFlagOff();

	// 2�̋��̂̒��S�Ԃ̋��������߂�.
	const D3DXVECTOR3& vLength = pMySphere->GetPosition() - pOpSphere->GetPosition();
	// ��L�̃x�N�g�����璷���ɕϊ�.
	const float& fLength = D3DXVec3Length( &vLength );

	// 2�̋��̂̔��a�̍��v�����߂�.
	const float& AddRadius = pMySphere->GetRadius() + pOpSphere->GetRadius();

	// 2�̋��̊Ԃ̋�����2�̋��̂̂��ꂼ��̔��a�𑫂������̂��A
	//	�������Ƃ������Ƃ́A���̓��m���d�Ȃ��Ă���(�Փ˂��Ă���)�Ƃ�������.
	if ( fLength <= AddRadius )
	{
		// �t���O�𗧂Ă�.
		pMySphere->SetFlagOn();
		pOpSphere->SetFlagOn();

		return true;	// �Փ�.
	}
	return false;		// �Փ˂��ĂȂ�.
}

//---------------------------.
// ���̂Ɠ_�̓����蔻��.
//	����/���W�� null / ���a�� 0 �ȉ��̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll::IsSphereToPosition(
	CSphere*				pMySphere,	// �����̋���.
	const D3DXPOSITION3&	pOpPos )	// ����̍��W.
{
	if ( pMySphere	== nullptr			) return false;
	if ( pOpPos		== nullptr			) return false;
	if ( pMySphere->GetRadius() <= 0.0f ) return false;

	// ������.
	pMySphere->SetFlagOff();

	// ���������߂�.
	const D3DXVECTOR3& vLength = pMySphere->GetPosition() - pOpPos;
	// ��L�̃x�N�g�����璷���ɕϊ�.
	const float& fLength = D3DXVec3Length( &vLength );

	// ���������a���A�������Ƃ������Ƃ́A
	//	���̂Ɠ_���d�Ȃ��Ă���(�Փ˂��Ă���)�Ƃ�������.
	if ( fLength <= pMySphere->GetRadius() )
	{
		// �t���O�𗧂Ă�.
		pMySphere->SetFlagOn();

		return true;	// �Փ�.
	}
	return false;		// �Փ˂��ĂȂ�.
}

//---------------------------.
// �~���Ɠ_�̓����蔻��.
//	�~��/���W�� null / ���a/������ 0 �ȉ��̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll::IsCylinderToPosition(
	CCylinder*				pMyCylinder,	// �����̉~��.
	const D3DXPOSITION3&	pOpPos )		// ����̍��W.
{
	if ( pMyCylinder	== nullptr			) return false;
	if ( pOpPos			== nullptr			) return false;
	if ( pMyCylinder->GetRadius() <= 0.0f	) return false;
	if ( pMyCylinder->GetHeight() <= 0.0f	) return false;

	// ������.
	pMyCylinder->SetFlagOff();

	// ���������߂�.
	const D3DXPOSITION3& CheckPos = { pOpPos.x, pMyCylinder->GetPosition().y, pOpPos.z };
	const D3DXVECTOR3&	 vLength  = pMyCylinder->GetPosition() - CheckPos;
	// ��L�̃x�N�g�����璷���ɕϊ�.
	const float& fLength = D3DXVec3Length( &vLength );

	// ���������a��菬�����A�����͈͓̔��ɍ��W�����邩���ׂ�.
	const float& YMax	= pMyCylinder->GetPosition().y + pMyCylinder->GetHeight();
	const float& YMin	= pMyCylinder->GetPosition().y - pMyCylinder->GetHeight();
	if ( fLength <= pMyCylinder->GetRadius() && (
		 YMax >= pOpPos.y && pOpPos.y >= YMin ) )
	{
		// �t���O�𗧂Ă�.
		pMyCylinder->SetFlagOn();

		return true;	// �Փ�.
	}
	return false;		// �Փ˂��ĂȂ�.
}

//---------------------------.
// ���C�ƃ��b�V���̓����蔻��.
//	���C�� null / ���b�V���� null �̏ꍇ�͏����͍s��Ȃ�.
//---------------------------.
bool Coll::IsRayToMesh(
	CRay*			pRay,			// ���C.
	CMesh*			pMesh,			// ���b�V��.
	float*			pDistance,		// (out)����.
	D3DXPOSITION3*	pIntersect,		// (out)�����_.
	D3DXVECTOR3*	pNormal,		// (out)�@��.
	bool			IsWallScratch,	// �ǂ��肷�邩.
	float			WallSpace )		// �ǂƂ̌��E����.
{
	if ( pRay	== nullptr ) return false;
	if ( pMesh	== nullptr ) return false;

	// ������.
	pRay->SetFlagOff();
	pMesh->SetFlagOff();

	// ���C�̏����擾.
	const D3DXROTATION3& vRayRot = pRay->GetRotation();
	const D3DXVECTOR3&	 vRayVec = pRay->GetVector();
	const float			 Length	 = pRay->GetLength();

	D3DXVECTOR3 vAxis;		// ���x�N�g��.
	D3DXMATRIX	mRotationY;	// Y����]�s��.
	D3DXMatrixRotationY( &mRotationY, vRayRot.y );// Y����]�s����쐬.
	// ���x�N�g�������݂̉�]��Ԃɕϊ�����.
	D3DXVec3TransformCoord( &vAxis, &vRayVec, &mRotationY );

	// ���C�̎n�_�ƏI�_.
	D3DXPOSITION3			vStart, vEnd;
	const D3DXPOSITION3&	RayPos = pRay->GetPosition();
	vStart	= RayPos;						// ���C�̎n�_��ݒ�.
	vEnd	= RayPos + ( vAxis * Length );	// ���C�̏I�_��ݒ�.

	// ���C�𓖂Ă������b�V�����ړ����Ă���ꍇ�ł�,
	//	�Ώۂ�world�s��̋t�s���p����ΐ��������C��������.
	D3DXMATRIX mWorld, mInvWorld;
	// �t�s������߂�.
	pMesh->SetRotation( INIT_FLOAT3 );
	mWorld = pMesh->GetTransform().GetWorldMatrix();
	D3DXMatrixInverse( &mInvWorld, nullptr, &mWorld );
	// ���C�̎n�_,�I�_�ɔ��f.
	D3DXVec3TransformCoord( &vStart, &vStart, &mInvWorld );
	D3DXVec3TransformCoord( &vEnd, &vEnd, &mInvWorld );

	// ���������߂�.
	D3DXVECTOR3 vDirection = vEnd - vStart;

	BOOL bHit = FALSE;			// �����t���O.
	DWORD dwIndex = 0;			// �C���f�b�N�X�ԍ�.
	D3DXVECTOR3 Vertex[3];		// ���_���W.
	FLOAT U = 0, V = 0;			// �d�S�q�b�g���W.

	// ���b�V���ƃ��C�̌����𒲂ׂ�.
	float Distance = 0.0f;
	D3DXIntersect(
		pMesh->GetMesh(),		// �Ώۃ��b�V��.
		&vStart,				// ���C�n�_.
		&vDirection,			// ���C�̕���.
		&bHit,					// (out)���茋��.
		&dwIndex,				// (out)bHit��True���Ƀ��C�̎n�_�ɍł��߂��̖ʂ̃C���f�b�N�X�l�ւ̃|�C���^.
		&U, &V,					// (out)�d�S�q�b�g���W.
		&Distance,				// ���b�V���Ƃ̋���.
		nullptr, nullptr );
	if ( pDistance != nullptr ) *pDistance = Distance;

	// �����ɐL�т郌�C�̂ǂ����Ń��b�V���ɓ������Ă�����.
	if ( bHit == TRUE ) {
		// �t���O�𗧂Ă�.
		pRay->SetFlagOn();
		pMesh->SetFlagOn();

		// ���������Ƃ�.
		pMesh->FindVerticesOnPoly( dwIndex, Vertex );
		// �d�S���W�����_���Z�o.
		//	���[�J����_�� v0 + U*(v1-v0) + V*(v2-v0) �ŋ��܂�.
		D3DXPOSITION3 Intersect = { 0.0f, 0.0f, 0.0f };
		Intersect =
			Vertex[0] + U * ( Vertex[1] - Vertex[0] ) + V * ( Vertex[2] - Vertex[0] );
		if ( pIntersect != nullptr ) *pIntersect = Intersect;

		// �@���̏o�͗v���������.
		D3DXVECTOR3 Normal;
		if ( pNormal != nullptr || IsWallScratch ) {
			D3DXPLANE	p;
			// ���̒��_(3�_)����u���ʂ̕������v�𓾂�.
			D3DXPlaneFromPoints( &p, &Vertex[0], &Vertex[1], &Vertex[2] );
			// �u���ʂ̕������v�̌W�����@���̐���.
			Normal = D3DXVECTOR3( p.a, p.b, p.c );
			if ( pNormal != nullptr ) *pNormal = Normal;
		}

		// vEnd����݂�������1.0f��菬������Γ������Ă���.
		if ( Distance < 1.0f ) {
			// �ǂ��菈��.
			if ( IsWallScratch ) {
				if ( Distance <= WallSpace ) {
					// �@���͕ǂ���^�������ɏo�Ă���̂ŁA�@���Ƃ������킹��.
					//	�Ώۂ𓮂����x�N�g����������.
					D3DXVECTOR3 vOffset = Normal * ( WallSpace - Distance );
					vOffset.y = 0.0f;

					// ���C�̈ʒu���X�V.
					pRay->SetPosition( pRay->GetPosition() + vOffset );
				}
			}
			return true;	// �������Ă���.
		}
	}
	return false;	// �O��Ă���.
}

//---------------------------.
// �\�����C�ƕ�(���b�V��)�̓����蔻��.
//	�\�����C�� null / ��(���b�V��)�� null �̏ꍇ�͏����͍s��Ȃ�.
//---------------------------.
bool Coll::IsCrossRayToWallMesh(
	CCrossRay*			pCrossRay,	// �\�����C.
	CMesh*				pWallMesh,	// ��(���b�V��).
	std::vector<int>*	pHitList )	// ���������������X�g.
{
	if ( pCrossRay == nullptr	) return false;
	if ( pWallMesh == nullptr	) return false;

	// �ǂƂ̌��E����.
	const float WSPACE = 0.8f;

	// ���C�̌����ɂ�铖����ǂ܂ł̋��������߂�.���x�N�g��(�O�㍶�E).
	const int			RayDireMax	= static_cast<int>( ECrossRayDire::Max );
	bool				IsHit		= false;
	std::vector<CRay*>	RayList		= pCrossRay->GetRay();
	if ( pHitList != nullptr ) pHitList->reserve( 4 );
	for ( int i = 0; i < RayDireMax; ++i ) {
		if ( Coll::IsRayToMesh( RayList[i], pWallMesh, nullptr, nullptr, nullptr, true, WSPACE ) ) {
			IsHit = true;

			// ��������������ǉ�.
			if ( pHitList != nullptr ) pHitList->emplace_back( i );

			// ���C�̈ʒu���X�V.
			for ( int j = 0; j < RayDireMax; ++j ) {
				RayList[j]->SetPosition( RayList[i]->GetPosition() );
			}
		}
	}
	return IsHit;
}



//---------------------------.
// 2D�̃{�b�N�X�Ɠ_�̓����蔻��.
//	�{�b�N�X2D/�_2D �� null / �T�C�Y�� 0 �ȉ��̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll2D::IsBox2DToPoint2D( CBox2D* pMyBox, const D3DXPOSITION2& pOpPos )
{
	if ( pMyBox == nullptr ) return false;
	if ( pOpPos == nullptr ) return false;

	const D3DXPOSITION2& mPos	= pMyBox->GetPosition();
	const SSize&		 mSize	= pMyBox->GetSize();

	if ( mSize.w <= 0.0f ) return false;
	if ( mSize.h <= 0.0f ) return false;

	// �{�b�N�X�Ɠ_���������Ă��邩���ׂ�.
	if ( ( mPos.x < pOpPos.x ) && ( mPos.x + mSize.w > pOpPos.x ) &&
		 ( mPos.y < pOpPos.y ) && ( mPos.y + mSize.h > pOpPos.y ) )
	{
		return true;	// �������Ă���.
	}
	return false;	// �O��Ă���.
}

//---------------------------.
// 2D�̃{�b�N�X���m�̓����蔻��.
//	�{�b�N�X2D�� null / �T�C�Y�� 0 �ȉ��̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll2D::IsBox2DToBox2D( CBox2D* pMyBox, CBox2D* pOpBox )
{
	if ( pMyBox == nullptr ) return false;
	if ( pOpBox == nullptr ) return false;

	const D3DXPOSITION2& mPos	= pMyBox->GetPosition();
	const D3DXPOSITION2& oPos	= pOpBox->GetPosition();
	const SSize&		 mSize	= pMyBox->GetSize();
	const SSize&		 oSize	= pOpBox->GetSize();

	if ( mSize.w <= 0.0f ) return false;
	if ( mSize.h <= 0.0f ) return false;
	if ( oSize.w <= 0.0f ) return false;
	if ( oSize.h <= 0.0f ) return false;

	// �{�b�N�X�ʂ����������Ă��邩���ׂ�.
	if ( ( mPos.x < oPos.x + oSize.w ) && ( mPos.x + mSize.w > oPos.x ) &&
		 ( mPos.y < oPos.y + oSize.h ) && ( mPos.y + mSize.h > oPos.y ) )
	{
		return true;	// �������Ă���.
	}
	return false;	// �O��Ă���.
}

//---------------------------.
// 2D�̉~�Ɠ_�̓����蔻��.
//	�~2D/�_2D�� null / ���a�� 0 �ȉ��̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll2D::IsSphere2DToPoint2D( CSphere2D* pMySphere, const D3DXPOSITION2& pOpPos )
{
	if ( pMySphere	== nullptr ) return false;
	if ( pOpPos		== nullptr ) return false;

	const D3DXPOSITION2& mPos		= pMySphere->GetPosition();
	const float			 mRadius	= pMySphere->GetRadius();

	if ( mRadius <= 0.0f ) return false;

	// �~�Ɠ_���������Ă��邩���ׂ�.
	const float dx = mPos.x - pOpPos.x;
	const float dy = mPos.y - pOpPos.y;
	const float dr = dx * dx + dy * dy;

	const float dl = mRadius * mRadius;
	if ( dr < dl )
	{
		return true;	// �������Ă���.
	}
	return false;	// �O��Ă���.

}

//---------------------------.
// 2D�̉~���m�̓����蔻��.
//	�~2D�� null / ���a�� 0 �ȉ��̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll2D::IsSphere2DToSphere2D( CSphere2D* pMySphere, CSphere2D* pOpSphere )
{
	if ( pMySphere == nullptr ) return false;
	if ( pOpSphere == nullptr ) return false;

	const D3DXPOSITION2& mPos		= pMySphere->GetPosition();
	const D3DXPOSITION2& oPos		= pOpSphere->GetPosition();
	const float			 mRadius	= pMySphere->GetRadius();
	const float			 oRadius	= pOpSphere->GetRadius();

	if ( mRadius <= 0.0f ) return false;
	if ( oRadius <= 0.0f ) return false;

	// �~�ʂ����������Ă��邩���ׂ�.
	const float dx = mPos.x - oPos.x;
	const float dy = mPos.y - oPos.y;
	const float dr = dx * dx + dy * dy;

	const float ar = mRadius + oRadius;
	const float dl = ar * ar;
	if ( dr < dl )
	{
		return true;	// �������Ă���.
	}
	return false;	// �O��Ă���.
}

//---------------------------.
// 2D�̃{�b�N�X�Ɖ~�̓����蔻��.
//	�{�b�N�X/�~2D�� null / �T�C�Y/���a�� 0 �ȉ��̂̏ꍇ�����͍s��Ȃ�.
//---------------------------.
bool Coll2D::IsBox2DToSphere2D( CBox2D* pMyBox, CSphere2D* pOpSphere )
{
	if ( pMyBox		== nullptr ) return false;
	if ( pOpSphere	== nullptr ) return false;

	const D3DXPOSITION2& mPos		= pMyBox->GetPosition();
	const D3DXPOSITION2& oPos		= pOpSphere->GetPosition();
	const SSize&		 mSize		= pMyBox->GetSize();
	const float			 oRadius	= pOpSphere->GetRadius();
	const float			 mBoxLeft	= mPos.x;
	const float			 mBoxRight	= mPos.x + mSize.w;
	const float			 mBoxTop	= mPos.y;
	const float			 mBoxBottom = mPos.y + mSize.h;

	if ( mSize.w <= 0.0f ) return false;
	if ( mSize.h <= 0.0f ) return false;
	if ( oRadius <= 0.0f ) return false;

	// �l�p�`�̎l�ӂɑ΂��ĉ~�̔��a�������������Ƃ��~���d�Ȃ��Ă�����.
	if ( ( oPos.x > mBoxLeft	- oRadius ) &&
		 ( oPos.x < mBoxRight	+ oRadius ) &&
		 ( oPos.y > mBoxTop		- oRadius ) &&
		 ( oPos.y < mBoxBottom	+ oRadius ) )
	{
		const float fl = oRadius * oRadius;

		// ����.
		if ( oPos.x < mBoxLeft &&
			 oPos.y < mBoxTop &&
			 DistanceSqrf( mBoxLeft, mBoxTop, oPos.x, oPos.y ) >= fl )
		{
			return false;	// �O��Ă���.
		}

		// ����.
		if ( oPos.x < mBoxLeft &&
			 oPos.y > mBoxBottom &&
			 DistanceSqrf( mBoxLeft, mBoxBottom, oPos.x, oPos.y ) >= fl )
		{
			return false;	// �O��Ă���.
		}

		// �E��.
		if ( oPos.x > mBoxRight &&
			 oPos.y < mBoxTop &&
			 DistanceSqrf( mBoxRight, mBoxTop, oPos.x, oPos.y ) >= fl )
		{
			return false;	// �O��Ă���.
		}

		// �E��.
		if ( oPos.x > mBoxRight &&
			 oPos.y > mBoxBottom &&
			 DistanceSqrf( mBoxRight, mBoxBottom, oPos.x, oPos.y ) >= fl )
		{
			return false;	// �O��Ă���.
		}

		return true;	// �������Ă���.
	}
	return false;		// �O��Ă���.
}
