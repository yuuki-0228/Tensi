//��۰��ٕϐ�.
//ø����� ڼ޽� t(n).
Texture2D		g_Texture	: register(t0);
//����ׂ� ڼ޽� s(n).
SamplerState	g_SamLinear	: register(s0);

//�ݽ����ޯ̧.
//ү���P��.
cbuffer per_mesh	: register( b0 )
{
	matrix	g_mW;		//ܰ��ލs��.
	matrix	g_mWVP;		//ܰ���,�ޭ�,��ۼު���݂̍����s��.
    float4  g_Color;    //�F(RGBA:xyzw).
};
//��رْP��.
cbuffer per_material: register( b1 )
{
	float4	g_Ambient;		// �A���r�G���g�F(���F).
	float4	g_Diffuse;		// �f�B�t���[�Y�F(�g�U���ːF).
	float4	g_Specular;		// �X�y�L�����F(���ʔ��ːF).
	float4	g_UV;			// UV(x,y�̂ݎg�p����).
};
//�ڰђP��.
cbuffer per_frame	: register( b2 )
{
	float4 g_CameraPos;				//��׈ʒu(���_�ʒu).
	float4 g_LightDir;				//ײĂ̕����޸��.
	float4 g_LightIntensity;		//ײĂ̋���.
	float4 g_vFlag;					//x�F�޲�ޔ������g�p���邩�A	y�F��̧��ۯ����g�p���邩�Az�F�����œ����ɂ��邩(�ȉ�)�Aw�F�����œ����ɂ��邩(�ȏ�).
	float4 g_AlphaDistance;			//x�F�J�n���鋗��(�ȉ�)�A		y�F�J�n���鋗��(�ȏ�)�A	z�F�����ɂ��Ă����Ԋu.
}

//���_����ނ̏o�����Ұ�.
struct VS_OUTPUT
{
	float4	Pos			: SV_Position;
	float3	Normal		: TEXCOORD0;
	float2	UV			: TEXCOORD1;
	float3	Light		: TEXCOORD2;
	float3	EyeVector	: TEXCOORD3;
	float4	PosWorld	: TEXCOORD4;
	float4	Color		: COLOR0;
};

static const float DITHER_PATTERN[4][4] = 
{
	{ 0.00f, 0.32f, 0.08f, 0.40f, },
	{ 0.48f, 0.16f, 0.56f, 0.24f, },
	{ 0.12f, 0.44f, 0.04f, 0.36f, },
	{ 0.60f, 0.28f, 0.52f, 0.20f, },
};

//-------------------------------------------------
//	���_(�ްï��)�����.
//-------------------------------------------------
VS_OUTPUT VS_Main(
	float4 Pos	: POSITION,
	float4 Norm	: NORMAL,
	float2 UV	: TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//��ۼު���ݕϊ�(ܰ��ށ��ޭ�����ۼު����).
	output.Pos = mul( Pos, g_mWVP );

	//�@�������ق̎p���ɍ��킹��.
	// (���ق���]����Ζ@������]������K�v�����邽��).
    output.Normal = mul( ( float1x3 )Norm, ( float3x3 )g_mW );
    output.Normal = normalize( output.Normal );
    
	//ײĕ���:
	// �ިڸ����ײẮA�ǂ��ł���������.�ʒu�͖��֌W.
	output.Light = (float3)normalize( g_LightDir );

	output.PosWorld = mul( Pos, g_mW );

	//�����޸��:
	// ܰ��ދ�ԏ�ł̒��_���璸�_�֌������޸��.
	output.EyeVector = (float3)normalize( g_CameraPos - output.PosWorld );

	//ø������W.
	output.UV = UV + g_UV.xy;

	return output;
}

//*************************************************
//	׽�ײ��(���ؑ��Őݒ肷�鍀�ځB�����ł͐G��Ȃ�).
//*************************************************

//-------------------------------------------------
//	�߸�ټ����.
//-------------------------------------------------
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	//ø����װ.
	float4 texColor = g_Texture.Sample( g_SamLinear, input.UV );

	//�����@�@.
	float4 ambient = texColor * g_Ambient;

	//�g�U���ˌ� �A.
    float NL = saturate( dot( input.Normal, input.Light ) );
	NL = NL * 0.5f + 0.5f;
	NL = NL * NL;
    float4 diffuse = ( g_Diffuse / 2 + texColor / 2 ) * NL;

	//���ʔ��ˌ� �B.
	float3 reflect = normalize( 2 * NL * input.Normal - input.Light );
	float4 specular =
		pow( saturate( dot( reflect, input.EyeVector ) ), 4 )*g_Specular;
	
	//�ŏI�F�@�@�A�B�̍��v.
	float4 finalColor = ( ambient + diffuse + specular ) * g_LightIntensity.x;
	finalColor *= g_Color;
	
	// �J�����̋����ɂ���ē����ɂ��Ă���.
	if ( g_vFlag.z >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance <= g_AlphaDistance.x ) {
			const float OverDistance = Distance - g_AlphaDistance.x;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	if ( g_vFlag.w >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance >= g_AlphaDistance.y ) {
			const float OverDistance = Distance - g_AlphaDistance.y;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	
	// �A���t�@�u���b�N�v�Z.
	if ( g_vFlag.y >= 1.0f ) {
		clip( finalColor.a - 0.0001f );
	}
	
	// �f�B�U�����v�Z.
	if ( g_vFlag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip( finalColor.a - dither );
	}
	
	return finalColor;
}

//========= ø��������p ========================================
//-------------------------------------------------
//	���_(�ްï��)�����.
//-------------------------------------------------
VS_OUTPUT VS_NoTex(
	float4 Pos : POSITION,
	float4 Norm: NORMAL)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//��ۼު���ݕϊ�(ܰ��ށ��ޭ�����ۼު����).
	output.Pos = mul( Pos, g_mWVP );

	//�@�������ق̎p���ɍ��킹��.
	// (���ق���]����Ζ@������]������K�v�����邽��).
    output.Normal = mul( ( float1x3 )Norm, ( float3x3 )g_mW );
    
	//ײĕ���:
	// �ިڸ����ײẮA�ǂ��ł���������.�ʒu�͖��֌W.
	output.Light = (float3)normalize( g_LightDir );

	output.PosWorld = mul( Pos, g_mW );

	//�����޸��:
	// ܰ��ދ�ԏ�ł̒��_���璸�_�֌������޸��.
	output.EyeVector = (float3)normalize( g_CameraPos - output.PosWorld );

	return output;
}

//-------------------------------------------------
//	�߸�ټ����.
//-------------------------------------------------
float4 PS_NoTex( VS_OUTPUT input ) : SV_Target
{
	float4 finalColor = g_Color;
	
	// �J�����̋����ɂ���ē����ɂ��Ă���.
	if ( g_vFlag.z >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance <= g_AlphaDistance.x ) {
			const float OverDistance = Distance - g_AlphaDistance.x;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	if ( g_vFlag.w >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance >= g_AlphaDistance.y ) {
			const float OverDistance = Distance - g_AlphaDistance.y;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	
	// �A���t�@�u���b�N�v�Z.
	if ( g_vFlag.y >= 1.0f){
		clip(finalColor.a - 0.0001f);
	}
	
	// �f�B�U�����v�Z.
	if ( g_vFlag.x >= 1.0f ){
		const int pt_x = (int) fmod(input.Pos.x, 4.0f);
		const int pt_y = (int) fmod(input.Pos.y, 4.0f);
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip(finalColor.a - dither);
	}
	return finalColor;
}
