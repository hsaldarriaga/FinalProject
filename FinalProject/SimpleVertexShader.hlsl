cbuffer VERTEX_MATRIX : register(b0)
{
	float4x4 Matrix;
};

struct VS_IN
{
	float3 pos : POSITION;
	float2 uvcoor : TEXCOORD;
};
struct VS_OUT
{
	float4 fpos : SV_POSITION;
	float2 uvcoord : TEXCOORD0;
};

VS_OUT main(VS_IN entry)
{
	VS_OUT retu;
	retu.fpos = mul(float4(entry.pos, 1.0f), Matrix);
	retu.uvcoord = entry.uvcoor;
	return retu;
}