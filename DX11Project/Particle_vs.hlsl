#include "Buffers.hlsl"

struct VS_INPUT_PARTICLE
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT_PARTICLE input) 
{
	VS_OUTPUT output;
	output.position = mul(input.position, model);
	output.color = input.color;
	return output;
}