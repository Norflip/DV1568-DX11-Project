#include "Buffers.hlsl"
#include "IO.hlsl"

VS_OUTPUT main (VS_INPUT input)
{
	VS_OUTPUT output;
	output.uv = input.uv;

	float4 tmp = input.position;
	tmp = mul(tmp, model);

	output.worldPosition = tmp.xyz;
	output.position = mul(mul(tmp, view), projection);

	float4 normal = float4(input.normal, 0);
	output.normal = normalize(mul(normal, model).xyz);

	float4 tangent = float4(input.tangent, 0);
	output.tangent = normalize(mul(tangent, model).xyz);

	return output;
}