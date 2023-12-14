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

static const float CRIT_ANGLE_RAD = radians(5.0f);
static const float CRIT_ANGLE_DEG = 5.0f;
static const float PI = 3.1412f;

float CalculateGaussProt(float d)
{
    return (2.0f * (903168.0f * pow(d, 10.0f) - 24613120.0f * pow(d, 9.0f) + 275587200.0f * pow(d, 8.0f) - 1540512000.0f * pow(d, 7.0f)
		+ 3869040000.0f * pow(d, 6.0f) - 1775340000.0f * pow(d, 5.0f) + 11280150000.0f * pow(d, 4.0f) - 126215250000.0f * pow(d, 3.0f)
		+ 5906250000.0f * pow(d, 2.0f) + 1928925534375.0f * d) / (8064.0f * pow(10.0f, 9.0f)));
}

float CalculateGaussIntegral(float lowerBorder, float upperBorder)
{
    return (CalculateGaussProt(upperBorder) - CalculateGaussProt(lowerBorder));
}

float CalculateRefractionProt(float d)
{
    return (0.15f * sin(4.0f * d) + 3.68f * sin(2.0f * d) - 0.2f * pow(sin(d), 5.0f)
        + 3.93f * pow(sin(d), 3.0f) - 15.71f * sin(d) + 8.73 * d) * 5.14f;
}

