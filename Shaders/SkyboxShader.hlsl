Texture2D gCubeMap:register(t0);
SamplerState gCubeMapSampler:register(s0);

float4 VSMain(uint vI : SV_VERTEXID) : SV_POSITION
{
    float2 texcoord = float2(vI & 1, vI >> 1);
    return float4((texcoord.x - 0.5f) * 2.0f, -(texcoord.y - 0.5f) * 2.0f, 0.0f, 1.0f);
}

float4 PSMain(float4 pos : SV_POSITION) : SV_TARGET
{
    return gCubeMap.Sample(gCubeMapSampler, float2((pos.x + 1.0f) / 2, -(pos.y + 1.0f) / 2), 0).x;
}