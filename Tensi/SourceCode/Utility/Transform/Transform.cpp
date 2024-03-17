#include "Transform.h"
#include "..\Math\Math.h"

stTransform::stTransform()
	: Position			( 0.0f, 0.0f, 0.0f )
	, Rotation			( 0.0f, 0.0f, 0.0f )
	, Scale				( 1.0f, 1.0f, 1.0f )
	, OldPosition		( 0.0f, 0.0f, 0.0f )
	, OldRotation		( 0.0f, 0.0f, 0.0f )
	, OldScale			( 1.0f, 1.0f, 1.0f )
	, pParent			( nullptr )
	, IsPostionRender	( true )
{
}

stTransform::stTransform(
	const D3DXPOSITION3&	NewPos,
	const D3DXROTATION3&	NewRot,
	const D3DXSCALE3&		NewScale,
	stTransform*			pNewParent )
	: Position			( NewPos )
	, Rotation			( NewRot )
	, Scale				( NewScale )
	, OldPosition		( NewPos )
	, OldRotation		( NewRot )
	, OldScale			( NewScale )
	, pParent			( pNewParent )
	, IsPostionRender	( false )
{
}

//---------------------------.
// ���[���h���W��ݒ�.
//---------------------------.
void stTransform::SetWorldPosition( const D3DXPOSITION3& WorldPos )
{
	Position = WorldPos - pParent->GetWorldPosition();
}

//---------------------------.
// ���[���h��]��ݒ�.
//---------------------------.
void stTransform::SetWorldRotation( const D3DXROTATION3& WorldRot )
{
	Rotation = WorldRot - pParent->GetWorldRotation();
}

//---------------------------.
// ���[���h�g�k��ݒ�.
//---------------------------.
void stTransform::SetWorldScale( const D3DXSCALE3& WorldScale )
{
	Scale = WorldScale - pParent->GetWorldScale();
}

//---------------------------.
// �x���ŉ�]���擾.
//---------------------------.
D3DXVECTOR3 stTransform::GetDegRotation()
{
	return Math::ToDegree( Rotation );
}

//---------------------------.
// ���W�A���ŉ�]���擾����.
//---------------------------.
D3DXVECTOR3 stTransform::GetRadRotation()
{
	return Math::ToRadian( Rotation );
}

//---------------------------.
// ��]�����̃x�N�g�����擾����.
//---------------------------.
D3DXVECTOR3 stTransform::GetRotationVector()
{
	return D3DXVECTOR3(
		sinf(  Rotation.y ),
		sinf( -Rotation.x ),
		cosf(  Rotation.y ) );
}

//---------------------------.
// ����̃g�����X�t�H�[�����烍�[�J���g�����X�t�H�[�����擾.
//---------------------------.
stTransform stTransform::GetLocalTransform( const stTransform& Parent ) const
{
	return stTransform(
		GetLocalPosition(	Parent.GetWorldPosition()	),
		GetLocalRotation(	Parent.GetWorldRotation()	),
		GetLocalScale(		Parent.GetWorldScale()		) );
}

//---------------------------.
// ����̍��W���烍�[�J�����W���擾.
//---------------------------.
D3DXPOSITION3 stTransform::GetLocalPosition( const D3DXPOSITION3& ParPos ) const
{
	return GetWorldPosition() - ParPos;
}

//---------------------------.
// ����̉�]�l���烍�[�J����]���擾.
//---------------------------.
D3DXROTATION3 stTransform::GetLocalRotation( const D3DXROTATION3& ParRot ) const
{
	return GetWorldRotation() - ParRot;
}

//---------------------------.
// ����̊g�k���烍�[�J���g�k���擾.
//---------------------------.
D3DXSCALE3 stTransform::GetLocalScale( const D3DXSCALE3& ParScale ) const
{
	const D3DXSCALE3& WorldScalse = GetWorldScale();
	return D3DXSCALE3(
		ParScale.x / WorldScalse.x,
		ParScale.y / WorldScalse.y,
		ParScale.z / WorldScalse.z );
}

//---------------------------.
// ���[���h��Ԃ̃g�����X�t�H�[�����擾.
//---------------------------.
stTransform stTransform::GetWorldTransform( const D3DXMATRIX* pRot ) const
{
	return stTransform( GetWorldPosition( pRot ), GetWorldRotation(), GetWorldScale() );
}

