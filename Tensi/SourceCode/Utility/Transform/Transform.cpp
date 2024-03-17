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
// ワールド座標を設定.
//---------------------------.
void stTransform::SetWorldPosition( const D3DXPOSITION3& WorldPos )
{
	Position = WorldPos - pParent->GetWorldPosition();
}

//---------------------------.
// ワールド回転を設定.
//---------------------------.
void stTransform::SetWorldRotation( const D3DXROTATION3& WorldRot )
{
	Rotation = WorldRot - pParent->GetWorldRotation();
}

//---------------------------.
// ワールド拡縮を設定.
//---------------------------.
void stTransform::SetWorldScale( const D3DXSCALE3& WorldScale )
{
	Scale = WorldScale - pParent->GetWorldScale();
}

//---------------------------.
// 度数で回転を取得.
//---------------------------.
D3DXVECTOR3 stTransform::GetDegRotation()
{
	return Math::ToDegree( Rotation );
}

//---------------------------.
// ラジアンで回転を取得する.
//---------------------------.
D3DXVECTOR3 stTransform::GetRadRotation()
{
	return Math::ToRadian( Rotation );
}

//---------------------------.
// 回転方向のベクトルを取得する.
//---------------------------.
D3DXVECTOR3 stTransform::GetRotationVector()
{
	return D3DXVECTOR3(
		sinf(  Rotation.y ),
		sinf( -Rotation.x ),
		cosf(  Rotation.y ) );
}

//---------------------------.
// 特定のトランスフォームからローカルトランスフォームを取得.
//---------------------------.
stTransform stTransform::GetLocalTransform( const stTransform& Parent ) const
{
	return stTransform(
		GetLocalPosition(	Parent.GetWorldPosition()	),
		GetLocalRotation(	Parent.GetWorldRotation()	),
		GetLocalScale(		Parent.GetWorldScale()		) );
}

//---------------------------.
// 特定の座標からローカル座標を取得.
//---------------------------.
D3DXPOSITION3 stTransform::GetLocalPosition( const D3DXPOSITION3& ParPos ) const
{
	return GetWorldPosition() - ParPos;
}

//---------------------------.
// 特定の回転値からローカル回転を取得.
//---------------------------.
D3DXROTATION3 stTransform::GetLocalRotation( const D3DXROTATION3& ParRot ) const
{
	return GetWorldRotation() - ParRot;
}

//---------------------------.
// 特定の拡縮からローカル拡縮を取得.
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
// ワールド空間のトランスフォームを取得.
//---------------------------.
stTransform stTransform::GetWorldTransform( const D3DXMATRIX* pRot ) const
{
	return stTransform( GetWorldPosition( pRot ), GetWorldRotation(), GetWorldScale() );
}

//---------------------------.
// ワールド空間の座標を取得.
//---------------------------.
D3DXPOSITION3 stTransform::GetWorldPosition( const D3DXMATRIX* pRot ) const
{
	// ワールド行列作成.
	if ( pParent == nullptr ) return Position;

	D3DXMATRIX mWorld, mScale, mRot, mTran, mParScale, mParRot, mParTran;

	// 拡縮行列作成.
	D3DXMatrixScaling( &mScale, Scale.x, Scale.y, Scale.z );
	// 回転行列作成.
	if ( pRot == nullptr )	D3DXMatrixRotationYawPitchRoll( &mRot, Rotation.y, Rotation.x, Rotation.z );
	else					mRot = *pRot;
	// 平行移動行列.
	D3DXMatrixTranslation( &mTran, Position.x, Position.y, Position.z );

	// 最終の拡縮行列作成.
	const D3DXSCALE3& ParentWorldScale = pParent->GetWorldScale();
	D3DXMatrixScaling( &mParScale, ParentWorldScale.x, ParentWorldScale.y, ParentWorldScale.z );
	// 親の中心回転行列作成.
	const D3DXROTATION3& ParentWorldRotation	= pParent->GetWorldRotation();
	D3DXMatrixRotationYawPitchRoll( &mParRot, ParentWorldRotation.y, ParentWorldRotation.x, ParentWorldRotation.z );
	// 親の移動行列.
	const D3DXPOSITION3& ParentWorldPosition	= pParent->GetWorldPosition();
	D3DXMatrixTranslation( &mParTran, ParentWorldPosition.x, ParentWorldPosition.y, ParentWorldPosition.z );

	// 親を含めたワールド行列を作成.
	mWorld = mScale * mRot * mTran * mParScale * mParRot * mParTran;
	return Math::GetPosFromWorldMatrix( mWorld );
}

