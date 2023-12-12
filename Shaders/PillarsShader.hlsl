struct ConstData
{
    float4x4 invertedCamViewProjection;
    float3 viewerPos;
    float dummy;
};

struct PointLightData
{
    float4 lightSourcePosition;
    float4 lightColor;
};

cbuffer ConstBuf : register(b0)
{
    ConstData constData;
};

cbuffer ConstBuf : register(b1)
{
    PointLightData pointLightData;
};

const float CRIT_ANGLE_RAD = radians(5);

float4 VSMain(uint vI : SV_VERTEXID) : SV_POSITION
{
	float2 texcoord = float2(vI & 1, vI >> 1);
    return float4((texcoord.x - 0.5f) * 2.0f, -(texcoord.y - 0.5f) * 2.0f, 0.0f, 1.0f);
}

float4 PSMain(float4 pos : SV_POSITION) : SV_TARGET
{
    float4 NDC = float4(pos.x * 2.0f / 800.0f - 1.0f, -pos.y * 2.0f / 800.0f + 1.0f, 0.0f, 1.0f);
    float4 worldPoint = mul(NDC, constData.invertedCamViewProjection);
    worldPoint /= worldPoint.w;
    float3 viewDir = normalize(worldPoint.xyz - constData.viewerPos);
    
    float4 col;
    col.x = abs(viewDir.x);
    col.y = abs(viewDir.y);
    col.z = abs(viewDir.z);
    col.w = 1.0f;
    
    return col;
    
    
    //return normalize(float4(abs(constData.viewerTransform[0].w), abs(constData.viewerTransform[1].w), abs(constData.viewerTransform[2].w), 1.0f));
    
    /*float shiftZ = (pointLightData.lightSourcePosition.z - camPos.z) / viewDir.z;
    
    float R0x = (pointLightData.lightSourcePosition.x + camPos.x + viewDir.x * shiftZ) / 2.0f;
    float R0y = (pointLightData.lightSourcePosition.y + camPos.y + viewDir.y * shiftZ) / 2.0f;
    float4 R0 = float4(
        R0x, 
        R0y, 
        viewDir.x == 0 ? camPos.z + viewDir.z * (R0y - camPos.y) / viewDir.y : camPos.z + viewDir.z * (R0x - camPos.x) / viewDir.x,
        1.0f
    );
    
    float4 reflectionDir = normalize(float4(
        pointLightData.lightSourcePosition.x - R0.x,
        pointLightData.lightSourcePosition.y - R0.y,
        pointLightData.lightSourcePosition.z - R0.z,
        0.0f
    ));
    
    float4 normal0 = normalize(float4(viewDir.x - reflectionDir.x, viewDir.y - reflectionDir.y, viewDir.z - reflectionDir.z, 0.0f));
    
    if (normal0.z > cos(CRIT_ANGLE_RAD)) return float4(0.0f, 0.0f, 0.0f, 1.0f);
    else
    {
        float4 lightPosDir = normalize(float4(pointLightData.lightSourcePosition - camPos));
        
        // REWRITE TO b^2-4ac format
        
        float aParameter = pow((viewDir.z * lightPosDir.x - viewDir.x * lightPosDir.z) / (viewDir.y * lightPosDir.z - viewDir.z * lightPosDir.y), 2) + 1;
        float bParameter = (cos(CRIT_ANGLE_RAD) * (viewDir.y * lightPosDir.x - viewDir.x * lightPosDir.y) * (viewDir.z * lightPosDir.x - viewDir.x * lightPosDir.z))
            / pow(viewDir.y * lightPosDir.z - viewDir.z * lightPosDir.y, 2);
        float sqrtDiscriminant = sqrt(pow(bParameter, 2) 
            - aParameter * (pow(cos(CRIT_ANGLE_RAD) * (viewDir.y * lightPosDir.x - viewDir.x * lightPosDir.y) / (viewDir.y * lightPosDir.z - viewDir.z * lightPosDir.y), 2)
            - pow(sin(CRIT_ANGLE_RAD), 2))
        );
        
        float n1y = (bParameter + sqrtDiscriminant) / aParameter;
        float n1x = (cos(CRIT_ANGLE_RAD) * (viewDir.y * lightPosDir.x - viewDir.x * lightPosDir.y) - (viewDir.z * lightPosDir.x - viewDir.x * lightPosDir.z) * n1y)
            / (viewDir.y * lightPosDir.z - viewDir.z * lightPosDir.y);
        
        float4 n1 = float4(n1x, n1y, cos(CRIT_ANGLE_RAD), 0.0f);
        
        float n2y = (bParameter - sqrtDiscriminant) / aParameter;
        float n2x = (cos(CRIT_ANGLE_RAD) * (viewDir.y * lightPosDir.x - viewDir.x * lightPosDir.y) - (viewDir.z * lightPosDir.x - viewDir.x * lightPosDir.z) * n2y)
            / (viewDir.y * lightPosDir.z - viewDir.z * lightPosDir.y);
        
        float4 n2 = float4(n2x, n2y, cos(CRIT_ANGLE_RAD), 0.0f);
        
        float4 borderRay1 = float4(viewDir.x - n1.x, viewDir.y - n1.y, viewDir.z - n1.z, 0.0f);
        float4 borderRay2 = float4(viewDir.x - n2.x, viewDir.y - n2.y, viewDir.z - n2.z, 0.0f);
        
        
        //return float4(normalize(worldPoint.xyz - camPos.xyz), 1.0f);
    }*/
}