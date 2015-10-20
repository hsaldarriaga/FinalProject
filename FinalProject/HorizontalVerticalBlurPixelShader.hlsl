static const float4 weights = {0.241732f, 0.060598f, 0.005977f, 0.000229f};
static const float middleweight = 0.382928f;

Texture2D shaderTexture: register(t0);
SamplerState SampleType : register(s0);
cbuffer	COLOR_BF : register(b0)
{
	float4 Kd;
};

struct PS_IN
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

float4 main(PS_IN entry) : SV_TARGET
{
	float4 poscolor = shaderTexture.Sample(SampleType, entry.uvcoord);
	//if (poscolor.r == Kd.r && poscolor.g == Kd.g && poscolor.b == Kd.b)
	//{
		float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
		color += shaderTexture.Sample(SampleType, entry.texCoord1) * weights[3];
		color += shaderTexture.Sample(SampleType, entry.texCoord2) * weights[2];
		color += shaderTexture.Sample(SampleType, entry.texCoord3) * weights[1];
		color += shaderTexture.Sample(SampleType, entry.texCoord4) * weights[0];
		color += shaderTexture.Sample(SampleType, entry.texCoord5) * middleweight;
		color += shaderTexture.Sample(SampleType, entry.texCoord6) * weights[0];
		color += shaderTexture.Sample(SampleType, entry.texCoord7) * weights[1];
		color += shaderTexture.Sample(SampleType, entry.texCoord8) * weights[2];
		color += shaderTexture.Sample(SampleType, entry.texCoord9) * weights[3];
		//color.a = 1.0f;
		return color;
	//}
	//return poscolor;
}