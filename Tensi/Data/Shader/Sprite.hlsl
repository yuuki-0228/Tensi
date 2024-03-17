//��۰��ٕϐ�.
//ø����� ڼ޽� t(n).
Texture2D		g_Texture	    : register(t0);
Texture2D       g_MaskTexture   : register(t1);
Texture2D		g_RuleTexture	: register(t2);
//����ׂ� ڼ޽� s(n).
SamplerState	g_samLinear	    : register(s0);

//�ݽ����ޯ̧.
//���ؑ��Ɠ����ޯ̧���ނɂȂ��Ă���K�v������.
cbuffer per_mesh : register(b0)	//ڼ޽��ԍ�.
{
	matrix	g_mWVP				: packoffset(c0);	//ܰ���,�ޭ�,��ۼު���݂̕ϊ������s��.
	float4	g_Color				: packoffset(c4);	//�F(RGBA:xyzw).
	float4	g_UV				: packoffset(c5);	//UV���W(xy�����g��Ȃ�).
	float4	g_RenderArea		: packoffset(c6);	//�`�悷��͈�(����x, ����y, ��, ����).
	float	g_ViewPortW			: packoffset(c7);	//�ޭ��߰ĕ�.
	float	g_ViewPortH			: packoffset(c8);	//�ޭ��߰č���.
	float4	g_vDitherFlag		: packoffset(c9);	//�޲�ޔ������g�p���邩.
	float4	g_vAlphaBlockFlag	: packoffset(c10);	//��̧��ۯ����g�p���邩.
};

//���_����ނ̏o�����Ұ�.
//���_����ނŐ��䂵�������̒l���߸�ټ���ޑ��ɓn�����߂ɗp�ӂ��Ă���.
struct VS_OUTPUT
{
	float4	Pos		: SV_Position;	//���W(SV_:System-Value Semantics).
	float2	UV		: TEXCOORD0;	//UV���W.
};

static const float DITHER_PATTERN[4][4] = 
{
	{ 0.00f, 0.32f, 0.08f, 0.40f, },
	{ 0.48f, 0.16f, 0.56f, 0.24f, },
	{ 0.12f, 0.44f, 0.04f, 0.36f, },
	{ 0.60f, 0.28f, 0.52f, 0.20f, },
};

//���_�����.
//������ق̒��_�̕\���ʒu�����肷��.
VS_OUTPUT VS_Main(
	float4 Pos	: POSITION,
	float2 UV	: TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul( Pos, g_mWVP );
	
	//��ذݍ��W�ɍ��킹��v�Z.
	if ( g_ViewPortW > 0 && g_ViewPortH > 0 ) {
		output.Pos.x = ( output.Pos.x / g_ViewPortW ) * 2.0f - 1.0f;
		output.Pos.y = 1.0f - ( output.Pos.y / g_ViewPortH ) * 2.0f;
	}
	output.UV = UV;

	//UV��۰�(UV���W�𑀍삷��).
	output.UV.x += g_UV.x;
	output.UV.y += g_UV.y;

	return output;
}

//�߸�ټ����(�}�X�N���g�p���Ȃ�).
//��ʏ�ɕ]������߸��(�ޯ�1��)�̐F�����肷��.
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	float4 finalColor = g_Texture.Sample(g_samLinear, input.UV); //�F��Ԃ�.

	//��۸��ѐ���̐F���������킹��.
	finalColor *= g_Color;
	
	// �`�悷��G���A�O�̏ꍇ�����ɂ���.
	if ( input.Pos.x < g_RenderArea.x || input.Pos.x > g_RenderArea.x + g_RenderArea.z ||
		 input.Pos.y < g_RenderArea.y || input.Pos.y > g_RenderArea.y + g_RenderArea.w )
	{
		finalColor.a = 0.0f;
	}
	
	// �A���t�@�u���b�N�v�Z.
	if (g_vAlphaBlockFlag.x >= 1.0f){
		clip(finalColor.a - 0.0001f);
	}
	
	// �f�B�U�����v�Z.
	if ( g_vDitherFlag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip( finalColor.a - dither );
	}

	return finalColor;
}

//�߸�ټ����(�}�X�N�e�N�X�`�����g�p����).
//��ʏ�ɕ]������߸��(�ޯ�1��)�̐F�����肷��.
float4 PS_Mask(VS_OUTPUT input) : SV_Target
{
	float4 finalColor = g_Texture.Sample(g_samLinear, input.UV); //�F��Ԃ�.
	float4 maskColor  = g_MaskTexture.Sample(g_samLinear, input.UV - g_UV.xy);

	// �F���|�����킹��.
	finalColor.a *= 1.0f - maskColor.r;
	finalColor *= g_Color;
	
	// �`�悷��G���A�O�̏ꍇ�����ɂ���.
	if ( input.Pos.x < g_RenderArea.x || input.Pos.x > g_RenderArea.x + g_RenderArea.z ||
		 input.Pos.y < g_RenderArea.y || input.Pos.y > g_RenderArea.y + g_RenderArea.w )
	{
		finalColor.a = 0.0f;
	}
	
	// �A���t�@�u���b�N�v�Z.
	if (g_vAlphaBlockFlag.x >= 1.0f){
		clip(finalColor.a - 0.0001f);
	}
	
	// �f�B�U�����v�Z.
	if ( g_vDitherFlag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip( finalColor.a - dither );
	}
	
	return finalColor;
}

//�߸�ټ����(���[���e�N�X�`�����g�p����).
//��ʏ�ɕ]������߸��(�ޯ�1��)�̐F�����肷��.
float4 PS_Transition(VS_OUTPUT input) : SV_Target
{
	float4 maskColor = g_RuleTexture.Sample(g_samLinear, input.UV);
	float4 texColor  = g_Texture.Sample(g_samLinear, input.UV);
	
	// �}�X�N���g�����A���t�@����.
	float  maskAlpha  = saturate(maskColor.r + (g_Color.a * 2.0f - 1.0f));
	float4 finalColor = texColor * maskAlpha + texColor * (1 - texColor.a);
	
	// �F���������킹��.
	finalColor.rbg *= g_Color.rgb;
	
	// �`�悷��G���A�O�̏ꍇ�����ɂ���.
	if ( input.Pos.x < g_RenderArea.x || input.Pos.x > g_RenderArea.x + g_RenderArea.z ||
		 input.Pos.y < g_RenderArea.y || input.Pos.y > g_RenderArea.y + g_RenderArea.w )
	{
		finalColor.a = 0.0f;
	}
	
	// �A���t�@�u���b�N�v�Z.
	if (g_vAlphaBlockFlag.x >= 1.0f){
		clip(finalColor.a - 0.0001f);
	}
	
	// �f�B�U�����v�Z.
	if ( g_vDitherFlag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip( finalColor.a - dither );
	}
	
	return finalColor;
}