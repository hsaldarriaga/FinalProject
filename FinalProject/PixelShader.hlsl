#ifndef CASCADE_COUNT_FLAG
#define CASCADE_COUNT_FLAG 3
#endif

Texture2D Diffuse: register(t0);

Texture2DArray ShadowMap: register(t1);

SamplerState SampleDiffuse : register(s0);

SamplerComparisonState SampleShadow : register(s1);

cbuffer MATERIAL_LIGHT: register(b0)
{
	float4 Ka; //ambient
	float4 Kd; //diffuse
}

cbuffer LIGHT: register(b1)
{
	float4 color;
	float3 dir;
}

cbuffer SHADOW_INTERVALS: register(b2)
{
	float3 Intervals;
}

struct PS_IN
{
	float4 fpos : SV_POSITION;
	float2 coord: TEXCOORD0;
	//float4 Wpos: TEXCOORD1;
	float3 normal : NORMAL0;
	float vDepth : TEXCOORD2;
	float4 LightViewProj : TEXCOORD3;
	//float3 LightPos : TEXCOORD4;
};

float4 main(PS_IN input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float4 Ia = Ka * color;
	float4 Id = Kd * saturate(dot(input.normal, -dir));
	//------------------------
	float4 vCurrentPixelDepth = input.vDepth;
	int shadowindex = dot(float3(1.0f, 1.0f, 1.0f), (input.vDepth > Intervals.x, input.vDepth > Intervals.y, input.vDepth > Intervals.z));
	//--------------------------
	float bias = 0.001;
	float3 ProjTexCoord;
	ProjTexCoord.x = input.LightViewProj.x / input.LightViewProj.z / 2.0f + 0.5f;
	ProjTexCoord.y = input.LightViewProj.y / input.LightViewProj.z / 2.0f + 0.5f;
	ProjTexCoord.z = shadowindex;
	float LightDepth = input.LightViewProj.z / input.LightViewProj.w;
	/*if (saturate(ProjTexCoord.x) == ProjTexCoord.x && saturate(ProjTexCoord.y) == ProjTexCoord.y && LightDepth > 0)
	{
		float depthValue = ShadowMap.SampleCmpLevelZero(SampleShadow, ProjTexCoord, LightDepth - bias);
		if (LightDepth > depthValue)
		{
			return 0.1* Ia * Diffuse.Sample(SampleDiffuse, input.coord);
		}
	}*/
	//--------------------------
	return Ia + Id * Diffuse.Sample(SampleDiffuse, input.coord);
}