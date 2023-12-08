cbuffer CameraConstantBuffer : register(b0)
{
    float4x4 viewProjection;
    float4x4 model;
};

struct VS_IN
{
    float3 pos : POSITION0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;

    float4 pos = float4(input.pos, 1.0f);
    float4 modelPos = mul(pos, model);
    output.pos = mul(modelPos, viewProjection);
    
    return output;
}

float PSMain(PS_IN input) : SV_Depth
{
    float depth = input.pos.z / input.pos.w;
    return depth;
}