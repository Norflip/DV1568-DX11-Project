#include "Buffers.hlsl"
#include "IO.hlsl"

struct GS_OUTPUT
{
	float4 position		 : SV_POSITION;
	float3 normal		 : NORMAL;
	float3 worldPosition : POSITION;
	float4 color		 : COLOR;
};

GBUFFER main(GS_OUTPUT input) : SV_TARGET
{
	// skickar tillbaka en alpha på 1, detta gör så den ges shading
	// specular strength samplas och sätts in i color alpha
	GBUFFER gbuffer;
	gbuffer.albedo = float4(input.color.xyz, 0.0f);
	gbuffer.normal = float4(input.normal, 1.0f);
	gbuffer.position = float4(input.worldPosition, 1.0f);
	return gbuffer;
}