#include "EffectManager.h"
#include "..\..\Resource\EffectResource\EffectResource.h"

CEffectManager::CEffectManager()
	: m_pEffect		( nullptr )
	, m_Handle		( -1 )
	, m_Transform	()
{
}

CEffectManager::~CEffectManager()
{
}

//------------------------------.
// �G�t�F�N�g�̐ݒ�.
//------------------------------.
bool CEffectManager::SetEffect( const std::string& Name )
{
	// �G�t�F�N�g���擾.
	m_pEffect = EffectResource::GetEffect( Name );

	return true;
}

//------------------------------.
// �`��.
//	�G�t�F�N�g��ݒ肵�Ă��Ȃ��ꍇ�͏����͍s��Ȃ�.
//------------------------------.
void CEffectManager::Render()
{
	if( m_pEffect	== nullptr	) return;
	if( m_Handle	== -1		) return;

	m_pEffect->Render( m_Handle );

	// �G�t�F�N�g��`�悵�Ă��邩.
	if( m_pEffect->IsShow( m_Handle ) == false ) m_Handle = -1;
}

//------------------------------.
// �Đ�.
//	�G�t�F�N�g��ݒ肵�Ă��Ȃ��ꍇ�͏����͍s��Ȃ�.
//------------------------------.
bool CEffectManager::Play( const STransform& Trans )
{
	return Play( Trans.Position, Trans.Rotation, Trans.Scale );
}
bool CEffectManager::Play( const D3DXPOSITION3& Pos, const D3DXROTATION3& Rot, const D3DXSCALE3& Scale )
{
	if( m_pEffect	== nullptr	) return false;
	if( m_Handle	!= -1		) return false;

	m_Handle				= m_pEffect->Play( Pos );
	m_Transform.Position	= Pos;

	SetRotation( Rot );	// ��]�̐ݒ�.
	SetScale( Scale );	// �T�C�Y�̐ݒ�.

	return true;
}

//------------------------------.
// ��~.
//------------------------------.
void CEffectManager::Stop()
{
	m_pEffect->Stop( m_Handle );
}

//------------------------------.
// ���X�ɒ�~������.
//------------------------------.
void CEffectManager::StopRoot()
{
	m_pEffect->StopRoot( m_Handle );
}

//------------------------------.
// �S�Ē�~.
//------------------------------.
void CEffectManager::StopAll()
{
	m_pEffect->StopAll();
}

//------------------------------.
// �ꎞ��~.
//------------------------------.
void CEffectManager::Pause( const bool& Flag )
{
	m_pEffect->Pause( m_Handle, Flag ); 
}

//------------------------------.
// ���ׂĂ��ꎞ��~�������͍ĊJ����.
//------------------------------.
void CEffectManager::AllPause( const bool& Flag )
{
	m_pEffect->AllPause( Flag );
}

//------------------------------.
// �ʒu���w�肷��.
//------------------------------.
void CEffectManager::SetPosition( const D3DXPOSITION3& Pos )
{
	m_pEffect->SetLocation( m_Handle, Pos );
	m_Transform.Position = Pos;
}

//------------------------------.
// ��]���w�肷��.
//------------------------------.
void CEffectManager::SetRotation( const D3DXROTATION3& Rot )
{
	m_pEffect->SetRotation( m_Handle, Rot );
	m_Transform.Rotation = { Rot };
}

//------------------------------.
// �T�C�Y���w�肷��.
//------------------------------.
void CEffectManager::SetScale( const D3DXSCALE3& Scale )
{
	m_pEffect->SetScale( m_Handle, Scale );
	m_Transform.Scale = Scale;
}
void CEffectManager::SetScale( const float& scale )
{
	m_pEffect->SetScale( m_Handle, { scale, scale, scale } );
	m_Transform.Scale = { scale, scale, scale };
}

//------------------------------.
// �Đ����x��ݒ肷��.
//------------------------------.
void CEffectManager::SetSpeed( const float& Speed )
{
	m_pEffect->SetSpeed( m_Handle, Speed );
}

//------------------------------.
// �F�̐ݒ�.
//------------------------------.
void CEffectManager::SetColor( const D3DXCOLOR3& Color )
{
	SetColor( Color4::RGBA( Color ) );
}
void CEffectManager::SetColor( const D3DXCOLOR4& Color )
{
	m_pEffect->SetColor( m_Handle, Color );
}

//------------------------------.
// �A���t�@�l�̐ݒ�.
//------------------------------.
void CEffectManager::SetAlpha( const float& Alpha )
{
	SetColor( Color4::RGBA( Color::White, Alpha ) );
}

//------------------------------.
// ���݂��Ă��邩.
//------------------------------.
bool CEffectManager::IsShow()
{
	return m_pEffect->IsShow( m_Handle );
}

//------------------------------.
// �G�t�F�N�g�Ɏg�p����Ă���C���X�^���X�����擾.
//------------------------------.
int CEffectManager::GetTotalInstanceCount()
{
	return m_pEffect->GetTotalInstanceCount();
}