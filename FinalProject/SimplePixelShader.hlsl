Texture2D Diffuse: register(t0);

SamplerState SampleDiffuse : register(s0);

cbuffer LIGHT: register(b1)
{
	float4 color;
	float3 dir;
}
struct PS_IN
{
	float4 fpos : SV_POSITION;
	float2 coord: TEXCOORD0;
	float3 normal : NORMAL0;
};

float4 main(PS_IN input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float4 Id = saturate(dot(input.normal, -dir));
	return Id * Diffuse.Sample(SampleDiffuse, input.coord);
}