//---------------------------.
// ���[���h��Ԃ̍��W���擾.
//---------------------------.
D3DXPOSITION3 stTransform::GetWorldPosition( const D3DXMATRIX* pRot ) const
{
	// ���[���h�s��쐬.
	if ( pParent == nullptr ) return Position;

	D3DXMATRIX mWorld, mScale, mRot, mTran, mParScale, mParRot, mParTran;

	// �g�k�s��쐬.
	D3DXMatrixScaling( &mScale, Scale.x, Scale.y, Scale.z );
	// ��]�s��쐬.
	if ( pRot == nullptr )	D3DXMatrixRotationYawPitchRoll( &mRot, Rotation.y, Rotation.x, Rotation.z );
	else					mRot = *pRot;
	// ���s�ړ��s��.
	D3DXMatrixTranslation( &mTran, Position.x, Position.y, Position.z );

	// �ŏI�̊g�k�s��쐬.
	const D3DXSCALE3& ParentWorldScale = pParent->GetWorldScale();
	D3DXMatrixScaling( &mParScale, ParentWorldScale.x, ParentWorldScale.y, ParentWorldScale.z );
	// �e�̒��S��]�s��쐬.
	const D3DXROTATION3& ParentWorldRotation	= pParent->GetWorldRotation();
	D3DXMatrixRotationYawPitchRoll( &mParRot, ParentWorldRotation.y, ParentWorldRotation.x, ParentWorldRotation.z );
	// �e�̈ړ��s��.
	const D3DXPOSITION3& ParentWorldPosition	= pParent->GetWorldPosition();
	D3DXMatrixTranslation( &mParTran, ParentWorldPosition.x, ParentWorldPosition.y, ParentWorldPosition.z );

	// �e���܂߂����[���h�s����쐬.
	mWorld = mScale * mRot * mTran * mParScale * mParRot * mParTran;
	return Math::GetPosFromWorldMatrix( mWorld );
}

//---------------------------.
// ���[���h��Ԃ̉�]���擾.
//---------------------------.
D3DXROTATION3 stTransform::GetWorldRotation() const
{
	return pParent == nullptr ?
		Rotation :
		pParent->GetWorldRotation() + Rotation;
}

//---------------------------.
// ���[���h��Ԃ̊g�k���擾.
//---------------------------.
D3DXSCALE3 stTransform::GetWorldScale() const
{
	if ( pParent == nullptr ) return Scale;
	const D3DXSCALE3& ParScale = pParent->GetWorldScale();
	return D3DXSCALE3(
		ParScale.x * Scale.x,
		ParScale.y * Scale.y,
		ParScale.z * Scale.z );
}

//---------------------------.
//�@���[���h�s����擾.
//---------------------------.
D3DXMATRIX stTransform::GetWorldMatrix( const D3DXMATRIX* pRot )
{
	D3DXMATRIX mScale, mRot, mTran;

	// ���݂̏�Ԃ�ۊ�.
	OldPosition	= Position;
	OldRotation	= Rotation;
	OldScale	= Scale;

	// �g�k�s��쐬.
	D3DXMatrixScaling( &mScale, Scale.x, Scale.y, Scale.z );
	// ��]�s��쐬.
	if ( pRot == nullptr )	D3DXMatrixRotationYawPitchRoll( &mRot, Rotation.y, Rotation.x, Rotation.z );
	else					mRot = *pRot;
	// ���s�ړ��s��.
	D3DXMatrixTranslation( &mTran, Position.x, Position.y, Position.z );

	// ���[���h�s��쐬.
	if ( pParent == nullptr ) return mScale * mRot * mTran;
	else {
		D3DXMATRIX mParScale, mParRot, mParTran;

		// �ŏI�̊g�k�s��쐬.
		const D3DXSCALE3& ParentWorldScale = pParent->GetWorldScale();
		D3DXMatrixScaling( &mParScale, ParentWorldScale.x, ParentWorldScale.y, ParentWorldScale.z );
		// �e�̒��S��]�s��쐬.
		const D3DXROTATION3& ParentWorldRotation	= pParent->GetWorldRotation();
		D3DXMatrixRotationYawPitchRoll( &mParRot, ParentWorldRotation.y, ParentWorldRotation.x, ParentWorldRotation.z );
		// �e�̈ړ��s��.
		const D3DXPOSITION3& ParentWorldPosition	= pParent->GetWorldPosition();
		D3DXMatrixTranslation( &mParTran, ParentWorldPosition.x, ParentWorldPosition.y, ParentWorldPosition.z );

		// �e���܂߂����[���h�s����쐬.
		return mScale * mRot * mTran * mParScale * mParRot * mParTran;
	}
}

//---------------------------.
// �e��ݒ肷��.
//---------------------------.
void stTransform::AttachParent( stTransform* NewParent )
{
	*this	= GetLocalTransform( *NewParent );
	pParent = NewParent;
}

