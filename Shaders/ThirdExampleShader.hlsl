struct VS_IN
{
	float4 pos : POSITION;
	float2 texCoord : TEXCOORD;
	float4 normal : NORMAL;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 worldPos : TEXCOORD1;
	float4 normal :NORMAL;
};

struct ConstData
{
	float4x4 worldViewProj;
	float4x4 world;
	float4x4 invertedWorldTransform;
};

cbuffer ConstBuf:register(b0)
{
	ConstData constData;
};

struct LightData
{
	float4 direction;
	float4 color;
	float4 viewerPos;
	float4x4 worldViewProj;
};

cbuffer LightBuf:register(b1)
{
	LightData lightData;
};

struct LightConstData
{
	float4 pos;
	float4x4 view;
	float4x4 projView;
};

cbuffer lightConstBuf:register(b2)
{
	LightConstData lightConstData;
};

Texture2D TextureImg:register(t0);
Texture2D depthTexture:register(t1);
SamplerState Sampler:register(s0);
SamplerState depthSampler:register(s1);

PS_IN VSMain(VS_IN input)
{
	PS_IN output = (PS_IN)0;
	output.pos = mul(float4(input.pos.xyz, 1.0f), constData.worldViewProj);
	output.uv = input.texCoord;
	output.worldPos = mul(float4(input.pos.xyz, 1.0f), constData.world);
	output.normal = mul(float4(input.normal.xyz, 1.0f), constData.invertedWorldTransform);
	
	return output;
}

float4 PSMain(PS_IN input) : SV_TARGET
{
    float4 color = TextureImg.SampleLevel(Sampler, input.uv.yx, 0);
	float4 lightSpacePos = mul(lightConstData.projView, input.worldPos);
	float depthValue = depthTexture.SampleLevel(depthSampler, 
		float2(lightSpacePos.x / lightSpacePos.w, -lightSpacePos.y / lightSpacePos.w) * 0.5f + 0.5f, 0).x;

	float3 LightDir = lightData.direction.xyz;
	float3 normal = normalize(input.normal.xyz);

	float3 viewDir = normalize(lightData.viewerPos.xyz - input.worldPos.xyz);
	float3 refVec = normalize(reflect(LightDir, normal));

	float3 ambient = color.xyz * 0.1;
	float3 diffuse = 0.5 * saturate(dot(LightDir, normal)) * color.xyz;
	float3 specular = pow(saturate(dot(-viewDir, refVec)), 5.0) * 0.7;

	float shadowTest = lightSpacePos.z / lightSpacePos.w - 0.001f < depthValue;

    return float4(lightData.color.xyz * (ambient + (diffuse + specular) * shadowTest), 1.0f);
}