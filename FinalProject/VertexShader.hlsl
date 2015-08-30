cbuffer	VS_CONSTANT : register(b0)
{
	float4x4 World;
	float4x4 ViewProj;
};

struct VS_IN
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uvcoor : TEXCOORD;
};
struct VS_OUT
{
	float4 fpos : SV_POSITION;
	float2 uvcoord: TEXCOORD0;
	float4 Wpos: TEXCOORD1;
	float3 normal : NORMAL0;
};

VS_OUT main(VS_IN entry)
{
	VS_OUT retu;
	retu.fpos = mul(mul(float4(entry.pos, 1.0f), World), ViewProj);
	retu.normal = normalize(mul(entry.normal,(float3x3)World));
	retu.Wpos = mul(float4(entry.pos, 1.0f), World);
	retu.uvcoord = entry.uvcoor;
	return retu;
}