//---------------------------.
// �e��ǉ�����.
//---------------------------.
void stTransform::AddParent( stTransform* NewParent )
{
	// ��ԏ�̐e��T��.
	stTransform* TopParent = this;
	while ( TopParent->pParent != nullptr ) {
		TopParent = TopParent->pParent;
	}

	// �e��ǉ�.
	*TopParent = GetLocalTransform( *NewParent );
	TopParent->pParent = NewParent;
}

//---------------------------.
// �e�I�u�W�F�N�g����؂藣��.
//---------------------------.
void stTransform::DetachParent()
{
	*this = GetWorldTransform();
}

//---------------------------.
// ���W��͈͓��Ɏ��߂�.
//---------------------------.
void stTransform::PostionClamp( const float Max )
{
	PostionClamp( Max, -Max, Max, -Max, Max, -Max );
}
void stTransform::PostionClamp( const float Max, const float Min )
{
	PostionClamp( Max, Min, Max, Min, Max, Min );
}
void stTransform::PostionClamp( const float Max_x, const float Max_y, const float Max_z )
{
	PostionClamp( Max_x, -Max_x, Max_y, -Max_y, Max_z, -Max_z );
}
void stTransform::PostionClamp( const float Max_x, const float Min_x, const float Max_y, const float Min_y, const float Max_z, const float Min_z )
{
	Position.x = std::clamp( Position.x, Min_x, Max_x );
	Position.y = std::clamp( Position.y, Min_x, Max_x );
	Position.z = std::clamp( Position.z, Min_x, Max_x );
}

//---------------------------.
// ��]��͈͓��Ɏ��߂�.
//---------------------------.
void stTransform::RotationClamp()
{
	if ( Rotation.x < Math::RADIAN_MIN ) Rotation.x += Math::RADIAN_MAX;
	if ( Rotation.x > Math::RADIAN_MAX ) Rotation.x -= Math::RADIAN_MAX;
	if ( Rotation.y < Math::RADIAN_MIN ) Rotation.y += Math::RADIAN_MAX;
	if ( Rotation.y > Math::RADIAN_MAX ) Rotation.y -= Math::RADIAN_MAX;
	if ( Rotation.z < Math::RADIAN_MIN ) Rotation.z += Math::RADIAN_MAX;
	if ( Rotation.z > Math::RADIAN_MAX ) Rotation.z -= Math::RADIAN_MAX;
}

//---------------------------.
// �I�y���[�^�[.
//---------------------------.
bool stTransform::operator==( const stTransform& Trans )
{
	if ( this->Position != Trans.Position	) return false;
	if ( this->Rotation != Trans.Rotation	) return false;
	if ( this->Scale	!= Trans.Scale		) return false;
	return true;
}
bool stTransform::operator!=( const stTransform& Trans )
{
	if ( this->Position != Trans.Position	) return true;
	if ( this->Rotation != Trans.Rotation	) return true;
	if ( this->Scale	!= Trans.Scale		) return true;
	return false;
}
void stTransform::operator+=( const stTransform& Trans )
{
	this->Position	+= Trans.Position;
	this->Rotation	+= Trans.Rotation;
	this->Scale		+= Trans.Scale;
}
void stTransform::operator-=( const stTransform& Trans )
{
	this->Position	-= Trans.Position;
	this->Rotation	-= Trans.Rotation;
	this->Scale		-= Trans.Scale;
}
void stTransform::operator*=( const float& f )
{
	this->Position	*= f;
	this->Rotation	*= f;
	this->Scale		*= f;
}
void stTransform::operator/=( const float& f )
{
	this->Position	/= f;
	this->Rotation	/= f;
	this->Scale		/= f;
}
stTransform stTransform::operator+( const stTransform& Trans ) const
{
	return stTransform(
		this->Position	+ Trans.Position,
		this->Rotation	+ Trans.Rotation,
		this->Scale		+ Trans.Scale,
		this->pParent
	);
}
stTransform stTransform::operator-( const stTransform& Trans ) const
{
	return stTransform(
		this->Position	- Trans.Position,
		this->Rotation	- Trans.Rotation,
		this->Scale		- Trans.Scale,
		this->pParent
	);
}
stTransform stTransform::operator*( const float& f ) const
{
	return stTransform(
		this->Position	* f,
		this->Rotation	* f,
		this->Scale		* f,
		this->pParent
	);
}
stTransform stTransform::operator/( const float& f ) const
{
	return stTransform(
		this->Position	/ f,
		this->Rotation	/ f,
		this->Scale		/ f,
		this->pParent
	);
}
