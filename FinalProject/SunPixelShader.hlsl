cbuffer	SUN_PARAMS : register(b0)
{
	float4 Kd; // more blue
};

struct PS_IN
{
	float4 fpos : SV_POSITION;
};

float4 main(PS_IN entry) : SV_TARGET
{
	return Kd;

}