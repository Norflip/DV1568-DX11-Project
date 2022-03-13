#include "Buffers.hlsl"
#include "IO.hlsl"

TextureCube m_texture : register(t0);
SamplerState m_sampler : register(s0);

struct DYNAMIC_CUBE_VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 worldPosition : POSITION0;
	float3 viewDirection : TANGENT;
};

GBUFFER main (DYNAMIC_CUBE_VS_OUTPUT input) : SV_TARGET
{
	float3 refl = reflect(input.viewDirection, input.normal);
	float4 color = m_texture.Sample(m_sampler, refl);
	color.a = 0.0f;

	GBUFFER output;
	output.albedo = color;
	output.normal = float4(input.normal, 1.0f);
	output.position = float4(input.worldPosition, 0.0f);

	return output;
}