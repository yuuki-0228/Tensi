//��۰��ٕϐ�.
//�ݽ����ޯ̧.
//���ؑ��Ɠ����ޯ̧���ނɂȂ��Ă���K�v������.
cbuffer per_mesh : register(b0)	//ڼ޽��ԍ�.
{
	matrix	g_mWVP;		//ܰ���,�ޭ�,��ۼު���݂̕ϊ������s��.
	float4	g_Color;	//�F(RGBA:xyzw).
};


//���_�����.
//������ق̒��_�̕\���ʒu�����肷��.
float4 VS_Main( float4 Pos : POSITION ) : SV_Position
{
	Pos = mul( Pos, g_mWVP );
	return Pos;
}

//�߸�ټ����.
//��ʏ�ɕ]������߸��(�ޯ�1��)�̐F�����肷��.
float4 PS_Main( float4 Pos : SV_Position ) : SV_Target
{
	return g_Color;
}