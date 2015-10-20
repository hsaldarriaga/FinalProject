Texture2D	g_texGradient		: register(t0);
SamplerState g_samplerGradient	: register(s0);

cbuffer ShaderParams : register(b0)
{
	float3 light_dir;
	float3 eye_dir;
}

struct PS_IN
{
	float4 Position	 : SV_POSITION;
	float2 TexCoord	 : TEXCOORD0;
	float3 LocalPos	 : TEXCOORD1;
};

float4 main(PS_IN entry) : SV_TARGET
{
	float2 grad = g_texGradient.Sample(g_samplerGradient, entry.TexCoord).xy;
	float3 normal = normalize(float3(grad, 1.0f));
	float3 V = normalize(eye_dir - (float3)entry.LocalPos);
	float3 R = reflect(light_dir, normal);
	float4 Is = 1.0f * pow(saturate(dot(R, V)), 32.0f);
	return Is * float4(0.0390625f, 0.41015625f, 0.578125f, 1.0f);
}