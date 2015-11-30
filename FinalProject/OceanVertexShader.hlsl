Texture2D	g_texDisplacement	: register(t0); // FFT wave displacement map in VS
Texture2D	g_TerrainHeightMap	: register(t1);

SamplerState g_samplerDisplacement	: register(s0);

cbuffer cbChangePerCall : register(b0)
{
	// Transform matrices
	float4x4	g_matLocal;
	float4x4	g_ViewProj;
}

cbuffer	WORLD_VIEW_PROJ_BUFFER : register(b1)
{
	float4x4 World;
	float4x4 View;
	float4x4 Proj;
};

struct VS_OUTPUT
{
	float4 Position	 : SV_POSITION;
	float2 TexCoord	 : TEXCOORD0;
	float3 LocalPos	 : TEXCOORD1;
	float Height : TEXCOORD2;
};

VS_OUTPUT main(float2 pos : POSITION)
{
	VS_OUTPUT OutPut;
	float4 local = mul(float4(pos.x, 5.0f, pos.y, 1.0f), g_matLocal);
	float2 uv = float2((local.x + 256.0f) / 512.0f, (local.z + 256.0f) / 512.0f);
	float3 displacement = g_texDisplacement.SampleLevel(g_samplerDisplacement, uv, 0).xzy;
	local.xyz += displacement.xyz;
	OutPut.Position = mul(local, g_ViewProj);
	OutPut.LocalPos = local.xyz;
	OutPut.TexCoord = uv;
	OutPut.Position = mul(local, g_ViewProj);
	if (local.x > -251.5373225f && local.x < 251.5373225f && local.z > -251.5373225f && local.z < 251.5373225f) {
		float2 uv = float2((0.5f * local.x) / 251.5373225f + 0.5f, (0.5f * local.z) / 251.5373225f + 0.5f);
		float Height = -0.0702662021f + g_TerrainHeightMap.SampleLevel(g_samplerDisplacement, uv, 0).r * 0.1659125984f;
		float4 TerrainLocal = mul(float4(uv.x, Height, uv.y, 1.0f), World);
		OutPut.Height = TerrainLocal.y;
	} else {
		OutPut.Height = -500.0f;	
	}
	return OutPut;
}