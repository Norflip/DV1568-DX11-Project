cbuffer ParticleWorldData : register (b0)
{
    float3 emitterPosition;
    float time;

    float lifeTime;
    float3 pad0;
};

struct ConstantParticleData
{
    float3 position;
    float3 velocity;
    float gravityModifier;
    float timeOffset;
    float4 startColor;
    float4 endColor;
};

// OUTGOING
struct Particle 
{
    float3 position : POSITION;
    float4 color : COLOR;
};

StructuredBuffer<ConstantParticleData>  inputConstantParticleData   : register(t0);
RWStructuredBuffer<Particle>        outputParticleData          : register(u0);

[numthreads(10, 1, 1)]
void main (uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ConstantParticleData cpd = inputConstantParticleData[dispatchThreadID.x];

    const float3 gravity = float3(0, -9.82f, 0);
    float3 acceleration = gravity * cpd.gravityModifier;

    float t = (time + cpd.timeOffset) % lifeTime;
    float progress = t / lifeTime;

    float3 startPos = emitterPosition + cpd.position;
    float3 startVelocity = cpd.velocity;
    float3 pos = startPos + startVelocity * t + (acceleration * t * t) * 0.5f;

    Particle particleResult;
    particleResult.position = pos;
    particleResult.color = lerp(cpd.startColor, cpd.endColor, progress);
    outputParticleData[dispatchThreadID.x] = particleResult;
  

}