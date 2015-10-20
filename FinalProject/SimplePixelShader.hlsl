Texture2D Diffuse: register(t0);
SamplerState SampleDiffuse : register(s0);

struct PS_IN
{
	float4 fpos : SV_POSITION;
	float2 coord: TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
	return Diffuse.Sample(SampleDiffuse, input.coord);
}