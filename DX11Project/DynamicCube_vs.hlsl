#include "Buffers.hlsl"
#include "IO.hlsl"

struct DYNAMIC_CUBE_VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 worldPosition : POSITION0;
	float3 viewDirection : TANGENT;
};

DYNAMIC_CUBE_VS_OUTPUT main (VS_INPUT input)
{
	DYNAMIC_CUBE_VS_OUTPUT output;
	output.uv = input.uv;

	float4 tmp = input.position;
	tmp = mul(tmp, model);
	output.worldPosition = tmp.xyz;
	output.position = mul(mul(tmp, view), projection);

	float4 normal = mul(float4(input.normal, 0), model);
	output.normal = normalize(normal.xyz);
	output.viewDirection = normalize(tmp.xyz - eye0);

	return output;
}