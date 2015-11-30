cbuffer	WORLD_VIEW_PROJ_BUFFER : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Proj;
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
	float3 normal : NORMAL0;
};

VS_OUT main(VS_IN entry)
{
	VS_OUT retu;
	float4 WorldPos = mul(float4(entry.pos, 1.0f), World);
	retu.fpos = mul(mul(WorldPos, View), Proj);
	retu.normal = normalize(mul(entry.normal, (float3x3)World));
	retu.uvcoord = entry.uvcoor;
	return retu;
}