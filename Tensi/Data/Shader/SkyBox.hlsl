//��۰��ٕϐ�.
//ø����� ڼ޽� t(n).
Texture2D		g_Texture	: register(t0);
//����ׂ� ڼ޽� s(n).
SamplerState	g_SamLinear	: register(s0);

//�ݽ����ޯ̧.
//ү���P��.
cbuffer per_mesh	: register( b0 )
{
	matrix	g_mW;			//ܰ��ލs��.
	matrix	g_mWVP;			//ܰ���,�ޭ�,��ۼު���݂̍����s��.
    float4  g_Color;		//�F(RGBA:xyzw).
};
//�ڰђP��.
cbuffer per_frame : register(b2)
{
	float4 g_vFlag;				//x�F�޲�ޔ������g�p���邩�Ay�F��̧��ۯ����g�p���邩.
}

//���_����ނ̏o�����Ұ�.
struct VS_OUTPUT
{
	float4	Pos			: SV_Position;
	float2	UV			: TEXCOORD1;
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
	float2 UV	: TEXCOORD )
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//��ۼު���ݕϊ�(ܰ��ށ��ޭ�����ۼު����).
	output.Pos		= mul( Pos, g_mWVP );
	output.PosWorld = mul( Pos, g_mW );

	//ø������W.
	output.UV = UV;

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
	float4 finalColor	 = g_Texture.Sample(g_SamLinear, input.UV);
	finalColor			*= g_Color;
	
	// �A���t�@�u���b�N�v�Z.
	if ( g_vFlag.y >= 1.0f ){
		clip(finalColor.a - 0.0001f);
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
VS_OUTPUT VS_NoTex( float4 Pos : POSITION )
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//��ۼު���ݕϊ�(ܰ��ށ��ޭ�����ۼު����).
	output.Pos		= mul( Pos, g_mWVP );
	output.PosWorld = mul( Pos, g_mW );

	return output;
}

//-------------------------------------------------
//	�߸�ټ����.
//-------------------------------------------------
float4 PS_NoTex( VS_OUTPUT input ) : SV_Target
{
	float4 finalColor = g_Color;
	
	// �A���t�@�u���b�N�v�Z.
	if ( g_vFlag.y >= 1.0f ){
		clip(finalColor.a - 0.0001f);
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
