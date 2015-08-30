Texture2D Diffuse: register(t0);

SamplerState SampleDiffuse :register(s0);
cbuffer CAMERA: register(b0)
{
	float3 eye;
};
cbuffer MATERIAL_LIGHT: register(b1)
{
	float4 Ka; //ambient
	float4 Ks; //specular
	float4 Kd; //diffuse
	float shininess; // how large specular lights are
}

cbuffer LIGHT: register(b2)
{
	float4 color;
	float3 dir;
}

struct PS_IN
{
	float4 fpos : SV_POSITION;
	float2 coord: TEXCOORD0;
	float4 Wpos: TEXCOORD1;
	float3 normal : NORMAL0;
};

float4 main(PS_IN input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float3 V = normalize(eye - (float3)input.Wpos);
	float3 R = reflect(dir, input.normal);

	float4 Ia = Ka * color;
	float4 Id = Kd * saturate(dot(input.normal, dir));
	float4 Is = Ks * pow(saturate(dot(R, V)), shininess);
	return Ia + (Id + Is) * Diffuse.Sample(SampleDiffuse, input.coord);
}