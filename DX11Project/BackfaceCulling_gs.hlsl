#include "Buffers.hlsl"
#include "IO.hlsl"


[maxvertexcount(3)]
void main(triangle VS_OUTPUT input[3], inout TriangleStream<VS_OUTPUT> output)
{
	float3 center = (input[0].worldPosition + input[1].worldPosition + input[2].worldPosition).xyz / 3.0f;
	float3 e1 = normalize(input[1].worldPosition - input[0].worldPosition).xyz;
	float3 e2 = normalize(input[2].worldPosition - input[0].worldPosition).xyz;

	float3 normal = normalize(cross(e1, e2));
	float d = dot(normalize(center - eye0), normal);

	if (d <= 0.0f)
	{
		output.Append(input[0]);
		output.Append(input[1]);
		output.Append(input[2]);
	}
}
