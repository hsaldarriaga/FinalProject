Texture2D	g_texDisplacement	: register(t0); // FFT wave displacement map in VS

SamplerState g_samplerDisplacement	: register(s0);

cbuffer cbChangePerCall : register(b0)
{
	// Transform matrices
	float4x4	g_matLocal;
	float4x4	g_ViewProj;
}

struct VS_OUTPUT
{
	float4 Position	 : SV_POSITION;
	float2 TexCoord	 : TEXCOORD0;
	float3 LocalPos	 : TEXCOORD1;
};

VS_OUTPUT main(float2 pos : POSITION)
{
	VS_OUTPUT OutPut;
	float4 local = mul(float4(pos, 0.0f, 1.0f), g_matLocal);
	float3 displacement = g_texDisplacement.SampleLevel(g_samplerDisplacement, local.xy, 0).xyz;
	local.xyz += displacement;
	OutPut.Position = mul(local, g_ViewProj);
	OutPut.LocalPos = local.xyz;
	OutPut.TexCoord = local.xy;
	return OutPut;
}