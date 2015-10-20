cbuffer	WORLD_VIEW_PROJ_BUFFER : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Proj;
};
struct VS_IN
{
	float3 pos : POSITION;
};
struct VS_OUT
{
	float4 fpos : SV_POSITION;
};

VS_OUT main(VS_IN entry)
{
	VS_OUT retu;
	retu.fpos = mul(mul(mul(float4(entry.pos, 1.0f), World), View), Proj);
	return retu;
}