float CalculateRefractionIntegral(float lowerBorder, float upperBorder)
{
    return (CalculateRefractionProt(upperBorder) - CalculateRefractionProt(lowerBorder));
}

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
    
    if (/*dot(viewDir, pointLightData.lightSourcePosition.xyz - constData.viewerPos) <= 0*/false) return float4(0, 0, 0, 1);
    else
    {
        float shiftY = (pointLightData.lightSourcePosition.y - constData.viewerPos.y) / viewDir.y;
    
        float R0x = (pointLightData.lightSourcePosition.x + constData.viewerPos.x + viewDir.x * shiftY) / 2.0f;
        float R0z = (pointLightData.lightSourcePosition.z + constData.viewerPos.z + viewDir.z * shiftY) / 2.0f;
        float3 R0 = float3(
            R0x,
            viewDir.x == 0 ? constData.viewerPos.y + viewDir.y * (R0z - constData.viewerPos.z) / viewDir.z 
                : constData.viewerPos.y + viewDir.y * (R0x - constData.viewerPos.x) / viewDir.x,
            R0z
        );
    
        float3 reflectionDir = normalize(pointLightData.lightSourcePosition.xyz - R0);
    
        float3 normal0 = normalize(viewDir - reflectionDir);
    
        if (normal0.y < cos(CRIT_ANGLE_RAD)) return float4(0.0f, 0.0f, 0.0f, 1.0f);
        else
        {
            float3 lightPosDir = normalize(pointLightData.lightSourcePosition.xyz - constData.viewerPos);
        
            float a = (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x) + pow((viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y), 2);
        
            float b = -2 * cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) * (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x);
        
            float c = pow(cos(CRIT_ANGLE_RAD), 2) * pow(viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x, 2)
            - pow(sin(CRIT_ANGLE_RAD), 2) * pow(viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y, 2);
        
            float discriminant = pow(b, 2) - 4 * a * c;
        
            float n1z = (-b + sqrt(discriminant)) / 2 / a;
        
            float3 n1 = float3(
                (cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) - n1z * (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x))
                    / (viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y),
                cos(CRIT_ANGLE_RAD),
                n1z
            );
        
            float n2z = (-b - sqrt(discriminant)) / 2 / a;
            float3 n2 = float3(
                (cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) - n2z * (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x))
                    / (viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y),
                cos(CRIT_ANGLE_RAD),
                n2z
            );
        
            float3 reflectionDir1 = float3(
                viewDir.x - n1.x,
                viewDir.y - n1.y,
                viewDir.z - n1.z
            );
        
            float3 reflectionDir2 = float3(
                viewDir.x - n2.x,
                viewDir.y - n2.y,
                viewDir.z - n2.z
            );
        
            float k = 0;
        
            if (viewDir.x != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir1.x / reflectionDir1.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.x * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.x 
                    - viewDir.z * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.z) / (viewDir.x * reflectionDir1.z - viewDir.z * reflectionDir1.x);
                else
                    k = (viewDir.x * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.x 
                    - viewDir.y * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.y) / (viewDir.x * reflectionDir1.y - viewDir.y * reflectionDir1.x);
            }
            else if (viewDir.y != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir1.x / reflectionDir1.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.y * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.y 
                    - viewDir.z * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.z) / (viewDir.y * reflectionDir1.z - viewDir.z * reflectionDir1.y);
                else
                    k = (viewDir.y * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.y 
                    - viewDir.x * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.x) / (viewDir.y * reflectionDir1.x - viewDir.x * reflectionDir1.y);
            }
            else if (viewDir.z != 0)
            {
                if (viewDir.x / viewDir.z == constData.viewerPos.x / constData.viewerPos.z
                && reflectionDir1.x / reflectionDir1.z == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.z)
                    k = (viewDir.z * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.z 
                    - viewDir.y * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.y) / (viewDir.z * reflectionDir1.y - viewDir.y * reflectionDir1.z);
                else
                    k = (viewDir.z * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.z 
                    - viewDir.x * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.x) / (viewDir.z * reflectionDir1.x - viewDir.x * reflectionDir1.z);
            }
        
            float3 R1 = float3(
                pointLightData.lightSourcePosition.x + k * reflectionDir1.x,
                pointLightData.lightSourcePosition.y + k * reflectionDir1.y,
                pointLightData.lightSourcePosition.z + k * reflectionDir1.z
            );
        
            if (viewDir.x != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir2.x / reflectionDir2.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.x * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.x 
                    - viewDir.z * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.z) / (viewDir.x * reflectionDir2.z - viewDir.z * reflectionDir2.x);
                else
                    k = (viewDir.x * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.x 
                    - viewDir.y * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.y) / (viewDir.x * reflectionDir2.y - viewDir.y * reflectionDir2.x);
            }
            else if (viewDir.y != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir2.x / reflectionDir2.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.y * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.y 
                    - viewDir.z * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.z) / (viewDir.y * reflectionDir2.z - viewDir.z * reflectionDir2.y);
                else
                    k = (viewDir.y * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.y 
                    - viewDir.x * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.x) / (viewDir.y * reflectionDir2.x - viewDir.x * reflectionDir2.y);
            }
            else if (viewDir.z != 0)
            {
                if (viewDir.x / viewDir.z == constData.viewerPos.x / constData.viewerPos.z
                && reflectionDir2.x / reflectionDir2.z == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.z)
                    k = (viewDir.z * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.z 
                    - viewDir.y * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.y) / (viewDir.z * reflectionDir2.y - viewDir.y * reflectionDir2.z);
                else
                    k = (viewDir.z * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.z 
                    - viewDir.x * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.x) / (viewDir.z * reflectionDir2.x - viewDir.x * reflectionDir2.z);
            }
        
            float3 R2 = float3(
                pointLightData.lightSourcePosition.x + k * reflectionDir2.x,
                pointLightData.lightSourcePosition.y + k * reflectionDir2.y,
                pointLightData.lightSourcePosition.z + k * reflectionDir1.z
            );
        
            float N = abs(CalculateGaussIntegral(acos(n1.y) / PI * 180.0f, acos(normal0.y) / PI * 180.0f)
                + CalculateGaussIntegral(acos(n2.y) / PI * 180.0f, acos(normal0.y) / PI * 180.0f));
            
            float e0 = acos(normal0.x * viewDir.x + normal0.y * viewDir.y + normal0.z * viewDir.z);
            float e1 = acos(n1.x * viewDir.x + n1.y * viewDir.y + n1.z * viewDir.z);
            float e2 = acos(n2.x * viewDir.x + n2.y * viewDir.y + n2.z * viewDir.z);
            
            //N *= (abs(CalculateRefractionIntegral(e1, e2)));
            
            return float4(pointLightData.lightColor.xyz * N, 1.0f);
            //return float4(pointLightData.lightColor);
        }
    }
}