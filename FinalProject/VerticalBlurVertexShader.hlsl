cbuffer	CONTENTS : register(b0)
{
	float4x4 Matrix;
};
cbuffer	SIZES : register(b1)
{
	float ScreenHeight;
};
struct VS_IN
{
	float3 pos : POSITION;
	float2 uvcoor : TEXCOORD;
};
struct VS_OUT
{
	float4 fpos : SV_POSITION;
	float2 uvcoord: TEXCOORD0;
	float2 texCoord1 : TEXCOORD1;
	float2 texCoord2 : TEXCOORD2;
	float2 texCoord3 : TEXCOORD3;
	float2 texCoord4 : TEXCOORD4;
	float2 texCoord5 : TEXCOORD5;
	float2 texCoord6 : TEXCOORD6;
	float2 texCoord7 : TEXCOORD7;
	float2 texCoord8 : TEXCOORD8;
	float2 texCoord9 : TEXCOORD9;
};
VS_OUT main(VS_IN input)
{
	VS_OUT retu;
	retu.fpos = float4(input.pos, 1.0f);
	retu.fpos = mul(float4(input.pos, 1.0f), Matrix);
	retu.uvcoord = input.uvcoor;
	float texelSize = 1.0f / ScreenHeight;
	retu.texCoord1 = input.uvcoor + float2(0.0f, texelSize * -4.0f);
	retu.texCoord2 = input.uvcoor + float2(0.0f, texelSize * -3.0f);
	retu.texCoord3 = input.uvcoor + float2(0.0f, texelSize * -2.0f);
	retu.texCoord4 = input.uvcoor + float2(0.0f, texelSize * -1.0f);
	retu.texCoord5 = input.uvcoor + float2(0.0f, texelSize *  0.0f);
	retu.texCoord6 = input.uvcoor + float2(0.0f, texelSize *  1.0f);
	retu.texCoord7 = input.uvcoor + float2(0.0f, texelSize *  2.0f);
	retu.texCoord8 = input.uvcoor + float2(0.0f, texelSize *  3.0f);
	retu.texCoord9 = input.uvcoor + float2(0.0f, texelSize *  4.0f);
	return retu;
}