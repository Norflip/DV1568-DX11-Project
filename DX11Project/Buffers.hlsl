
cbuffer cb_World : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;

	float3 eye0;
	float cb_w_pad0;
};



cbuffer cb_Material : register(b1)
{
	float4 Ka; //ambient color
	float4 Kd; // diffuse color

	bool hasAlbedoTexture;		// map_Ka 
	bool hasNormalTexture;		// norm 

	bool cb_m_pad0[2];  // 1 byte
	//float3 cb_m_pad1;
}

struct PointLight
{
	float4 color;
	float3 position;
	float radius;
};

cbuffer cb_Light : register (b2)
{
	float4 sunColor;
	float3 sunDirection;
	float sunStrength;

	float3 eye;
	int pointLightCount;
	PointLight pointLights[16]; // n * 16 * 3 byte
}
