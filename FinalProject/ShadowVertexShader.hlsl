cbuffer	WORLD_VIEWxPROJ_BUFFER : register(b0)
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
};

VS_OUT main(VS_IN entry)
{
	VS_OUT retu;
	retu.fpos = mul(mul(float4(entry.pos, 1.0f), World), ViewProj);
	return retu;
}