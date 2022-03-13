#include "Buffers.hlsl"

struct VS_INPUT
{
	float4 position : POSITION;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
};

struct VS_OUTPUT
{
	float4 position		 : SV_POSITION;
	float3 normal		 : NORMAL;
	float2 uv			 : TEXCOORD0;
	float3 worldPosition : POSITION;
	float3 tangent		 : TANGENT;
};

Texture2D m_heightTexture : register(t0);
SamplerState m_heightSampler : register(s0);

float4 GetNormal(float2 uv, float texel, float height)
{
	//texel is one uint size, ie 1.0/texture size
	float t0 = m_heightTexture.SampleLevel(m_heightSampler, uv + float2(-texel, 0), 0).x * height;
	float t1 = m_heightTexture.SampleLevel(m_heightSampler, uv + float2(texel, 0), 0).x * height;
	float t2 = m_heightTexture.SampleLevel(m_heightSampler, uv + float2(0, -texel), 0).x * height;
	float t3 = m_heightTexture.SampleLevel(m_heightSampler, uv + float2(0, texel), 0).x * height;

	float3 va = normalize(float3(1.0f, 0.0f, t1 - t0));
	float3 vb = normalize(float3(0.0f, 1.0f, t3 - t2));
	return float4(cross(va, vb).rbg, 0.0f);
}

VS_OUTPUT main (VS_INPUT input)
{
	VS_OUTPUT output;
	output.uv = input.uv;


	float4 tmp = input.position;
	tmp = mul(tmp, model);


	const float terrainHeight = 5.0f;
	float t = m_heightTexture.SampleLevel(m_heightSampler, input.uv, 0).x;
	tmp.y += t * terrainHeight;

	output.worldPosition = tmp.xyz;
	output.position = mul(mul(tmp, view), projection);

	float4 normal = GetNormal(input.uv, 1.0f / 65.0f, terrainHeight);
	normal = mul(normal, model);
	output.normal = normalize(normal.xyz);
	output.tangent = input.tangent;

	return output;
}