#include "Buffers.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

struct GS_OUTPUT
{
	float4 position		 : SV_POSITION;
	float3 normal		 : NORMAL;
	float3 worldPosition : POSITION;
	float4 color		 : COLOR;
};

[maxvertexcount(4)]
void main(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> output)
{
	// BILLBOARD PER PARTICLE
	float3 pos = input[0].position / input[0].position.w;
	
	float3 forward = normalize(eye0 - pos);
	float3 right = normalize(cross(float3(0, 1, 0), forward));
	float3 up = normalize(cross(forward, right));

	const float SIZE = 0.2f;

	float3 wpos [4];
	wpos[0] = pos + up * SIZE - right * SIZE;
	wpos[1] = pos + up * SIZE + right * SIZE;
	wpos[2] = pos - up * SIZE - right * SIZE;
	wpos[3] = pos - up * SIZE + right * SIZE;

	float2 uv[4];
	uv[0] = float2(0, 0);
	uv[1] = float2(1, 0);
	uv[2] = float2(1, 1);
	uv[3] = float2(0, 1);

	GS_OUTPUT outp;
	outp.normal = -forward;
	outp.color = input[0].color;

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		outp.position = mul(mul(float4(wpos[i], 1.0f), view), projection);
		outp.worldPosition = wpos[i];
		//outp.uv = uv[i];
		output.Append(outp);
	}

}