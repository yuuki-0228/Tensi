//��۰��ٕϐ�.
//ø����� ڼ޽� t(n).
Texture2D		g_Texture	: register(t0);
//����ׂ� ڼ޽� s(n).
SamplerState	g_samLinear	: register(s0);

//�ݽ����ޯ̧.
//���ؑ��Ɠ����ޯ̧���ނɂȂ��Ă���K�v������.
cbuffer per_mesh : register(b0)	//ڼ޽��ԍ�.
{
	matrix	g_mWVP		: packoffset(c0);	//ܰ���,�ޭ�,��ۼު���݂̕ϊ������s��.
	float4	g_Color		: packoffset(c4);	//�F(RGBA:xyzw).
	float	g_ViewPortW : packoffset(c5);	//�ޭ��߰ĕ�.
	float	g_ViewPortH : packoffset(c6);	//�ޭ��߰č���.
};

//���_����ނ̏o�����Ұ�.
//���_����ނŐ��䂵�������̒l���߸�ټ���ޑ��ɓn�����߂ɗp�ӂ��Ă���.
struct VS_OUTPUT
{
	float4	Pos		: SV_Position;	//���W(SV_:System-Value Semantics).
	float2	UV		: TEXCOORD0;	//UV���W.
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
	output.Pos.x = (output.Pos.x / g_ViewPortW) * 2.0f - 1.0f;
	output.Pos.y = 1.0f - (output.Pos.y / g_ViewPortH) * 2.0f;

	output.UV = UV;

	return output;
}

//�߸�ټ����.
//��ʏ�ɕ]������߸��(�ޯ�1��)�̐F�����肷��.
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	float4 color = g_Texture.Sample( g_samLinear, input.UV );//�F��Ԃ�.

	//��۸��ѐ���̃��l��ø����������Ă��郿�l�ɂ������킹��.
	color *= g_Color;

	return color;
}