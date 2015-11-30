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
	float Height : TEXCOORD2;
};

float4 main(PS_IN entry) : SV_TARGET
{
	float2 grad = g_texGradient.Sample(g_samplerGradient, entry.TexCoord).xy;
	float3 normal = normalize(float3(grad.x, 1.0f, grad.y));
	float3 V = normalize(eye_dir - (float3)entry.LocalPos);
	float3 R = reflect(light_dir, normal);
	float4 value = saturate(dot(R, V));
	float4 Is = 1.0f * pow(value, 20.0f);
	float In = max(saturate(dot(normal, -light_dir)), 0.5f);
	float alpha = 0.0f;
	float Hdiff = entry.Height - entry.LocalPos.y;
	if (Hdiff > -15.0f && Hdiff < 1.5f) {
		alpha = (Hdiff + 16.0f) / 16.0f;
		if (Hdiff > -5.0f) {
			float4 white = float4(0.890623f, 0.58203125f, 0.375f, 0.0f) * 0.7f * ((Hdiff + 5.0f) / 6.5f);
			return Is + saturate(In * (float4(0.109375f, 0.41796875f, 0.625f, alpha) + white));
		}
	}
	return Is + saturate(In * float4(0.109375f, 0.41796875f, 0.625f, alpha));
}