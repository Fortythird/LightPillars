struct ConstData
{
    float4x4 worldViewProj;
    float4x4 world;
    float4x4 invertedWorldTransform;
};

cbuffer ConstBuf : register(b0)
{
    ConstData constData;
};

float4 VSMain(uint vI : SV_VERTEXID) : SV_POSITION
{
	float2 texcoord = float2(vI&1,vI>>1);
    return float4((texcoord.x - 0.5f) * 2, -(texcoord.y - 0.5f) * 2, 0, 1);
}

float4 PSMain(float4 pos : SV_POSITION) : SV_TARGET
{
	float4 color = pos;
	return color;
}