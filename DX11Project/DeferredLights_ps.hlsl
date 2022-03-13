#include "Buffers.hlsl"
#include "IO.hlsl"

Texture2D colorTexture: register(t0);
Texture2D normalTexture: register(t1);
Texture2D positionTexture: register(t2);

SamplerState samplerState: register(s0);

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 CalculatePointLight(PointLight light, float3 eyeDirection, float3 normal, float3 position)
{
	const float SPECULAR_POW = 32;
	const float SPECULAR_STRENGTH = 0.05f;


	float3 lightDirection = normalize(light.position - position);

	//diffuse
	float diffuseShade = saturate(dot(normal, lightDirection));

	// specular
	float3 specularReflection = reflect(-lightDirection, normal);
	float specularShading = pow(saturate(dot(eyeDirection, specularReflection)), SPECULAR_POW);

	float distance = length(light.position - position);
	float attenuation = saturate(1.0 - distance / light.radius);
	attenuation *= attenuation;

	// COMBINE
	float4 ambient = light.color * 0.2f;
	float4 diffuse = light.color * diffuseShade;
	float4 specular = light.color * specularShading * SPECULAR_STRENGTH;

	float4 result = saturate(ambient + diffuse + specular) * attenuation;
	//float4 result = saturate(ambient + diffuse) * attenuation;

	result.w = 1.0f;
	return result;
}


float4 main (PixelInputType input) : SV_TARGET
{
	float4 color = colorTexture.Sample(samplerState, input.uv);
	float3 normal = normalTexture.Sample(samplerState, input.uv).xyz;
	float3 position = positionTexture.Sample(samplerState, input.uv).xyz;

	const float SPECULAR_POW = 32;
	const float SPECULAR_STRENGTH = 0.05f;

	float3 eyeDirection = (position - eye);

	// DIFFUSE
	float3 directionToLight = normalize(-sunDirection);
	float diffuseShade = saturate(dot(normal, directionToLight));

	// SPECULAR
	float3 specularReflection = reflect(-directionToLight, normal);
	float specularShade = pow(saturate(dot(eyeDirection, specularReflection)), SPECULAR_POW);

	// COMBINE
	float4 ambient = color * sunStrength;
	float4 diffuse = color * sunColor * diffuseShade;
	float4 specular = color * sunColor * specularShade * SPECULAR_STRENGTH;

	float4 finalColor = saturate(ambient + diffuse + specular);

	for (int i = 0; i < pointLightCount; i++)
	{
		finalColor += CalculatePointLight(pointLights[i], eyeDirection, normal, position);
	}

	finalColor = float4(saturate(finalColor).xyz, 1.0f);
	
	// om alpha är 0 så skickas basfärgen tillbaka, bra för partiklar och dynamic cuben
	return lerp(color, finalColor, color.w);
}