//---------------------------.
// ワールド空間の回転を取得.
//---------------------------.
D3DXROTATION3 stTransform::GetWorldRotation() const
{
	return pParent == nullptr ?
		Rotation :
		pParent->GetWorldRotation() + Rotation;
}

//---------------------------.
// ワールド空間の拡縮を取得.
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
//　ワールド行列を取得.
//---------------------------.
D3DXMATRIX stTransform::GetWorldMatrix( const D3DXMATRIX* pRot )
{
	D3DXMATRIX mScale, mRot, mTran;

	// 現在の状態を保管.
	OldPosition	= Position;
	OldRotation	= Rotation;
	OldScale	= Scale;

	// 拡縮行列作成.
	D3DXMatrixScaling( &mScale, Scale.x, Scale.y, Scale.z );
	// 回転行列作成.
	if ( pRot == nullptr )	D3DXMatrixRotationYawPitchRoll( &mRot, Rotation.y, Rotation.x, Rotation.z );
	else					mRot = *pRot;
	// 平行移動行列.
	D3DXMatrixTranslation( &mTran, Position.x, Position.y, Position.z );

	// ワールド行列作成.
	if ( pParent == nullptr ) return mScale * mRot * mTran;
	else {
		D3DXMATRIX mParScale, mParRot, mParTran;

		// 最終の拡縮行列作成.
		const D3DXSCALE3& ParentWorldScale = pParent->GetWorldScale();
		D3DXMatrixScaling( &mParScale, ParentWorldScale.x, ParentWorldScale.y, ParentWorldScale.z );
		// 親の中心回転行列作成.
		const D3DXROTATION3& ParentWorldRotation	= pParent->GetWorldRotation();
		D3DXMatrixRotationYawPitchRoll( &mParRot, ParentWorldRotation.y, ParentWorldRotation.x, ParentWorldRotation.z );
		// 親の移動行列.
		const D3DXPOSITION3& ParentWorldPosition	= pParent->GetWorldPosition();
		D3DXMatrixTranslation( &mParTran, ParentWorldPosition.x, ParentWorldPosition.y, ParentWorldPosition.z );

		// 親を含めたワールド行列を作成.
		return mScale * mRot * mTran * mParScale * mParRot * mParTran;
	}
}

//---------------------------.
// 親を設定する.
//---------------------------.
void stTransform::AttachParent( stTransform* NewParent )
{
	*this	= GetLocalTransform( *NewParent );
	pParent = NewParent;
}

//---------------------------.
// 親を追加する.
//---------------------------.
void stTransform::AddParent( stTransform* NewParent )
{
	// 一番上の親を探す.
	stTransform* TopParent = this;
	while ( TopParent->pParent != nullptr ) {
		TopParent = TopParent->pParent;
	}

	// 親を追加.
	*TopParent = GetLocalTransform( *NewParent );
	TopParent->pParent = NewParent;
}

//---------------------------.
// 親オブジェクトから切り離す.
//---------------------------.
void stTransform::DetachParent()
{
	*this = GetWorldTransform();
}

//---------------------------.
// 座標を範囲内に収める.
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
// 回転を範囲内に収める.
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
// オペレーター.
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
