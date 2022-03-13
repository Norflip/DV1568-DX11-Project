#include "Buffers.hlsl"
#include "IO.hlsl"

Texture2D m_albedoTexture : register(t0);
Texture2D m_normalTexture : register(t1);
SamplerState m_sampler : register(s0);

GBUFFER main(VS_OUTPUT input) : SV_TARGET
{
	// kolla med bools i material
	//float3 textureColor = m_albedoTexture.Sample(m_sampler, input.uv).xyz;
	//// räknar ut bitangent och gör en 3x3 matrix för att transformera normalen

	float4 color = Kd;
	if (hasAlbedoTexture)
	{
		color = m_albedoTexture.Sample(m_sampler, input.uv);
	}


	float3 normal = input.normal;

	if (hasNormalTexture)
	{
		float4 bump = m_normalTexture.Sample(m_sampler, input.uv);
		bump = ((bump * 2.0f) - 1.0f);

		float3 bitangent = cross(input.tangent, input.normal);
		float3x3 TBN = float3x3(input.tangent, bitangent, input.normal);
		normal = normalize(mul(bump.xyz, TBN));
	}

	// skickar tillbaka en alpha på 1, detta gör så den ges shading
	// specular strength samplas och sätts in i color alpha
	GBUFFER gbuffer;
	gbuffer.albedo = float4(color.rgb, 1.0f);
	gbuffer.normal = float4(normal, 1.0f);
	gbuffer.position = float4(input.worldPosition, 1.0f);
	return gbuffer;
}