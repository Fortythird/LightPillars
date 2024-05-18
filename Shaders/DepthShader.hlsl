cbuffer CameraConstantBuffer : register(b0)
{
    float4x4 viewProjection;
    float4x4 model;
};

float4 VSMain(float3 input : POSITION0) : SV_POSITION
{
    float4 pos = float4(input, 1.0f);
    float4 modelPos = mul(pos, model);
    float4 output = mul(modelPos, viewProjection);
    
    return output;
}

float PSMain(float4 input : SV_POSITION) : SV_Depth
{
    float depth = input.z / input.w;
    return depth;
}