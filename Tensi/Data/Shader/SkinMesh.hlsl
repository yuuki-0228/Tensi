/***************************************************************************************************
*	SkinMeshCode Version 2.00
*	LastUpdate	: 2019/10/09.
**/
//��`.
#define MAX_BONE_MATRICES (255)

//�O���[�o��.
Texture2D		g_Texture		: register( t0 );	//�e�N�X�`���[�� ���W�X�^�[t(n).
Texture2D		g_LightTexture	: register( t1 );	//�e�N�X�`���[�� ���W�X�^�[t(n).
SamplerState	g_Sampler		: register( s0 );	//�T���v���[�̓��W�X�^�[s(n).


//�R���X�^���g�o�b�t�@(���b�V������).
cbuffer per_mesh		: register( b0 )
{
	matrix g_mW;		//���[���h�s��.
	matrix g_mWVP;		//���[���h����ˉe�܂ł̕ϊ��s��.
	float4 g_Color;		//�F(RGBA:xyzw).
};
//�R���X�^���g�o�b�t�@(�}�e���A������).
cbuffer per_material	: register( b1 )
{
	float4 g_Ambient	= float4( 0, 0, 0, 0 );	//�A���r�G���g��.
	float4 g_Diffuse	= float4( 1, 0, 0, 0 );	//�g�U����(�F�j
	float4 g_Specular	= float4( 1, 1, 1, 1 );	//���ʔ���.
	float4 g_UV			= float4( 0, 0, 0, 0 );	//���ʔ���.
};
//�R���X�^���g�o�b�t�@(�t���[������).
cbuffer per_frame		: register( b2 )
{
    float4 g_CameraPos;				//�J�����ʒu(�n�_�ʒu).
    float4 g_LightDir;				//���C�g�̕����x�N�g��.
	float4 g_LightIntensity;		//ײĂ̋���.
	float4 g_PointLightIntensity;	//���b�V���ɑ΂��Ẵ|�C���g���C�g�̋���.
	float4 g_vFlag;					//x�F�f�B�U�������g�p���邩�A	y�F�A���t�@�u���b�N���g�p���邩�A	z�F�����œ����ɂ��邩(�ȉ�)�Aw�F�����œ����ɂ��邩(�ȏ�).
	float4 g_AlphaDistance;			//x�F�J�n���鋗��(�ȉ�)�A		y�F�J�n���鋗��(�ȏ�)�A			z�F�����ɂ��Ă����Ԋu.
};
//�{�[���̃|�[�Y�s�񂪓���.
cbuffer per_bones		: register( b3 )
{
	matrix g_mConstBoneWorld[MAX_BONE_MATRICES];
};

//�X�L�j���O��̒��_�E�@��������.
struct Skin
{
	float4 Pos;
	float3 Norm;
};
//�o�[�e�b�N�X�o�b�t�@�[�̓���.
struct VSSkinIn
{
	float3 Pos		: POSITION;		//�ʒu.  
	float3 Norm		: NORMAL;		//���_�@��.
	float2 Tex		: TEXCOORD;		//�e�N�X�`���[���W.
	uint4  Bones	: BONE_INDEX;	//�{�[���̃C���f�b�N�X.
	float4 Weights	: BONE_WEIGHT;	//�{�[���̏d��.
};

//�s�N�Z���V�F�[�_�[�̓��́i�o�[�e�b�N�X�o�b�t�@�[�̏o�́j�@
struct PSSkinIn
{
	float4 Pos	: SV_Position;	//�ʒu.
	float3 Norm : NORMAL;		//���_�@��.
	float2 Tex	: TEXCOORD;		//�e�N�X�`���[���W.
	float4 Color: COLOR0;		//�ŏI�J���[�i���_�V�F�[�_�[�ɂ����Ắj.
};

static const float DITHER_PATTERN[4][4] =
{
	{ 0.00f, 0.32f, 0.08f, 0.40f, },
	{ 0.48f, 0.16f, 0.56f, 0.24f, },
	{ 0.12f, 0.44f, 0.04f, 0.36f, },
	{ 0.60f, 0.28f, 0.52f, 0.20f, },
};


