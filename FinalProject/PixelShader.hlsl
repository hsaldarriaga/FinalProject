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
	//float LightPos : TEXCOORD4;
};

float4 main(PS_IN input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float4 Ia = Ka * color;
	float4 Id = Kd * max(saturate(dot(input.normal, -dir)), 0.3f);
	//------------------------
	int shadowindex;
	if (input.vDepth <= Intervals.x)
		shadowindex = 0;
	else if (input.vDepth <= Intervals.y)
		shadowindex = 1;
	else
		shadowindex = 2;
	//--------------------------
	float bias = 0.001f;
	float3 ProjTexCoord;
	ProjTexCoord.x = input.LightViewProj.x / input.LightViewProj.z / 2.0f + 0.5f;
	ProjTexCoord.y = -input.LightViewProj.y / input.LightViewProj.z / 2.0f + 0.5f;
	ProjTexCoord.z = shadowindex;
	float LightDepth = input.LightViewProj.z / input.LightViewProj.w;
	if (saturate(ProjTexCoord.x) == ProjTexCoord.x && saturate(ProjTexCoord.y) == ProjTexCoord.y && LightDepth > 0)
	{
		float depthfactor = ShadowMap.SampleCmpLevelZero(SampleShadow, ProjTexCoord, LightDepth - bias);
		return Ia + Id * depthfactor * Diffuse.Sample(SampleDiffuse, input.coord);
	}
	//--------------------------
	return Ia + Id * Diffuse.Sample(SampleDiffuse, input.coord);
}