//�w�肵���ԍ��̃{�[���̃|�[�Y�s���Ԃ�.
//�T�u�֐��i�o�[�e�b�N�X�V�F�[�_�[�Ŏg�p�j.
matrix FetchBoneMatrix( uint iBone )
{
	return g_mConstBoneWorld[iBone];
}


//���_���X�L�j���O�i�{�[���ɂ��ړ��j����.
//�T�u�֐��i�o�[�e�b�N�X�V�F�[�_�[�Ŏg�p�j.
Skin SkinVert( VSSkinIn Input )
{
	Skin Output = (Skin)0;

	float4 Pos = float4(Input.Pos,1);
	float3 Norm = Input.Norm;
	//�{�[��0.
	uint iBone	=Input.Bones.x;
	float fWeight= Input.Weights.x;
	matrix m	=  FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );
	//�{�[��1.
	iBone	= Input.Bones.y;
	fWeight	= Input.Weights.y;
	m		= FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );
	//�{�[��2.
	iBone	= Input.Bones.z;
	fWeight	= Input.Weights.z;
	m		= FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );
	//�{�[��3.
	iBone	= Input.Bones.w;
	fWeight	= Input.Weights.w;
	m		= FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );

	return Output;
}

// �o�[�e�b�N�X�V�F�[�_.
PSSkinIn VS_Main( VSSkinIn input )
{
	PSSkinIn output;
	Skin vSkinned = SkinVert( input);

	output.Pos	= mul( vSkinned.Pos, g_mWVP );
	output.Norm	= normalize( mul( vSkinned.Norm, (float3x3)g_mW ) );
	output.Tex	= input.Tex + g_UV.xy;
    float3 LightDir = (float3)normalize( g_LightDir );
	float3 PosWorld	= (float3)mul( vSkinned.Pos, g_mW );
	float3 ViewDir	= normalize( (float3)g_CameraPos - PosWorld );
	float3 Normal	= normalize( output.Norm );
	float4 NL		= saturate( dot( Normal, LightDir ) );

	float3 Reflect = normalize( 2 * (float3)NL * Normal - LightDir );
	float4 specular = pow( saturate( dot( Reflect, ViewDir ) ), 4.0f ); 

	output.Color	= g_Diffuse * NL + specular * g_Specular;

	return output;
}


// �s�N�Z���V�F�[�_.
float4 PS_Main( PSSkinIn input ) : SV_Target
{
	// �e�N�X�`������s�N�Z���F�����o��.
	float4 TexDiffuse		= g_Texture.Sample( g_Sampler, input.Tex );
	float4 LightTexDiffuse	= g_LightTexture.Sample(g_Sampler, input.Tex); // �@���F.
	// �o�͗p���쐬.
	float4 finalColor = ((input.Color / 2.0f) * (1.0f - LightTexDiffuse.r * 1.0f) + TexDiffuse / 2.0f) * (g_LightIntensity.x + (g_PointLightIntensity.x - g_LightIntensity.x) * LightTexDiffuse.r);

	// �e�N�X�`���̃��l�����̂܂܎g�p�B
	finalColor.a = TexDiffuse.a;
	finalColor *= g_Color;
	
	// �J�����̋����ɂ���ē����ɂ��Ă���.
	if ( g_vFlag.z >= 1.0f ) {
		const float Distance = length( mul( input.Pos, g_mWVP ) - g_CameraPos );
		if ( Distance <= g_AlphaDistance.x * 1000.0f ) {
			const float OverDistance = g_AlphaDistance.x * 1000.0f - Distance;
			finalColor.a -= OverDistance / ( g_AlphaDistance.z * 10.0f ) / 10000.0f;
		}
	}
	if ( g_vFlag.w >= 1.0f ) {
		const float Distance = length( mul( input.Pos, g_mWVP ) - g_CameraPos );
		if ( Distance >= g_AlphaDistance.y * 1000.0f ) {
			const float OverDistance = Distance - g_AlphaDistance.y * 1000.0f;
			finalColor.a -= OverDistance / ( g_AlphaDistance.z * 10.0f ) / 10000.0f;
		}
	}

	// �A���t�@�u���b�N�v�Z.
	if ( g_vFlag.y >= 1.0f ){
		clip(finalColor.a - 0.0001f);
	}
	
	// �f�B�U�����v�Z.
	if ( g_vFlag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip(finalColor.a - dither);
	}
	
	return finalColor;
}
